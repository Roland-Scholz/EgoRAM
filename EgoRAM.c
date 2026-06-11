/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*

A0-12  13
D0-7    8
CCTL    1   gpio 21
PHI2    1   gpio 22
RW      1   gpio 23
S4      1   gpio 24
S5      1   gpio 25
    ------
       26
    ======

#define ALL_GPIO_MASK 0x3FFFFFFF
#define ADDR_GPIO_MASK 0x00001FFF
#define DATA_GPIO_MASK 0x001FE000
#define CCTL_GPIO_MASK 0x00200000 // gpio 21
#define PHI2_GPIO_MASK 0x00400000 // gpio 22
#define RW_GPIO_MASK 0x00800000	  // gpio 23
#define S4_GPIO_MASK 0x01000000	  // gpio 24
#define S5_GPIO_MASK 0x02000000	  // gpio 25
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/rand.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/pll.h"
#include "hardware/structs/systick.h"
#include "hardware/vreg.h"
#include "hardware/structs/bus_ctrl.h"
#include "hardware/regs/busctrl.h"

#include "vt100_font_8x8.h"
#include "atari_cart.h"
#include "EgoRAM.h"
#include "cvideo.h"

char c;

int dx_array[EGO_MAX_SPRITES];
int dy_array[EGO_MAX_SPRITES];

static char strbuf[80];

static uint8_t *char_font = (uint8_t *)vt100_font_8x8;
static uint32_t mode;

static volatile uint8_t *video_ram;
static volatile uint8_t *cart_d5xx;
static volatile uint32_t ticks;
static bool uart_init_once;

static uint8_t ego_cmd;
static uint32_t ego_state;
static uint32_t ego_shape_no;
static uint32_t ego_sprite_no;
static uint32_t ego_line_no;
static uint32_t ego_line_cnt;
static uint32_t ego_shape_width;
static uint32_t ego_shape_height;
static uint16_t ego_blitwidth;
static uint16_t ego_blitheight;
static uint16_t ego_line_ptr[256];

static shape_t shape_array[EGO_MAX_SHAPES];
static sprite_t sprite_array[EGO_MAX_SPRITES];

static PIO pio = pio0;

static void systick_init(void)
{
    systick_hw->csr = 0x5;        // Enable + CPU-Clock als Quelle
    systick_hw->rvr = 0x00FFFFFF; // Maximaler Reload-Wert (24-Bit)
}

uint16_t __not_in_flash_func(get_blitwidth)()
{
    return ego_blitwidth;
}

void __not_in_flash_func(set_blitwidth)(uint16_t width)
{
    ego_blitwidth = width;
}

uint16_t __not_in_flash_func(get_blitheight)()
{
    return ego_blitheight;
}

void __not_in_flash_func(set_blitheight)(uint16_t height)
{
    ego_blitheight = height;
}

uint8_t read_vram(uint16_t addr)
{
    return video_ram[addr];
}

void write_vram(uint16_t addr, uint8_t data)
{
    if (addr >= 0x1fff)
    {
        ego_log("error vram addr: %d\n", addr);
    }
    else
    {
        video_ram[addr] = data;
    }
}

void __not_in_flash_func(sprite_draw)(sprite_t *sp)
{

    int xpos = sp->xpos;
    int ypos = sp->ypos;
    int width = shape_array[sp->shape_no].width;
    int height = shape_array[sp->shape_no].height;

    int pixx = xpos & 0x07;
    int bytex = (xpos >> 3);

    int ystart;
    int vstart;
    int ylen;
    int vdiff;
    int x, y;

    uint16_t vpos;
    uint8_t *sppos;
    uint8_t b, c;

    // ego_log("blitwidth: %d, blitheight %d\n", ego_blitwidth, ego_blitheight);
    // ego_log("sprite_draw: shape_no:%d, x:%d y:%d, width:%d, height: %d \n", sp->shape_no, xpos, ypos, width, height);

    /*
    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            ego_log("%02X ", shape_array[sp->shape_no].data[y * width + x]);
        }
        ego_log("\n");
    }
    */

    // if (xpos < 0 || ypos < 0)
    //    return;

    ystart = height - ypos;
    if (ystart < 0)
        ystart = 0;

    ylen = height - ystart;

    vstart = ypos - height;
    if (vstart < 0)
        vstart = 0;

    vdiff = ego_blitheight + height - ypos;
    if (vdiff < height)
    {
        ylen = vdiff;
    }

    sppos = &(shape_array[sp->shape_no].data[ystart * width]);

    // ego_log("sprite xpos:%d ypos:%d bytex: %d pixx:%d ystart:%d ylen:%d vstart:%d \n", xpos, ypos, bytex, pixx, ystart, ylen, vstart);

    for (y = 0; y < ylen; y++)
    {
        c = 0;

        // vpos = &video_ram[ego_line_ptr[vstart + y]];
        vpos = ego_line_ptr[vstart + y] + bytex;

        if (vstart + y >= 192)
            ego_log("vstart too high %d\n", vstart + y);

        for (x = 0; x < width; x++)
        {
            if (bytex + x >= 0 && bytex + x < ego_blitwidth)
            {
                b = read_vram(vpos + x);
                b ^= (c | (sppos[x] >> pixx));
                write_vram(vpos + x, b);
            }
            // vpos[bytex + x] ^= (c | (sppos[x] >> pixx));
            c = (sppos[x]) << (8 - pixx);
        }

        if (bytex + x >= 0 && bytex + x < ego_blitwidth)
        {
            // vpos = &video_ram[ego_line_ptr[vstart + y]];
            // vpos[bytex + x] ^= c;
            vpos = ego_line_ptr[vstart + y] + bytex;
            b = read_vram(vpos + x);
            b ^= c;
            write_vram(vpos + x, b);
        }

        sppos += width;
    }
}

