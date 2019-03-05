/*
 * Copyright (c) 2006, Swedish Institute of Computer Science.
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
 */

/**
 * \file
 *         A very simple Contiki application showing how Contiki programs look
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "contiki.h"
#include "core/dev/serial-line.h"
#include "dev/leds.h"
#include "powertrace.h"
#include "rpl-like-net.h"
#include <stdio.h> /* For printf() */
#include "random.h"

/*---------------------------------------------------------------------------*/
PROCESS(hello_world_process, "nd process");
AUTOSTART_PROCESSES(&hello_world_process);
/*---------------------------------------------------------------------------*/

static uint8_t selfMsg[20];
static uint8_t gPkt_created = 0;

static linkaddr_t addrThis;

void msg_confirm(int status, int num_tx) {
    // printf("message processed %u\n",status);
}

void msg_input(const linkaddr_t *source) {
    printf("message received %u bytes: %s\n",packetbuf_datalen(), (uint8_t *)packetbuf_dataptr());
}

static void
send_msg(void *ptr)
{
  if(gPkt_created >= MAX_GENERATED_PKT) {
      return;
  }
  gPkt_created++;
  printf("App sending to 1\n");
  TIC();
  packetbuf_copyfrom(selfMsg, 20);
  //addr.u8[0] = nodeToSend;
  //addr.u8[1] = 0;
#if BROADCAST_EXAMPLE == 1
  packetbuf_set_addr(PACKETBUF_ADDR_SENDER, &linkaddr_node_addr);
#else
  packetbuf_set_addr(PACKETBUF_ADDR_RECEIVER, &addrThis);
  packetbuf_set_addr(PACKETBUF_ADDR_SENDER, &linkaddr_node_addr);
#endif
  rpllikenet_output();
}

PROCESS_THREAD(hello_world_process, ev, data) {
    PROCESS_BEGIN();

    //leds_init();
    //leds_off(LEDS_ALL);

    rpllikenet_signup(msg_confirm, msg_input);

    static uint8_t selfAddrValue;

    selfAddrValue = linkaddr_node_addr.u8[0];

    sprintf((char *)selfMsg, "%02u  %02u  %02u %02u %02u %02u+", selfAddrValue,
                                                               selfAddrValue,
                                                               selfAddrValue,
                                                               selfAddrValue,
                                                               selfAddrValue,
                                                               selfAddrValue);

    // sprintf((char *)selfMsg, "+%u %u %u %u %u %u %u %u %u %u %u %u %u %u +", selfAddrValue,
    //                                                                            selfAddrValue,
    //                                                                            selfAddrValue,
    //                                                                            selfAddrValue,
    //                                                                            selfAddrValue,
    //                                                                            selfAddrValue,
    //                                                                            selfAddrValue,
    //                                                                            selfAddrValue,
    //                                                                            selfAddrValue,
    //                                                                            selfAddrValue,
    //                                                                            selfAddrValue,
    //                                                                            selfAddrValue,
    //                                                                            selfAddrValue,
    //                                                                            selfAddrValue);

    // sender addr info...
    //linkaddr_t addr;
    //
    addrThis.u8[0] = 1;
    addrThis.u8[1] = 0;

    /* Start powertracing, once every two seconds. */
    powertrace_start(CLOCK_SECOND * POWER_TRACE_RATE);
    
    static struct ctimer sendMsgTimer;
    static struct etimer timer;
    // set the etimer module to generate an event in one second.
    etimer_set(&timer, CLOCK_CONF_SECOND*MSG_INTERVAL);

    for(;;) {
        //uint8_t nodeToSend = n%7 + 2;

        if(ev == serial_line_event_message) {
            // printf("got %s\n", (uint8_t *)data);
            powertrace_stop();
            powertrace_print("ND");
            break;
        }

#if SINGLE_NODE_NULL == 0
        if(!linkaddr_cmp(&addrThis, &linkaddr_node_addr)) {
#else
          {
#endif


#if EXP_ONE_NODE_GEN > 0
        if(selfAddrValue != EXP_ONE_NODE_GEN) {
          PROCESS_WAIT_EVENT();
          continue;
        }
#endif
          // send_msg(NULL);
          #if SINGLE_NODE_NULL == 1
            #if DOUBLE_NODE_NULL == 1
              if(!linkaddr_cmp(&addrThis, &linkaddr_node_addr)) {
                // send_msg(NULL);
              }
            #else
              // send_msg(NULL);
            #endif
          #else
            uint32_t backoff = (CLOCK_SECOND*(2000+(random_rand()%BACKOFF_TIME)));
            backoff /= 1000;//ms
            // ctimer_set(&sendMsgTimer, backoff, send_msg, NULL);
          #endif

        }
        
        PROCESS_WAIT_EVENT();
        etimer_reset(&timer);
    }



    PROCESS_END();
}
/*---------------------------------------------------------------------------*/
