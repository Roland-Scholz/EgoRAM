//
// Title:	        Pico-mposite Video Output
// Description:		The composite video stuff
// Author:	        Dean Belfield
// Created:	        26/01/2021
// Last Updated:	27/09/2024
//
// Modinfo:
// 15/02/2021:      Border buffers now have horizontal sync pulse set correctly
//                  Decreased RAM usage by updating the image buffer scanline on the fly during the horizontal interrupt
//					Fixed logic error in cvideo_dma_handler; initial memcpy done twice
// 31/01/2022:      Refactored to use less memory
//					Split the video generation into two state machines; sync and data
// 01/02/2022:      Added a handful of graphics primitives
// 02/02/2022:      Split main loop out into main.c
// 04/02/2022:      Added set_border
// 05/02/2022:      Added support for colour, fixed bug in video generation
// 20/02/2022:      Bitmap is now dynamically allocated; added two higher resolution video modes
// 25/02/2022:      Lengthened HSYNC to 12us
// 27/09/2024:		PIO state machines now started simultaneously

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include "pico/stdlib.h"
#include "pico/multicore.h" // Wichtig für Multicore-Funktionen

#include "hardware/pio.h"
#include "hardware/dma.h"
#include "hardware/irq.h"
#include "hardware/sync.h"
#include "hardware/structs/systick.h"

#include "cvideo.h"
#include "cvideo_sync.pio.h" // The assembled PIO code
#include "clock.pio.h"       // The assembled PIO code
#include "vt100_font_8x8.h"

// PIO pio_0;                      // The PIO that this uses
static uint offset_0; // Program offsets
static uint offset_1; // Program offsets

static uint dma_channel_0; // DMA channel for transferring sync data to PIO
static uint vline;         // Current PAL(ish) video line being processed
static uint bline;         // Line in the bitmap to fetch

static uint vblank_count; // Vblank counter
static bool cursor_onoff = true;
static int cursor_frame_cnt = 0;
static int blitwidth;
static int blitheight;

static uint8_t __attribute__((aligned(4))) hsync[BYTESPERLINE];
static uint8_t __attribute__((aligned(4))) border[BYTESPERLINE];
static uint8_t __attribute__((aligned(4))) vsync[BYTESPERLINE];
static uint8_t __attribute__((aligned(4))) black[BYTESPERLINE];
static uint8_t __attribute__((aligned(4))) line0[BYTESPERLINE];
static uint8_t __attribute__((aligned(4))) line1[BYTESPERLINE];
static uint8_t __attribute__((aligned(4))) video_ram[PAL_WIDTH * PAL_HEIGHT * 8];
static uint8_t __attribute__((aligned(4))) color_ram[PAL_WIDTH * PAL_HEIGHT];

static uint8_t *pline0 = line0;
static uint8_t *pline1 = line1;
static int pal_mode = true;
static int graphics_mode = false;
static uint8_t *uint8_tset = (uint8_t *)vt100_font_8x8;
static double divisor_clock;
static double divisor_cvideo;
static PIO glb_pio;
static int clock_sm;

static uint8_t __attribute__((aligned(4))) mapping[] = {
    0b01010101,
    0b01010111,
    0b01011101,
    0b01011111,
    0b01110101,
    0b01110111,
    0b01111101,
    0b01111111,
    0b11010101,
    0b11010111,
    0b11011101,
    0b11011111,
    0b11110101,
    0b11110111,
    0b11111101,
    0b11111111};

static void systick_init(void)
{
    systick_hw->csr = 0x5;        // Enable + CPU-Clock als Quelle
    systick_hw->rvr = 0x00FFFFFF; // Maximaler Reload-Wert (24-Bit)
}

static void generate_synclines()
{
    memset(black, BLACK, sizeof(black));
    memset(hsync, BLACK, sizeof(hsync));
    memset(&hsync[HSYNCSTART], SYNC, HSYNCLEN);

    memcpy(border, hsync, sizeof(border));
    memset(&border[DATASTART - GREY_OFFSET], GREY, 160 + 2 * GREY_OFFSET);

    memset(vsync, SYNC, sizeof(vsync));
    // memset(&vsync[BYTESPERLINE - 1 - HSYNCLEN], BLACK, HSYNCLEN);

    memcpy(line0, border, sizeof(line0));
    memcpy(line1, border, sizeof(line1));
}

