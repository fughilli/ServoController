/*
 * servo.c
 *
 * Copyright (C) 2016-2017  Kevin Balke
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

#include "servo.h"

#include <msp430.h>
#include <string.h>

#include "simple_io.h"

#define PWM_FREQUENCY (50)
#define TIMER_A_DIVIDER (32)

const uint8_t PWM_PINS[] = { 1, 2 };
#define DEFAULT_CENTER_POS (DEFAULT_MAXBAND_CLK_TIME_DIFF/2)

static servo_ctl_t* servo_ctl;
static servo_ctl_t servo_ctl_buffer;

static uint8_t current_servo;
static uint16_t norm_period, last_period;

static bool(*servo_ctl_busy)();

static uint16_t current_servo_pos;

void servo_init(servo_ctl_t* control, bool(*ctl_busy)())
{
    servo_ctl = control;
    servo_ctl_busy = ctl_busy;

    norm_period = PWM_PERIOD / NUM_SERVOS;
    last_period = PWM_PERIOD - (((uint32_t)norm_period) * (NUM_SERVOS - 1));

    // Timer counts for 0
    norm_period--;
    last_period--;
    current_servo = 0;

    TA0CTL |= TACLR;
    TA0CTL = TASSEL_2 | ID_2;
    TA0CCTL1 |= CCIE;
    TA0CCTL0 |= CCIE;
    TA0CCR0 = norm_period;
    TA0CCR1 = DEFAULT_CENTER_POS;

    servo_ctl_buffer.baseband = DEFAULT_BASEBAND_CLK_TIME;
    servo_ctl_buffer.maxband = DEFAULT_MAXBAND_CLK_TIME;

    for (uint8_t i = 0; i < NUM_SERVOS; i++) {
        set_pin_output(PWM_PINS[i]);
        servo_ctl_buffer.pos[i] = DEFAULT_CENTER_POS;
    }

    memcpy(control, &servo_ctl_buffer, sizeof(servo_ctl_t));

    TA0CTL |= MC_1;
}

__attribute__((__interrupt__(TIMER0_A0_VECTOR)))
void ISR_timer0_a0()
{
    _BIC_SR(GIE);

    current_servo++;
    if(current_servo == NUM_SERVOS)
    {
        current_servo = 0;
    }

    set_pin(PWM_PINS[current_servo]);

    /*
     * If the control structure is not locked, go ahead and access it
     * (Also access if there was no busy query function specified)
     */
    if(!servo_ctl_busy || !servo_ctl_busy())
    {
        current_servo_pos = servo_ctl->pos[current_servo];
        servo_ctl_buffer.pos[current_servo] = current_servo_pos;
        memcpy(&servo_ctl_buffer.baseband, &servo_ctl->baseband, 4);
    }
    else
    {
        current_servo_pos = servo_ctl_buffer.pos[current_servo];
    }

    // Clamp servo position
    uint16_t maxband_diff = servo_ctl_buffer.maxband -
                            servo_ctl_buffer.baseband;
    if(current_servo_pos > maxband_diff)
        current_servo_pos = maxband_diff;

    TA0CCR1 = servo_ctl_buffer.baseband + current_servo_pos;

    _BIS_SR_IRQ(GIE);
}

__attribute__((__interrupt__(TIMER0_A1_VECTOR)))
static void ISR_timer0_a1() {
    _BIC_SR(GIE);

    switch(TAIV)
    {
        case 0x02:
            clear_pin(PWM_PINS[current_servo]);

            if(current_servo == 0)
            {
                TA0CCR0 = last_period;
            } else {
                TA0CCR0 = norm_period;
            }
            break;

        default:

            break;
    }

    _BIS_SR_IRQ(GIE);
}
