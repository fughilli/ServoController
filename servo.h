/*
 * servo.h
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

#ifndef SERVO_H
#define SERVO_H

#include <stdbool.h>
#include <stdint.h>

/*-----Period, in TACLK cycles-----*/
#define PWM_PERIOD (CLOCK_SPEED / (PWM_FREQUENCY * TIMER_A_DIVIDER))

#define DEFAULT_BASEBAND_TIME_US 		(1000ul)
#define DEFAULT_MAXBAND_TIME_US 		(2000ul)

#define DEFAULT_BASEBAND_CLK_TIME 		(((CLOCK_SPEED/1000000ul) * DEFAULT_BASEBAND_TIME_US) / \
                                         (TIMER_A_DIVIDER))
#define DEFAULT_MAXBAND_CLK_TIME 		(((CLOCK_SPEED/1000000ul) * DEFAULT_MAXBAND_TIME_US) / \
                                 (TIMER_A_DIVIDER))

#define DEFAULT_MAXBAND_CLK_TIME_DIFF (DEFAULT_MAXBAND_CLK_TIME - DEFAULT_BASEBAND_CLK_TIME)
#define NUM_SERVOS (2)

typedef struct
{
    uint16_t pos[NUM_SERVOS];
    uint16_t baseband, maxband;
} servo_ctl_t;

void servo_init(servo_ctl_t* control, bool(*ctl_ready)());

#endif // SERVO_H
