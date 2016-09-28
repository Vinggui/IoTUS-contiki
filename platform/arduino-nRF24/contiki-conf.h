/*
 * Copyright (c) 2009, University of Colombo School of Computing
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
 *         Kernel configuration for Arduino
 *
 * \author
 * 	   Ilya Dmitrichenko <errordeveloper@gmail.com>
 *         Kasun Hewage <kasun.ch@gmail.com>
 */

#ifndef __CONTIKI_CONF_H__
#define __CONTIKI_CONF_H__

#define HAVE_STDINT_H
#include "avrdef.h"

#include "Arduino.h"
#include "platform-conf.h"

/* These routines are not part of the contiki core but can be enabled in cpu/avr/clock.c */
void clock_delay_msec(uint16_t howlong);

/* Network setup for non-IPv6 (rime). */

#define NETSTACK_CONF_NETWORK rime_driver
#define NETSTACK_CONF_MAC     nullmac_driver
#define NETSTACK_CONF_RDC     nullrdc_driver
#define NETSTACK_CONF_FRAMER  framer_nullmac
#define NETSTACK_CONF_RADIO   nRF24_driver


#define COLLECT_NEIGHBOR_CONF_MAX_NEIGHBORS      32

#define PACKETBUF_CONF_ATTRS_INLINE 1

#ifndef RF_CHANNEL
#define RF_CHANNEL              26
#endif /* RF_CHANNEL */

#define CONTIKIMAC_CONF_BROADCAST_RATE_LIMIT 0

#define SERIAL_LINE_CONF_BUFSIZE 64

#define WITH_ASCII 1

#define PROCESS_CONF_NUMEVENTS 8
#define PROCESS_CONF_STATS 1


#define WITH_NULLMAC 0

#define CCIF
#define CLIF

/* The process names are not used to save RAM */
#define PROCESS_CONF_NO_PROCESS_NAMES 1


typedef unsigned short clock_time_t;
typedef unsigned short uip_stats_t;
typedef unsigned long off_t;


#endif /* __CONTIKI_CONF_H__ */