static void __not_in_flash("generate_line") generate_line(uint8_t *buffer)
{
    unsigned int screenline = bline >> 3;
    unsigned int uint8_tline = bline & 0x07;
    uint8_t *pscreen;
    uint8_t *pcolor;
    uint8_t c;
    uint8_t pix;
    uint8_t mask;
    uint8_t fgcol1, fgcol2, bgcol1, bgcol2;
    int x, i;

    if (false)
    {
        pscreen = &video_ram[bline * PAL_WIDTH];
        for (x = 0; x < PAL_WIDTH; x++)
        {
            c = *pscreen++;
            *buffer++ = mapping[c >> 4];
            *buffer++ = mapping[c & 15];
        }
    }
    else
    {
        pscreen = &video_ram[bline * PAL_WIDTH];
        pcolor = &color_ram[screenline * PAL_WIDTH];

        for (x = 0; x < PAL_WIDTH; x++)
        {
            // c = uint8_tset[(*pscreen++ << 3) + uint8_tline];
            c = pscreen[x];

            if (BITSPERPIXEL == 4)
            {
                mask = 128;
                for (i = 0; i < 4; i++)
                {
                    pix = 0x11;
                    if (c & mask)
                    {
                        pix |= pcolor[x] << 5;
                    }

                    mask >>= 1;
                    if (c & mask)
                    {
                        pix |= ((pcolor[x] << 1) & 0x0f);
                    }

                    mask >>= 1;
                    *buffer++ = pix;
                }
            }
            else
            {
                *buffer++ = mapping[c >> 4];
                *buffer++ = mapping[c & 0xf];
            }
        }
    }
}

// Funktion zur Berechnung der echten Frequenz einer State Machine
float get_sm_frequency(PIO pio, uint sm)
{
    // 1. Die aktuelle Systemfrequenz abfragen (z.B. Ihre 216 MHz)
    uint32_t sys_clk_hz = clock_get_hz(clk_sys);

    // 2. Den Ganzzahl- und Nachkommateil des Taktteilers aus den Hardware-Registern lesen
    // pio0_hw oder pio1_hw enthalten das Array 'sm' mit den Konfigurationen
    uint32_t clkdiv_reg = pio->sm[sm].clkdiv;

    // Bit 16 bis 31 enthalten den Integer-Teil
    uint16_t div_int = (clkdiv_reg >> 16) & 0xffff;
    // Bit 8 bis 15 enthalten den fraktionalen Teil (Basis 1/256)
    uint8_t div_frac = (clkdiv_reg >> 8) & 0xff;

    // Sonderfall der Hardware: Wenn INT = 0 ist, wird es als maximaler Teiler (65536) interpretiert
    float total_divider;
    if (div_int == 0)
    {
        total_divider = 65536.0f;
    }
    else
    {
        total_divider = (float)div_int + ((float)div_frac / 256.0f);
    }

    // 3. Systemtakt durch den Gesamtteiler teilen
    return (float)sys_clk_hz / total_divider;
}

/*
 * The main routine sets up the whole shebang
 */
