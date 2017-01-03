/*
 * Copyright (c) 2006, Technical University of Munich
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 * @(#)$$
 */

/**
 * \file
 *         Configuration for AVR Zigbit Contiki kernel
 *
 * \author
 *         Simon Barner <barner@in.tum.de>
 */

#ifndef CONTIKI_CONF_H_
#define CONTIKI_CONF_H_

/* Platform name, type, and MCU clock rate */
#define PLATFORM_NAME  "Zigbit"
#define PLATFORM_TYPE  ZIGBIT
#ifndef F_CPU
#define F_CPU          8000000UL
#endif

#include <stdint.h>
#include <avr/eeprom.h>

/* Skip the last four bytes of the EEPROM, to leave room for things
 * like the avrdude erase count and bootloader signaling. */
#define EEPROM_CONF_SIZE		((E2END + 1) - 4)

/* The AVR tick interrupt usually is done with an 8 bit counter around 128 Hz.
 * 125 Hz needs slightly more overhead during the interrupt, as does a 32 bit
 * clock_time_t.
 */
/* Clock ticks per second */
#define CLOCK_CONF_SECOND 125

typedef uint32_t clock_time_t;
#define CLOCK_LT(a,b)  ((int32_t)((a)-(b)) < 0)

/* These routines are not part of the contiki core but can be enabled in cpu/avr/clock.c */
void clock_delay_msec(uint16_t howlong);
void clock_adjust_ticks(clock_time_t howmany);

/* COM port to be used for SLIP connection */
#define SLIP_PORT RS232_PORT_0

/* Pre-allocated memory for loadable modules heap space (in bytes)*/
#define MMEM_CONF_SIZE 256

/* Use the following address for code received via the codeprop
 * facility
 */
#define EEPROMFS_ADDR_CODEPROP 0x8000

#define CCIF
#define CLIF
#ifndef CC_CONF_INLINE
#define CC_CONF_INLINE inline
#endif

#define LINKADDR_CONF_SIZE       8

/* No radio cycling */
#define NETSTACK_CONF_NETWORK       edytee_net_driver
#define NETSTACK_CONF_MAC           edytee_mac_driver
#define NETSTACK_CONF_RDC           edytee_rdc_driver
#define NETSTACK_CONF_FRAMER        framer_edytee_802154
#define NETSTACK_CONF_RADIO         rf230_driver
#define CHANNEL_802_15_4            26
#define RF230_CONF_AUTOACK          1
#define RF230_CONF_FRAME_RETRIES    2

#define WITH_NULL_LLSEC                  1
#define CONTIKI_WITHOUT_NETWORK          1
#define UIP_CONF_TCP                     0
#define UIP_CONF_UDP                     0
#define WITH_NULLMAC 0


/* These names are deprecated, use C99 names. */
/*
typedef unsigned char u8_t;
typedef unsigned short u16_t;
typedef unsigned long u32_t;
typedef int32_t s32_t;
*/
typedef unsigned short uip_stats_t;
typedef unsigned long off_t;


/* The process names are not used to save RAM */
#define PROCESS_CONF_NO_PROCESS_NAMES 1


#endif /* CONTIKI_CONF_H_ */
