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
#include <stdio.h> /* For printf() */
#include "iotus-api.h"
#include "random.h"

#include "global-functions.h"
/*---------------------------------------------------------------------------*/
#define MSG_INTERVAL        2//ec

PROCESS(hello_world_process, "Test process");
AUTOSTART_PROCESSES(&hello_world_process);
/*---------------------------------------------------------------------------*/

static void
app_packet_confirm(iotus_packet_t *packet, iotus_netstack_return returnAns)
{
    printf("message processed %u\n", returnAns);
}

static void
app_packet_handler(iotus_packet_t *packet)
{
    printf("message received %u bytes: %s\n", packet_get_payload_size(packet), packet_get_payload_data(packet));
}

PROCESS_THREAD(hello_world_process, ev, data) {
    PROCESS_BEGIN();

    //leds_init();
    //leds_off(LEDS_ALL);


    static struct etimer timer;
    // set the etimer module to generate an event in one second.
    etimer_set(&timer, CLOCK_CONF_SECOND*MSG_INTERVAL);

    IOTUS_CORE_START(0,0,contikiMAC,0);
    packet_set_interface_functions(app_packet_confirm,app_packet_handler);

    static uint8_t selfAddrValue;
    selfAddrValue = addresses_self_get_pointer(IOTUS_ADDRESSES_TYPE_ADDR_SHORT)[0];
    static uint8_t selfMsg[20];

    sprintf((char *)selfMsg, "%u %u %u %u %u %u %u %u %u %u+", selfAddrValue,
                                                               selfAddrValue,
                                                               selfAddrValue,
                                                               selfAddrValue,
                                                               selfAddrValue,
                                                               selfAddrValue,
                                                               selfAddrValue,
                                                               selfAddrValue,
                                                               selfAddrValue,
                                                               selfAddrValue);

    static uint8_t n = 0;
    for(;;) {
        PROCESS_WAIT_EVENT();

        //leds_toggle(LEDS_ALL);
        //if(linkaddr_node_addr.u8[0] == 1) {
            //send_wireless_packet(MESSAGE_TO_ROOT, &addr, NULL, "Oi!", 3);
        //}
        
        if(selfAddrValue != 1 &&
           random_rand()%100 > 75) {
            n++;
            uint8_t nodeAddr = 1;//n%7 + 2;
            printf("App sending to %u\n", nodeAddr);
            
            uint8_t dest[2];
            dest[0] = nodeAddr;
            dest[1] = 0;
            iotus_node_t *destNode = nodes_update_by_address(IOTUS_ADDRESSES_TYPE_ADDR_SHORT, dest);
            if(destNode != NULL) {
                iotus_initiate_msg(
                        20,
                        selfMsg,
                        PACKET_PARAMETERS_WAIT_FOR_ACK | PACKET_PARAMETERS_ALLOW_PIGGYBACK,
                        IOTUS_PRIORITY_APPLICATION,
                        5000,
                        destNode);
            }
        }

        etimer_reset(&timer);
        iotus_allow_sleep(TRUE);
    }



    PROCESS_END();
}
/*---------------------------------------------------------------------------*/