void initialise_cvideo(PIO pio)
{
    // pio_0 = pio0;	                    // Assign the PIO

    glb_pio = pio;

    printf("Composite-Video initialising... \n");

    systick_init();

    setPalNtscMode(PAL);

    generate_synclines();

    // Load up the PIO programs
    //
    offset_0 = pio_add_program(pio, &clock_program);
    offset_1 = pio_add_program(pio, &cvideo_sync_program);

    dma_channel_0 = dma_claim_unused_channel(true); // Claim a DMA channel for the sync

    vline = 1;        // Initialise the video scan line counter to 1
    bline = 0;        // And the index into the bitmap pixel buffer to 0
    vblank_count = 0; // And the vblank counter

    divisor_cvideo = ((float)clock_get_hz(clk_sys) / (float)PIXCLOCK);
    divisor_clock = ((float)clock_get_hz(clk_sys) / (PALBURST * 8.0));

    printf("SysClk: %lu\n", clock_get_hz(clk_sys));
    printf("divisor_cvideo: %f\n", divisor_cvideo);
    printf("divisor_clock: %f\n", divisor_clock);
    printf("PIXCLOCK: %d\n", PIXCLOCK);
    printf("PIXPERLINE: %d\n", PIXPERLINE);
    printf("BYTESPERLINE: %d\n", BYTESPERLINE);
    printf("HSYNCLEN_PIX: %d\n", HSYNCLEN_PIX);
    printf("HSYNCLEN: %d\n", HSYNCLEN);
    printf("BACKPORCH: %d\n", BORDER);

    // Initialise the first PIO (Color clock)
    //
    pio_sm_set_enabled(pio, sm_clock, false); // Disable the PIO state machine
    pio_sm_clear_fifos(pio, sm_clock);        // Clear the PIO FIFO buffers
    clock_initialise_pio(                     // Initialise the PIO (function in cvideo.pio)
        pio,                                  // The PIO to attach this state machine to
        sm_clock,                             // The state machine number
        offset_0,                             // And offset
        GPIO_BASE + BITSPERPIXEL,             // Start pin in the GPIO
        1,                                    // Number of pins
        divisor_clock                         // State machine clock frequency
    );

    // Initialise the second PIO (video sync)
    //
    pio_sm_set_enabled(pio, sm_sync, false); // Disable the PIO state machine
    pio_sm_clear_fifos(pio, sm_sync);        // Clear the PIO FIFO buffers
    cvideo_sync_initialise_pio(              // Initialise the PIO (function in cvideo.pio)
        pio,                                 // The PIO to attach this state machine to
        sm_sync,                             // The state machine number
        offset_1,                            // And offset
        GPIO_BASE,                           // Start pin in the GPIO
        BITSPERPIXEL,                        // Number of pins
        divisor_cvideo                       // State machine clock frequency
    );

    cvideo_configure_pio_dma( // Configure the DMA
        pio,                  // The PIO to attach this DMA to
        sm_sync,              // The state machine number
        dma_channel_0,        // The DMA channel
        DMA_SIZE_8,           // Size of each transfer
        BYTESPERLINE,         // Number of bytes to transfer
        cvideo_dma_handler    // The DMA handler
    );

    printf("color_clock: %f\n", get_sm_frequency(pio, sm_clock) / 2.0f);
    printf("pixel_clock: %f\n", get_sm_frequency(pio, sm_sync));

    // Start the PIO state machines
    //
    // pio_enable_sm_mask_in_sync(pio, (1u << sm_sync));
    // pio_enable_sm_mask_in_sync(pio, (1u << sm_clock));
    uint32_t sm_maske = (1u << sm_clock) | (1u << sm_sync);
    // uint32_t sm_maske = (1u << sm_sync);

    pio_enable_sm_mask_in_sync(pio0, sm_maske);

    memset(video_ram, 0, PAL_WIDTH * PAL_HEIGHT);
    strcpy(video_ram, "pico-pal (c) by R. Scholz");

    printf("Composite-Video enabled \n");
    // uint32_t status = save_and_disable_interrupts();
    // irq_set_enabled(USBCTRL_IRQ, false);
}

extern sprite_t *sp_array[MAX_SPRITES];
extern int dx_array[MAX_SPRITES];
extern int dy_array[MAX_SPRITES];

void __not_in_flash("jiffy_callback") jiffy_callback(void)
{
    int xpos, ypos;
    sprite_t *sp;
    uint32_t ticks;

    ticks = systick_hw->cvr;

    for (int i = 0; i < MAX_SPRITES; i++)
    {
        sp = sp_array[i];

        sprite_draw(sp);

        xpos = sp->xpos;
        ypos = sp->ypos;

        xpos += dx_array[i];
        if (xpos >= PAL_WIDTH << 3 || xpos <= 0)
        {
            dx_array[i] *= -1;
            xpos += dx_array[i];
        }

        ypos += dy_array[i];
        if (ypos >= PAL_HEIGHT << 3 || ypos <= 0)
        {
            dy_array[i] *= -1;
            ypos += dy_array[i];
        }

        sprite_draw_xy(sp, xpos, ypos);
    }

    // printf("ticks: %d\n", (ticks - systick_hw->cvr) & 0xffffff);
}

