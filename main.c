/*
 * Copyright (C) 2012-2017  Kevin Balke
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#include <msp430.h>

#include <string.h>

#include "adc.h"
#include "i2c_memdev.h"
#include "servo.h"
#include "simple_io.h"
#include "simple_math.h"

#define COMMIT_MAGIC_NUMBER (0b101)

typedef struct
{
    uint8_t commit : 3;
    uint8_t pad : 5;
    uint8_t pad2;
} control_word_t;

servo_ctl_t shadow_servos;

typedef struct
{
    control_word_t control_word;
    servo_ctl_t servos;
    adc_t pots;
} memmap_t;

static memmap_t memmap;

void i2c_indicate_activity()
{
    P1OUT ^= 0x01;
}

bool busy_flag = true;

bool get_busy_flag()
{
    return busy_flag;
}

int main(void) {
    WDTCTL = WDTPW + WDTHOLD;

    BCSCTL1 = CALBC1_16MHZ;
    DCOCTL = CALDCO_16MHZ;

    BCSCTL2 = DIVS_3;

    INIT_PORT1();
    INIT_PORT2();

    i2c_init_readmem((uint8_t*)&memmap, sizeof(memmap));
    i2c_init_writemem((uint8_t*)&memmap.control_word,
                      sizeof(memmap.control_word) + sizeof(memmap.servos));

    servo_init(&shadow_servos, get_busy_flag);
    memcpy(&memmap.servos, &shadow_servos, sizeof(shadow_servos));
    adc_init(&memmap.pots);

    i2c_init_mem(0x40);

    _BIS_SR(GIE);

    P1DIR |= 0x01;

    uint32_t counter;

    while (1)
    {
        counter = 100000l;

        while(counter--)
        {
            if(!i2c_busy())
            {
                busy_flag = true;

                if(memmap.control_word.commit == COMMIT_MAGIC_NUMBER)
                {
                    memcpy(&shadow_servos, &memmap.servos, sizeof(memmap.servos));
                    memmap.control_word.commit = 0;
                }

                busy_flag = false;
            }
        }

        P1OUT ^= 0x01;
    }

    return 0;
}

