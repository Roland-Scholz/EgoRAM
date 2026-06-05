//
// Title:	        Pico FBAS Video Output
//

#pragma once

#include "hardware/clocks.h"

// 14.5 Mhz pixelclock	14.500.000 / 15625 = 928
// 17734476 und 1135 pixel
//   1 pixel = 69ns
// 928 pixels per line
//
// 1.65uS frontporch	24 pixels			6  bytes
// 4,7uS hsync			68 pixels			17 bytes
// 8 uS backporch    	116 pixels			29 bytes
// pause				72 pixels			17 bytes	== 61 bytes
// start visible                           160 bytes
// end = 640+244 = 884                      12 bytes
//                                         232 bytes
#define GPIO_BASE 0
#define sm_clock 0
#define sm_sync 1 // State machine number in the PIO for the sync data

#define PALBURST 4433619
#define PIXCLOCK (14500000)
#define PIXPERLINE ((int)(PIXCLOCK * 0.000064))
#define BITSPERPIXEL 2
#define BYTESPERLINE ((int)(PIXPERLINE * BITSPERPIXEL / 8))

#define SYNC 0x00
#define BLACK 0x55
#define GREY 0xaa
#define WHITE 0xff

#define HSYNCSTART 0
#define HSYNCLEN_PIX ((int)(PIXCLOCK * 0.0000047))
#define HSYNCLEN ((int)(PIXCLOCK * 0.0000047 * BITSPERPIXEL / 8))
#define BACKPORCH ((int)(PIXCLOCK * 0.000006 * BITSPERPIXEL / 8))
#define BORDER ((int)(PIXCLOCK * 0.000004 * BITSPERPIXEL / 8))
#define DATASTART ((int)(HSYNCLEN + BACKPORCH + BORDER))
#define GREY_OFFSET 7
#define BORDERSCANLINES 16

#define PAL_WIDTH 80 // define 80 columns by 25 rows
#define PAL_HEIGHT 25

#define CURSOR_TIME 25
#define PAL true
#define NTSC false
#define GRAPH true
// #define uint8_t false

#define MAX_SPRITES 64

static void cvideo_configure_pio_dma(PIO pio, uint sm, uint dma_channel, uint transfer_size, size_t buffer_size, irq_handler_t handler);
static void cvideo_dma_handler(void);
static void generate_line(uint8_t *buffer);
static void generate_synclines();

typedef struct
{
	uint8_t enabled;
	uint8_t width;
	uint8_t height;
	uint16_t xpos;
	uint16_t ypos;
	uint8_t *data;
} sprite_t;

void initialise_cvideo(PIO);
uint8_t *getVideoRam(void);
void setGraphicsMode(int mode);
void setPalNtscMode(int pal);
void change_divisior(int d);
void putChar(int posx, int posy, uint8_t c);
sprite_t *sprite_new(uint8_t width, uint8_t height);
void sprite_free(sprite_t *sp);
void sprite_draw_xy(sprite_t *sp, uint16_t x, uint16_t y);
void sprite_draw_all();
void sprite_draw(sprite_t *sp);
void set_blitwidth(int w);
void set_blitheight(int h);
void jiffy_callback(void);
void putChar(int xpos, int ypos, uint8_t c);