// The DMA interrupt handler
// This feeds the state machine cvideo_sync with data for the PAL(ish) video signal
//
static void __not_in_flash("cvideo_dma_handler") cvideo_dma_handler(void)
{
    // Switch condition on the vertical scanline number (vline)
    // Each statement does a dma_channel_set_read_addr to point the PIO to the next data to output
    //

    // dma_channel_set_read_addr(dma_channel_0, line0, true);
    if (true)
    {
        switch (vline)
        {

        // First deal with the vertical sync scanlines
        // Also on scanline 3, preload the first pixel buffer scanline
        //
        case 1 ... 3:
            dma_channel_set_read_addr(dma_channel_0, vsync, true);
            break;
        case 4 ... 5:
            dma_channel_set_read_addr(dma_channel_0, hsync, true);
            bline = 0;
            pline0 = line0;
            pline1 = line1;
            generate_line(pline1 + DATASTART);
            break;
        case 310 ... 312:
            dma_channel_set_read_addr(dma_channel_0, black, true);
            break;

        // Then the border scanlines
        case 52 ... 68:
        case 269 ... 284:
            dma_channel_set_read_addr(dma_channel_0, border, true);
            break;

        case 6 ... 51:
        case 285 ... 309:
            dma_channel_set_read_addr(dma_channel_0, hsync, true);
            break;

        // Now point the dma at the first buffer for the pixel data,
        // and preload the data for the next scanline
        //
        default:

            if (pline0 == line0)
            {
                pline0 = line1;
                pline1 = line0;
            }
            else
            {
                pline0 = line0;
                pline1 = line1;
            }
            dma_channel_set_read_addr(dma_channel_0, pline0, true);
            bline++;
            generate_line(pline1 + DATASTART);
            break;
        }
    }
    else // NTSC
    {
        switch (vline)
        {
        case 1 ... 2:
            dma_channel_set_read_addr(dma_channel_0, vsync, true);
            break;
        case 3:
            dma_channel_set_read_addr(dma_channel_0, vsync, true);

            pline0 = line0;
            pline1 = line1;
            bline = 0;
            generate_line(pline1 + DATASTART);
            break;

#ifdef STATUS_LINE_ypos
        case 40 ... 247:
#else
        case 40 ... 239:
#endif
            if (pline0 == line0)
            {
                pline0 = line1;
                pline1 = line0;
            }
            else
            {
                pline0 = line0;
                pline1 = line1;
            }

            dma_channel_set_read_addr(dma_channel_0, pline0, true);

            bline++;
            generate_line(pline1 + DATASTART);

            break;
        default:
            dma_channel_set_read_addr(dma_channel_0, hsync, true);
            break;
        }
    }

    //
    // Increment and wrap the counters
    //
    vline++;
    if ((pal_mode && vline > 312) || (!pal_mode && vline > 262))
    {              // If we've gone past the bottom scanline then
        vline = 1; // Reset the scanline counter
        vblank_count++;

        cursor_frame_cnt++;

        multicore_fifo_push_blocking(42);

        if (cursor_frame_cnt >= CURSOR_TIME)
        {
            cursor_frame_cnt = 0;
            cursor_onoff = !cursor_onoff;
        }
    }

    // Finally, clear the interrupt request ready for the next horizontal sync interrupt
    dma_hw->ints0 = 1u << dma_channel_0;
}

// Configure the PIO DMA
// Parameters:
// - pio: The PIO to attach this to
// - sm: The state machine number
// - dma_channel: The DMA channel
// - transfer_size: Size of each DMA bus transfer (DMA_SIZE_8, DMA_SIZE_16 or DMA_SIZE_32)
// - buffer_size_words: Number of bytes to transfer
// - handler: Address of the interrupt handler, or NULL for no interrupts
//
static void __not_in_flash("cvideo_configure_pio_dma") cvideo_configure_pio_dma(PIO pio, uint sm, uint dma_channel, uint transfer_size, size_t buffer_size, irq_handler_t handler)
{
    dma_channel_config c = dma_channel_get_default_config(dma_channel);
    channel_config_set_transfer_data_size(&c, transfer_size);
    channel_config_set_read_increment(&c, true);
    channel_config_set_dreq(&c, pio_get_dreq(pio, sm, true));
    dma_channel_configure(dma_channel, &c,
                          &pio->txf[sm], // Destination pointer
                          NULL,          // Source pointer
                          buffer_size,   // Size of buffer
                          true           // Start flag (true = start immediately)
    );
    if (handler != NULL)
    {
        dma_channel_set_irq1_enabled(dma_channel, true);
        irq_set_exclusive_handler(DMA_IRQ_1, handler);
        irq_set_enabled(DMA_IRQ_1, true);
    }
}

void setPalNtscMode(int pal)
{
    pal_mode = pal;
    printf("Pal/Ntsc mode set to mode: %d\n", pal);
}

void setGraphicsMode(int mode)
{
    graphics_mode = mode;
    printf("Graphics mode set to mode: %d\n", mode);
}

uint8_t *getVideoRam()
{
    return video_ram;
}

