/*
 * i2c_memdev.c
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
 * References in brackets, e.g., [14.2.4], refer to the corresponding section
 * in the MSP430x2xx Family User's Guide
 * (http://www.ti.com/lit/ug/slau144j/slau144j.pdf).
 */

#include <msp430.h>

#include "i2c_memdev.h"

#define USIDIR_IN() do {\
    USICTL0 &= ~USIOE; \
} while(0)

#define USIDIR_OUT() do {\
    USICTL0 |= USIOE; \
} while(0)

#define USICOUNT(x) do {\
        USICNT = (USICNT & 0xF0) | (x); \
} while(0)

#define USIIFG_CLEAR() do {\
    USICTL1 &= ~USIIFG; \
} while(0)

typedef enum
{
    I2CS_IDLE = 0,
    I2CS_RXADDR,
    I2CS_RXADDR_DONE,
    I2CS_RXDATADDR,
    I2CS_RXDATADDR_DONE,
    I2CS_RX,
    I2CS_RX_DONE,
    I2CS_TX,
    I2CS_TX_DONE,
    I2CS_NACK_DONE
} i2c_state_e;

static struct {
    uint8_t *writemem;
    const uint8_t *readmem;
    uint16_t writelen, readlen, idx;
    bool have_address, read;
    bool busy;
    slvaddr_t addr;
    uint8_t chksum;
    i2c_state_e state;
} i2c_state;

/**
 * @brief Determines whether or not the I2C memory is busy (being written to).
 *
 * Check the return value of this function before accessing the writable memory
 * assigned to the driver. Failing to do so can result in a data race where the
 * master has partially updated a multi-byte field in the memory and the memory
 * is then accessed for some other purpose.
 *
 * @return Whether or not the memory is busy.
 */
bool i2c_busy()
{
    /*
     * Handle case where master writes partial memory and then ends
     * transmission, leaving state machine hung in I2CS_RX_DONE with the USISTP
     * flag set.
     */
    if (i2c_state.busy && i2c_state.state == I2CS_RX_DONE && (USICTL1 & USISTP))
        return (i2c_state.busy = false);

    return i2c_state.busy;
}

/* Dummy implementation to keep linker from complaining. */
__attribute__((weak))
void i2c_indicate_activity() {}

/**
 * @brief Initializes the writable memory region.
 *
 * @param mem A pointer to the memory to be made writable via I2C.
 * @param len The length of the memory.
 */
void i2c_init_writemem(uint8_t* mem, uint8_t len)
{
    i2c_state.writemem = mem;
    i2c_state.writelen = (mem) ? (len) : 0;
}

/**
 * @brief Initializes the readable memory region.
 *
 * @param mem A pointer to the memory to be made readable via I2C.
 * @param len The length of the memory.
 */
void i2c_init_readmem(const uint8_t* mem, uint8_t len)
{
    i2c_state.readmem = mem;
    i2c_state.readlen = (mem) ? (len) : 0;
}

/**
 * @brief Resets the I2C memory state machine.
 */
static void i2c_reset()
{
    USIDIR_IN();

    //i2c_state.idx = 0;
    //i2c_state.have_address = false;
    i2c_state.busy = false;
    i2c_state.state = I2CS_IDLE;
    i2c_state.chksum = I2C_CHECKSUM_MAGIC;
}

/**
 * @brief Initializes the I2C memory driver.
 *
 * After calling this function, the driver ISRs will be enabled, and can begin
 * servicing I2C communications immediately. Before calling this function, the
 * read and write memories should be initialized with i2c_init_(read|write)mem.
 *
 * @param slave_addr The slave address the driver should use.
 */
void i2c_init_mem(slvaddr_t slave_addr)
{
    i2c_reset();
    i2c_state.addr = slave_addr;

    /*
     * Enable pins 1.6 and 1.7 for use with the USI module (I2C), MSB first
     * [14.2.4].
     */
    USICTL0 = USIPE6 + USIPE7 + USISWRST;

    // Set the USI module to I2C mode and enable the start condition interrupt
    USICTL1 = USII2C + USISTTIE + USIIE;

    // SCL is inactive high
    USICKCTL = USICKPL;

    // Disable automatic clear control
    USICNT |= USIIFGCC;


    // Enable USI
    USICTL0 &= ~USISWRST;

    // Clear pending flag
    USICTL1 &= ~(USIIFG | USISTTIFG);
}