void __not_in_flash_func(render_sprites)(void)
{

    sprite_t *sp;

    for (int i = 0; i < EGO_MAX_SPRITES; i++)
    {
        sp = &sprite_array[i];

        if (sp->enabled)
        {
            sprite_draw(sp);
        }
    }
}

void do_movement()
{
    sprite_t *sp;
    uint16_t xpos, ypos;

    for (int i = 0; i < EGO_MAX_SPRITES; i++)
    {
        sp = &sprite_array[i];

        if (sp->enabled)
        {
            sprite_draw(sp);

            xpos = sp->xpos;
            ypos = sp->ypos;

            xpos += dx_array[i];
            if (xpos >= (ego_blitwidth - shape_array[sp->shape_no].width) << 3 || xpos <= 0)
            {
                dx_array[i] *= -1;
                xpos += dx_array[i];
            }

            ypos += dy_array[i];
            if (ypos >= ego_blitheight || ypos - shape_array[sp->shape_no].height <= 0)
            {
                dy_array[i] *= -1;
                ypos += dy_array[i];
            }

            sp->xpos = xpos;
            sp->ypos = ypos;

            sprite_draw(sp);
        }
    }

    cart_d5xx[EGO_REG_STATUS] &= ~0x80;
}

void __not_in_flash_func(do_data)(uint8_t data)
{
    switch (ego_state)
    {
    case EGO_ST_SHAPE_NO:
        ego_shape_no = data;
        ego_state = EGO_ST_SHAPE_WIDTH;
        break;
    case EGO_ST_SHAPE_WIDTH:
        shape_array[ego_shape_no].width = data;
        ego_state = EGO_ST_SHAPE_HEIGHT;
        break;
    case EGO_ST_SHAPE_HEIGHT:
        shape_array[ego_shape_no].height = data;
        if (shape_array[ego_shape_no].data != NULL)
        {
            free(shape_array[ego_shape_no].data);
        }
        shape_array[ego_shape_no].data = malloc(shape_array[ego_shape_no].width * shape_array[ego_shape_no].height);
        shape_array[ego_shape_no].count = 0;

        ego_state = EGO_ST_SHAPE_DATA;
        break;
    case EGO_ST_SHAPE_DATA:
        shape_array[ego_shape_no].data[shape_array[ego_shape_no].count] = data;
        shape_array[ego_shape_no].count++;

        if (shape_array[ego_shape_no].count == shape_array[ego_shape_no].width * shape_array[ego_shape_no].height)
        {
            ego_state = EGO_ST_IDLE;
        }
        break;
    case EGO_ST_SPRITE_NO:
        ego_sprite_no = data;

        if (ego_cmd == EGO_CMD_SET_SPRITE_X || ego_cmd == EGO_CMD_SET_SPRITE_XY)
            ego_state = EGO_ST_SPRITE_X_LO;
        else if (ego_cmd == EGO_CMD_SET_SPRITE_Y)
            ego_state = EGO_ST_SPRITE_Y_LO;
        else
            ego_state = EGO_ST_SPRITE_SHAPE_NO;
        break;
    case EGO_ST_SPRITE_SHAPE_NO:
        sprite_array[ego_sprite_no].shape_no = data;
        ego_state = EGO_ST_IDLE;
        break;
    case EGO_ST_SPRITE_X_LO:
        sprite_array[ego_sprite_no].xold = sprite_array[ego_sprite_no].xpos;
        sprite_array[ego_sprite_no].xpos = data;
        ego_state = EGO_ST_SPRITE_X_HI;
        break;
    case EGO_ST_SPRITE_X_HI:
        sprite_array[ego_sprite_no].xpos |= data << 8;
        if (ego_cmd == EGO_CMD_SET_SPRITE_XY)
            ego_state = EGO_ST_SPRITE_Y_LO;
        else
            ego_state = EGO_ST_IDLE;
        break;
    case EGO_ST_SPRITE_Y_LO:
        sprite_array[ego_sprite_no].yold = sprite_array[ego_sprite_no].ypos;
        sprite_array[ego_sprite_no].ypos = data;
        ego_state = EGO_ST_SPRITE_Y_HI;
        break;
    case EGO_ST_SPRITE_Y_HI:
        sprite_array[ego_sprite_no].ypos |= data << 8;
        ego_state = EGO_ST_IDLE;
        break;
    case EGO_ST_SPRITE_ENA:
        ego_sprite_no = data;
        sprite_array[ego_sprite_no].enabled = true;
        ego_state = EGO_ST_IDLE;
        break;
    case EGO_ST_SPRITE_DIS:
        ego_sprite_no = data;
        sprite_array[ego_sprite_no].enabled = false;
        ego_state = EGO_ST_IDLE;
        break;
    case EGO_ST_BLITWITH:
        ego_blitwidth = data;
        ego_state = EGO_ST_IDLE;
        break;
    case EGO_ST_BLITHEIGHT:
        ego_blitheight = data;
        ego_state = EGO_ST_IDLE;
        break;
    case EGO_ST_LINE_NO:
        ego_line_no = data;
        ego_line_cnt = 0;
        ego_state = EGO_ST_LINE_DATA_LO;
        break;
    case EGO_ST_LINE_DATA_LO:
        ego_line_ptr[ego_line_cnt] = data;
        ego_state = EGO_ST_LINE_DATA_HI;
        break;
    case EGO_ST_LINE_DATA_HI:
        ego_line_ptr[ego_line_cnt++] |= (data & 0x1f) << 8;
        if (ego_line_cnt >= ego_line_no)
        {
            ego_state = EGO_ST_IDLE;
            ego_log("cnt:%d %04X\n", ego_line_cnt, ego_line_ptr[ego_line_cnt - 1]);
        }
        else
            ego_state = EGO_ST_LINE_DATA_LO;
        break;
    default:
        break;
    }
}