static float pio_sm_get_frequency(PIO pio, uint sm)
{
    uint32_t div_reg = pio->sm[sm].clkdiv;

    // Extrahiere den Integer-Teil (Bits 31:16) und den Fraktional-Teil (Bits 15:8)
    float div_int = (float)(div_reg >> 16);
    float div_frac = (float)((div_reg & 0xff00) >> 8) / 256.0f;

    float tatsaechlicher_teiler = div_int + div_frac;

    if (tatsaechlicher_teiler == 0.0f)
    {
        tatsaechlicher_teiler = 1.0f;
    }

    return (float)clock_get_hz(clk_sys) / tatsaechlicher_teiler;
}

void change_divisior(int d)
{
    static int shifter = 0;
    static double value;

    shifter += d;

    value = divisor_clock + (shifter / 256.0f);
    printf("divisor_clock: %f\n", value);
    pio_sm_set_clkdiv(pio0, sm_clock, value);

    // set_sys_clock_pll(1064068560 + shifter, 3, 2);
    printf("color_clock: %f\n", pio_sm_get_frequency(pio0, sm_clock));
}

void putChar(int xpos, int ypos, uint8_t c)
{
    uint8_t *pscreen = &video_ram[(ypos * PAL_WIDTH * 8) + xpos];
    uint8_t *puint8_t = &uint8_tset[c << 3];

    for (int i = 0; i < 8; i++)
    {
        *pscreen = *puint8_t;
        puint8_t++;
        pscreen += PAL_WIDTH;
    }
}

sprite_t *sprite_new(uint8_t width, uint8_t height)
{
    sprite_t *sp;

    sp = malloc(sizeof(sprite_t));
    sp->width = width;
    sp->height = height;
    sp->data = malloc(width * height);
    memset(sp->data, 0xff, width * height);

    if (sp == NULL || sp->data == NULL)
    {
        printf("sprite_new failed!\n");
    }

    return sp;
}

void sprite_free(sprite_t *sp)
{
    if (sp)
    {
        free(sp->data);
        free(sp);
    }
}

void __not_in_flash("sprite_draw_all") sprite_draw_all()
{
    for (int i = 0; i < MAX_SPRITES; i++)
    {
        // printf("%d %p\n", i, sp_array[i]);
        sprite_draw(sp_array[i]);
    }
}

void __not_in_flash("sprite_draw") sprite_draw(sprite_t *sp)
{
    if (sp != NULL)
    {
        // printf("sprite_draw: %p x:%d y:%d \n", sp, (int)sp->xpos, (int)sp->ypos);
        sprite_draw_xy(sp, sp->xpos, sp->ypos);
    }
}

void __not_in_flash("sprite_draw_xy") sprite_draw_xy(sprite_t *sp, uint16_t xpos, uint16_t ypos)
{
    int bytex = (xpos >> 3) - sp->width;
    int pixx = xpos & 0x07;

    int ystart;
    int vstart;
    int ylen;
    int vdiff;
    int x, y;

    uint8_t *vpos;
    uint8_t *sppos;
    char c;

    // printf("sprite_draw_xy: %p x:%d y:%d \n", sp, sp->xpos, sp->ypos);

    if (xpos < 0 || ypos < 0)
        return;

    sp->xpos = xpos;
    sp->ypos = ypos;

    ystart = sp->height - ypos;
    if (ystart < 0)
        ystart = 0;

    ylen = sp->height - ystart;

    vstart = ypos - sp->height;
    if (vstart < 0)
        vstart = 0;

    vdiff = blitheight + sp->height - ypos;
    if (vdiff < sp->height)
    {
        ylen = vdiff;
    }

    vpos = &video_ram[vstart * blitwidth];
    sppos = &sp->data[ystart * sp->width];

    // printf("sprite xpos:%d ypos:%d bytex: %d pixx:%d ystart:%d ylen:%d\n", xpos, ypos, bytex, pixx, ystart, ylen);

    for (y = 0; y < ylen; y++)
    {
        c = 0;
        for (x = 0; x < sp->width; x++)
        {
            if (bytex + x >= 0 && bytex + x < blitwidth)
                vpos[bytex + x] ^= (c | (sppos[x] >> pixx));
            c = (sppos[x]) << (8 - pixx);
        }
        if (bytex + x >= 0 && bytex + x < blitwidth)
            vpos[bytex + x] ^= c;

        vpos += blitwidth;
        sppos += sp->width;
    }
}

void set_blitwidth(int w)
{
    blitwidth = w;
}

void set_blitheight(int h)
{
    blitheight = h;
}
