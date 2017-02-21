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

#include "adc.h"
#include "i2c_memdev.h"
#include "servo.h"
#include "simple_io.h"
#include "simple_math.h"

typedef struct
{
    servo_ctl_t servos;
    adc_t pots;
} memmap_t;

static memmap_t memmap;

void i2c_indicate_activity()
{
    P1OUT ^= 0x01;
}

int main(void) {
    WDTCTL = WDTPW + WDTHOLD;

    BCSCTL1 = CALBC1_16MHZ;
    DCOCTL = CALDCO_16MHZ;

    BCSCTL2 = DIVS_3;

    INIT_PORT1();
    INIT_PORT2();

    i2c_init_readmem((uint8_t*)&memmap, sizeof(memmap));
    i2c_init_writemem((uint8_t*)&memmap.servos, sizeof(memmap.servos));

    servo_init(&memmap.servos, i2c_busy);
    adc_init(&memmap.pots);

    i2c_init_mem(0x40);

    _BIS_SR(GIE);

    P1DIR |= 0x01;

    while (1)
    {
        delay(500);

        P1OUT ^= 0x01;
    }

    return 0;
}