void __not_in_flash_func(do_command)(uint8_t data)
{
    ego_cmd = data;

    switch (ego_cmd)
    {
    case EGO_CMD_RENDER_SPRITES:
        render_sprites();
        break;
    case EGO_CMD_SET_WRITEABLE:
        set_writeable(true);
        cart_d5xx[EGO_REG_STATUS] |= 0x01;
        break;
    case EGO_CMD_SET_READONLY:
        set_writeable(false);
        cart_d5xx[EGO_REG_STATUS] &= ~0x01;
        break;
    case EGO_CMD_ABORT:
        ego_state = EGO_ST_IDLE;
        break;
    case EGO_CMD_SHAPE_DATA:
        ego_state = EGO_ST_SHAPE_NO;
        break;
    case EGO_CMD_DEL_SHAPE:
        break;
    case EGO_CMD_SPRITE_DATA:
        ego_state = EGO_ST_SPRITE_NO;
        break;
    case EGO_CMD_DEL_SPRITE:
        break;
    case EGO_CMD_ENA_SPRITE:
        ego_state = EGO_ST_SPRITE_ENA;
        break;
    case EGO_CMD_DIS_SPRITE:
        ego_state = EGO_ST_SPRITE_DIS;
        break;
    case EGO_CMD_SET_SPRITE_X:
        ego_state = EGO_ST_SPRITE_NO;
        break;
    case EGO_CMD_SET_SPRITE_Y:
        ego_state = EGO_ST_SPRITE_NO;
        break;
    case EGO_CMD_SET_SPRITE_XY:
        ego_state = EGO_ST_SPRITE_NO;
        break;
    case EGO_CMD_SET_BLIT_WIDTH:
        ego_state = EGO_ST_BLITWITH;
        break;
    case EGO_CMD_SET_BLIT_HEIGHT:
        ego_state = EGO_ST_BLITHEIGHT;
        break;
    case EGO_CMD_LINE_PTR:
        ego_state = EGO_ST_LINE_NO;
        break;
    case EGO_CMD_MOVEMENT:
        do_movement();
        break;
    default:
        break;
    }
}

