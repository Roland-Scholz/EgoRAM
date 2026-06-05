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
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/rand.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/pll.h"
#include "hardware/structs/systick.h"
#include "hardware/vreg.h"
#include "cvideo.h"

PIO pio = pio0;
char c;
int posx, posy;
char *vram;

sprite_t *sp0;
sprite_t *sp_array[MAX_SPRITES];
int dx_array[MAX_SPRITES];
int dy_array[MAX_SPRITES];
int sp_cnt;

// #define SYSCLOCK 250000000
//  #define SYSCLOCK 177333333

static void systick_init(void)
{
    systick_hw->csr = 0x5;        // Enable + CPU-Clock als Quelle
    systick_hw->rvr = 0x00FFFFFF; // Maximaler Reload-Wert (24-Bit)
}

void __not_in_flash("core1_main") core1_main()
{
    printf("core1 started...\n");
    systick_init();

    while (true)
    {
        // Kern 1 legt sich HIER schlafen, bis Kern 0 etwas sendet!
        uint32_t signal = multicore_fifo_pop_blocking();
        jiffy_callback();
    }
}

int main()
{
    int first = true;
    uint32_t ticks;

    vreg_set_voltage(VREG_VOLTAGE_1_20);
    sleep_ms(100);

    set_sys_clock_khz(250000, true);

    sleep_ms(100);

    systick_init();
    stdio_init_all();

    sleep_ms(2000);
    printf("\033[2J\033[Hpico-pal starting...\n");

    set_blitwidth(80);
    set_blitheight(200);

    for (int i = 0; i < MAX_SPRITES; i++)
    {
        sp_array[i] = sprite_new(3, 16);
        sp_array[i]->xpos = get_rand_32() % 639;
        sp_array[i]->ypos = get_rand_32() % 199;

        dx_array[i] = 1;
        if (get_rand_32() & 0x01)
            dx_array[i] *= -1;

        dy_array[i] = 1;
        if (get_rand_32() & 0x01)
            dy_array[i] *= -1;
    }
    sp_cnt = MAX_SPRITES;

    initialise_cvideo(pio);

    for (int i = 0; i < PAL_WIDTH * PAL_HEIGHT; i++)
    {
        putChar(i % PAL_WIDTH, i / PAL_WIDTH, i);
    }

    sprite_draw_all();
    multicore_launch_core1(core1_main);

    while (true)
    {

        c = ' ';
        if (!first)
        {
            c = getchar();
            // sprite_draw(sp0, posx, posy);
        }

        if (c == 'x')
        {
            posx++;
        }
        if (c == 'y')
        {
            posy++;
        }
        if (c == 's')
        {
            posx--;
            if (posx < 0)
                posx = 0;
        }
        if (c == 'a')
        {
            posy--;
            if (posy < 0)
                posy = 0;
        }

        ticks = systick_hw->cvr;
        // sprite_draw(sp0, posx, posy);

        printf("ticks: %d\n", ticks - systick_hw->cvr);
        first = false;
    }
}