__attribute__((__interrupt__(USI_VECTOR)))
static void usi_int() {
    _BIC_SR(GIE);

    if (USICTL1 & USISTTIFG)
    {
        if (USICTL1 & USISTP)
        {
            i2c_reset();

            USICTL1 &= ~USISTP;
        }

        i2c_state.state = I2CS_RXADDR;

        USICTL1 &= ~USISTTIFG;

        i2c_indicate_activity();
    }
    else
    {
        USIIFG_CLEAR();
    }

    switch (i2c_state.state)
    {

    case I2CS_IDLE:

        // Should never get here...

        USIIFG_CLEAR();

        break;

    case I2CS_RXADDR:
        i2c_state.state = I2CS_RXADDR_DONE;
        USIDIR_IN();
        USICOUNT(8);
        break;

    case I2CS_RXADDR_DONE:

        if ((USISRL >> 1) == i2c_state.addr)
        {
            i2c_state.read = (USISRL & 0x01);
            USISRL = 0x00;

            if (i2c_state.read)
            {
                if (i2c_state.have_address)
                {
                    i2c_state.state = I2CS_TX;
                }
                else
                {
#ifdef READ_NOADDR_FROM_START
                    i2c_state.state = I2CS_TX;
                    i2c_state.idx = 0;
                    i2c_state.have_address = true;
#else
                    i2c_state.state = I2CS_NACK_DONE;
                    USISRL = 0xFF;
#endif
                }
            }
            else
            {
                if (i2c_state.have_address)
                {
                    i2c_state.state = I2CS_RX;
                }
                else
                {
                    i2c_state.state = I2CS_RXDATADDR;
                }
            }
        }
        else
        {
            i2c_state.state = I2CS_NACK_DONE;
            USISRL = 0xFF;
        }
        USIDIR_OUT();
        USICOUNT(1);
        break;

    case I2CS_RXDATADDR:
        i2c_state.state = I2CS_RXDATADDR_DONE;
        USIDIR_IN();
        USICOUNT(8);
        break;

    case I2CS_RXDATADDR_DONE:
        i2c_state.state = I2CS_RX;

        i2c_state.idx = USISRL;
        i2c_state.have_address = true;

        USIDIR_OUT();
        USISRL = 0x00;
        USICOUNT(1);
        break;

    case I2CS_RX:
        i2c_state.state = I2CS_RX_DONE;
        USIDIR_IN();
        USICOUNT(8);
        break;

    case I2CS_RX_DONE:
        i2c_state.state = I2CS_RX;

        /*
         * After we get the first byte, we can unmark having the address. We do
         * this here as opposed to in I2CS_RX or I2CS_RXADDR_DONE so that we
         * can allow a repeated start condition after the register address is
         * written without forgetting that we got that register address.
         */
        i2c_state.have_address = false;

        if(i2c_state.idx < i2c_state.writelen)
        {
            i2c_indicate_activity();

            i2c_state.busy = true;
            i2c_state.writemem[i2c_state.idx++] = USISRL;
            USISRL = 0x00;
        }
        else
        {
            i2c_state.state = I2CS_NACK_DONE;
            USISRL = 0xFF;
        }

        USIDIR_OUT();
        USICOUNT(1);

        break;

    case I2CS_TX:
        /*
         * If we sent the master a byte already and they didn't ACK, then we
         * can give up now.
         */
        if(!i2c_state.have_address && (USISRL & 0x01))
        {
            i2c_reset();
        }
        else
        {
            i2c_state.state = I2CS_TX_DONE;

            USIDIR_OUT();

            if(i2c_state.idx < i2c_state.readlen)
            {
                i2c_indicate_activity();

                USISRL = i2c_state.readmem[i2c_state.idx++];
                i2c_state.chksum ^= USISRL;
            }
            else
            {
                /*
                 * The master just gets the checksum if it asks for more data
                 * than we have.
                 */
                USISRL = i2c_state.chksum;
            }

            USICOUNT(8);
        }
        break;

    case I2CS_TX_DONE:
        i2c_state.state = I2CS_TX;

        /* See case I2CS_RX_DONE for explanation. */
        i2c_state.have_address = false;

        USIDIR_IN();
        USICOUNT(1);
        break;

    case I2CS_NACK_DONE:

        i2c_reset();

        break;
    }

    _BIS_SR_IRQ(GIE);
}