void __not_in_flash_func(core1_main)()
{
    uint32_t msg;
    uint32_t data;
    uint32_t addr;

    systick_init();

    ego_log("core1 started...\n");

    while (true)
    {
        msg = multicore_fifo_pop_blocking();

        ticks = systick_hw->cvr;

        addr = msg & 0xff;
        data = msg >> 16;

        switch (addr)
        {
        case EGO_REG_CMD:
            do_command(data);
            /*
            if (data == EGO_CMD_RENDER_SPRITES)
            {
                ticks = (ticks - systick_hw->cvr) & 0xffffff;
                ego_log("addr: %04X, data: %02X, ticks: %d\n", msg & 0xffff, msg >> 16, ticks);
            }
            */
            break;
        case EGO_REG_DATA:
            do_data(data);
            break;
        default:
            break;
        }

        cart_d5xx[EGO_REG_STATUS] &= ~0x80;
    }
}

void __not_in_flash_func(putChar)(int xpos, int ypos, uint8_t c)
{
    // ego_log("%p %02X ", (ypos * ego_blitwidth * 8) + xpos, c);

    uint8_t volatile *pscreen = &video_ram[(ypos * ego_blitwidth * 8) + xpos];
    uint8_t *pchar = &char_font[c << 3];

    for (int i = 0; i < 8; i++)
    {
        *pscreen = *pchar;
        pchar++;
        pscreen += ego_blitwidth;
    }
}

uint8_t read_d5xx(uint8_t addr)
{
    return cart_d5xx[addr];
}

void __not_in_flash_func(write_d5xx)(uint8_t addr, uint8_t data)
{

    cart_d5xx[EGO_REG_STATUS] |= 0x80;

    uint32_t msg = data << 16 | addr;
    multicore_fifo_push_blocking(msg);
}

void __not_in_flash_func(ego_log)(const char *format, ...)
{

    va_list args;

    va_start(args, format);
    vsnprintf(strbuf, sizeof(strbuf), format, args);
    va_end(args);

    if (mode == EGO_MODE_USB)
    {
        while (!stdio_usb_connected())
        {
            sleep_ms(100);
        }

        printf("%s", strbuf);
    }
    else
    {
        if (!uart_init_once)
        {
            uart_init_once = true;
            uart_init(EGO_UART_ID, EGO_BAUD_RATE);
            gpio_set_function(EGO_UART_TX_PIN, GPIO_FUNC_UART);
            gpio_set_function(EGO_UART_RX_PIN, GPIO_FUNC_UART);
        }

        uart_puts(EGO_UART_ID, strbuf);
    }
}

