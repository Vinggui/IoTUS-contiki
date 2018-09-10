/*
 * Copyright (c) 2004, Swedish Institute of Computer Science.
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
 * Author: Adam Dunkels <adam@sics.se>
 *
 */
#ifndef CONTIKI_H_
#define CONTIKI_H_

#include "contiki-version.h"
#include "contiki-conf.h"

#ifndef CONTIKI_COMM_NEW_STACK
#include "contiki-default-conf.h"
#endif

#include "sys/process.h"
#include "sys/autostart.h"

#include "sys/timer.h"
#include "sys/ctimer.h"
#include "sys/etimer.h"
#include "sys/rtimer.h"

#include "sys/pt.h"

#include "sys/procinit.h"

#include "sys/loader.h"
#include "sys/clock.h"

#include "sys/energest.h"



// #define POWER_TRACE_RATE                  2
// #define BROADCAST_EXAMPLE                 1
// #define USE_NEW_FEATURES                  0
// #define ALOHA_STYLE                       0
// #define EXP_STAR_LIKE                      0
// #define SINGLE_NODE_NULL                  0
// #define BACKOFF_TIME                      1000


#define POWER_TRACE_RATE                  60
#define BROADCAST_EXAMPLE                 0
#define USE_NEW_FEATURES                  1
#define ALOHA_STYLE                       0
#define EXP_STAR_LIKE                     0
#define SINGLE_NODE_NULL                  0
#define DOUBLE_NODE_NULL                  0
#define MSG_INTERVAL                      30//sec
#define BACKOFF_TIME                      15000
#define TRANSMISSION_CHANCE               100//%
#define KEEP_ALIVE_INTERVAL               30//sec
#define ROUTING_PACKETS_TIMEOUT           30000//msec


extern uint16_t gPkt_tx_successful;
extern uint16_t gpkt_tx_attemps;

#if DOUBLE_NODE_NULL == 1
    #if SINGLE_NODE_NULL != 1
        #error For DOUBLE_NODE_NULL, SINGLE_NODE_NULL has to be 1!
    #endif
#endif
#if ALOHA_STYLE == 1
    #if CONTIKIMAC_CONF_WITH_PHASE_OPTIMIZATION == 1
        #error CONTIKIMAC_CONF_WITH_PHASE_OPTIMIZATION is ON!
    #endif
#elif SINGLE_NODE_NULL == 1
    #if CONTIKIMAC_CONF_WITH_PHASE_OPTIMIZATION == 1
        #error CONTIKIMAC_CONF_WITH_PHASE_OPTIMIZATION is ON!
    #endif
    #if NETSTACK_CONF_RDC != nullrdc_driver
        #error NETSTACK_CONF_RDC is not nullrdc_driver!
    #endif
#else
    #if EXP_STAR_LIKE == 0
        #if CONTIKIMAC_CONF_WITH_PHASE_OPTIMIZATION == 0
            #error CONTIKIMAC_CONF_WITH_PHASE_OPTIMIZATION is OFF!
        #endif
    #endif
#endif

#endif /* CONTIKI_H_ */
