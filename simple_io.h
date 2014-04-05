/*
 * simple_io.h
 *
 *  Created on: Jul 11, 2012
 *      Author: Kevin
 *
 *
 * Copyright (C) 2012-2014  Kevin Balke
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

#ifndef SIMPLE_IO_H_
#define SIMPLE_IO_H_

#include "global_const.h"

#if (CLOCK_SPEED_MHz==16)
#define CLOCK_SPEED 16000000	//16*(1000^2)	16MHz
#define CLOCK_TIME_MS 16000		//CLOCK_SPEED/1000
#elif(CLOCK_SPEED_MHz==1)
#define CLOCK_SPEED 1000000		//1000^2			1MHz
#define CLOCK_TIME_MS 1000		//CLOCK_SPEED/1000
#endif

#define INIT_PORT1 								 \
		P1OUT = 0								,\
		P1DIR = 0								// Initialize PORT1 pins
#define INIT_PORT2 								 \
		P2OUT = 0								,\
		P2DIR = 0								// Initialize PORT2 pins
#define set_pin(pin) 							 \
		P1OUT |= (1<<pin)&0xFF					,\
		P2OUT |= (((1<<pin)&0xFF00)>>8) 		// Set PORT1 pin to high
#define clear_pin(pin) 							 \
		P1OUT &= ~(1<<pin)&0xFF					,\
		P2OUT &= ~(((1<<pin)&0xFF00)>>8)		// Set PORT1 pin to low
#define get_pin(pin) 							 \
		(pin>=8)?((P2IN>>(pin-8))&1):((P1IN>>pin)&1)
												// Read the digital level of the PORT1 pin
#define set_pin_output(pin) 					 \
		P1DIR |= (1<<pin)&0xFF					,\
		P2DIR |= (((1<<pin)&0xFF00)>>8)			// Set PORT1 pin to be a digital output
#define set_pin_input(pin) 						 \
		P1DIR &= ~(1<<pin)&0xFF					,\
		P2DIR &= ~(((1<<pin)&0xFF00)>>8)		// Set PORT1 pin to be a digital input
#define set_pin_analog_input(pin) 				 \
		ADC10AE0 |= 1<<pin						,\
		P1DIR &= ~(1<<pin)						// Set PORT1 pin to be an ADC10 analog input
#define select_analog_channel(pin)				 \
		ADC10CTL1 &= ~0xF000					,\
		ADC10CTL1 |= pin*0x1000u				// Select the ADC10 input channel
void delay(long ms);
void delay_ticks(long ticks);

#endif /* SIMPLE_IO_H_ */
