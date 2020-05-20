/*
 * i2c_memdev.h
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

#ifndef I2C_MEMDEV_H
#define I2C_MEMDEV_H

#include <stdbool.h>
#include <stdint.h>

/*
 * Comment this to disallow reading without providing a register address. If
 * allowed, the driver assumes the master wants to read from address 0 when it
 * reads without providing an address.
 */
#define READ_NOADDR_FROM_START

#define I2C_CHECKSUM_MAGIC (0xAA)

typedef uint8_t slvaddr_t;

bool i2c_busy();
void i2c_indicate_activity();
void i2c_init_mem(slvaddr_t slave_addr);
void i2c_init_readmem(const uint8_t* mem, uint8_t len);
void i2c_init_writemem(uint8_t* mem, uint8_t len);

#endif // I2C_MEMDEV_H