void test()
{
    int width = 40;
    int height = 200;
    int x, y, c;
    uint16_t ptr = 0;

    write_d5xx(EGO_REG_CMD, EGO_CMD_SET_BLIT_WIDTH);
    write_d5xx(EGO_REG_DATA, (uint8_t)width);
    write_d5xx(EGO_REG_CMD, EGO_CMD_SET_BLIT_HEIGHT);
    write_d5xx(EGO_REG_DATA, (uint8_t)height);

    write_d5xx(EGO_REG_CMD, EGO_CMD_LINE_PTR);
    write_d5xx(EGO_REG_DATA, 200);
    for (x = 0; x < 200; x++)
    {
        write_d5xx(EGO_REG_DATA, ptr & 0xff);
        write_d5xx(EGO_REG_DATA, ptr >> 8);
        ptr += width;
    }

    for (x = 0; x < EGO_MAX_SPRITES; x++)
    {
        dx_array[x] = 1;
        if (get_rand_32() & 0x01)
            dx_array[x] *= -1;

        dy_array[x] = 1;
        if (get_rand_32() & 0x01)
            dy_array[x] *= -1;
    }

    c = 0;
    for (y = 0; y < (height >> 3); y++)
    {
        for (x = 0; x < width; x++)
        {
            putChar(x, y, c);
            c++;
        }
    }

    write_d5xx(EGO_REG_CMD, EGO_CMD_SHAPE_DATA);
    write_d5xx(EGO_REG_DATA, 0x00); // shape number
    write_d5xx(EGO_REG_DATA, 0x01); // shape width
    write_d5xx(EGO_REG_DATA, 0x08); // shape height
    write_d5xx(EGO_REG_DATA, 0xff); // shape data
    write_d5xx(EGO_REG_DATA, 0xff); // shape data
    write_d5xx(EGO_REG_DATA, 0xff); // shape data
    write_d5xx(EGO_REG_DATA, 0xff); // shape data
    write_d5xx(EGO_REG_DATA, 0xff); // shape data
    write_d5xx(EGO_REG_DATA, 0xff); // shape data
    write_d5xx(EGO_REG_DATA, 0xff); // shape data
    write_d5xx(EGO_REG_DATA, 0xff); // shape data

    for (int i = 0; i < EGO_MAX_SPRITES; i++)
    {
        write_d5xx(EGO_REG_CMD, EGO_CMD_SPRITE_DATA);
        write_d5xx(EGO_REG_DATA, i);    // sprite number
        write_d5xx(EGO_REG_DATA, 0x00); // shape number

        write_d5xx(EGO_REG_CMD, EGO_CMD_SET_SPRITE_XY);
        write_d5xx(EGO_REG_DATA, i);                            // sprite number
        write_d5xx(EGO_REG_DATA, get_rand_32() % (width << 3)); // sprite X lo
        write_d5xx(EGO_REG_DATA, 0x00);                         // sprite X hi
        write_d5xx(EGO_REG_DATA, get_rand_32() % height);       // sprite Y lo
        write_d5xx(EGO_REG_DATA, 0x00);                         // sprite Y hi

        write_d5xx(EGO_REG_CMD, EGO_CMD_ENA_SPRITE);
        write_d5xx(EGO_REG_DATA, i); // sprite number
    }

    write_d5xx(EGO_REG_CMD, EGO_CMD_RENDER_SPRITES);

    while (true)
    {
        c = getchar();
        printf("%c", c);

        /*
        write_d5xx(EGO_REG_CMD, EGO_CMD_RENDER_SPRITES);
        while (read_d5xx(EGO_REG_STATUS) & 0x80)
            ;

        sprite_array[0].xpos++;
        sprite_array[0].ypos++;

        write_d5xx(EGO_REG_CMD, EGO_CMD_RENDER_SPRITES);
        while (read_d5xx(EGO_REG_STATUS) & 0x80)
            ;
        */
    }
}

void set_core0_highest_bus_priority()
{
    // Setzt die Bus-Priorität für Kern 0 auf hoch (1) und Kern 1 auf niedrig (0)
    bus_ctrl_hw->priority = BUSCTRL_BUS_PRIORITY_PROC0_BITS;
}

int main()
{
    vreg_set_voltage(VREG_VOLTAGE_1_20);
    sleep_ms(100);
    set_sys_clock_khz(250000, true);
    sleep_ms(100);
    systick_init();

    stdio_init_all();

    // check to see if we are plugged into Atari 8-bit
    // by checking for high on PHI2 gpio for xxx ms
    gpio_init(ATARI_PHI2_PIN);
    gpio_set_dir(ATARI_PHI2_PIN, GPIO_IN);

    uart_init_once = false;
    mode = EGO_MODE_USB;

    while (to_ms_since_boot(get_absolute_time()) < 300)
    {
        if (gpio_get(ATARI_PHI2_PIN))
        {
            mode = EGO_MODE_ATARI;
        }
    }

    video_ram = get_cart_ram();
    cart_d5xx = get_cart_d5xx();

    memset((void *)video_ram, 0x01, 4 * 1024);
    memset((void *)video_ram + 0x1000, 0x10, 4 * 1024);
    memset((void *)video_ram + 0x2000, 0xaa, 8 * 1024);
    memset((void *)sprite_array, 0x00, sizeof(sprite_array));
    memset((void *)shape_array, 0x00, sizeof(shape_array));

    ego_log("\033[2J\033[HEgoRAM starting...\n");
    if (mode == EGO_MODE_USB)
        ego_log("mode: USB\n");
    else
        ego_log("mode: ATARI\n");

    ego_log("sys_clock: %d\n", clock_get_hz(clk_sys));
    ego_log("video_ram at %p\n", video_ram);
    ego_log("cart_d5xx at %p\n", cart_d5xx);

    for (int x = 0; x < EGO_MAX_SPRITES; x++)
    {
        dx_array[x] = 1;
        if (get_rand_32() & 0x01)
            dx_array[x] *= -1;

        dy_array[x] = 1;
        if (get_rand_32() & 0x01)
            dy_array[x] *= -1;
    }

    multicore_launch_core1(core1_main);

    if (mode == EGO_MODE_ATARI)
    {
        // irq_set_enabled(USBCTRL_IRQ, false);
        uint32_t status = save_and_disable_interrupts();
        set_core0_highest_bus_priority();
        atari_cart_main();
    }

    initialise_cvideo(pio0);

    test();
}
