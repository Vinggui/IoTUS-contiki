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

#include <stdio.h> /* For printf() */
#include "contiki.h"
#include "core/dev/serial-line.h"
#include "dev/button-sensor.h"
#include "powertrace.h"
#include "iotus-api.h"
#include "piggyback.h"
#include "random.h"
#include "tree_manager.h"

#include "global-functions.h"
/*---------------------------------------------------------------------------*/

PROCESS(hello_world_process, "nd exem");
AUTOSTART_PROCESSES(&hello_world_process);


static uint8_t selfMsg[20];
static uint8_t gPkt_created = 0;
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
static void
receive_piggyback_cb(iotus_packet_t *packet, uint8_t size, uint8_t *data)
{
  // SAFE_PRINTF_LOG_INFO("nd %p sent %u", packet, returnAns);
  printf("App piggy:%s\n", data);
}


static void
app_packet_confirm(iotus_packet_t *packet, iotus_netstack_return returnAns)
{
    printf("message processed %u\n", returnAns);
    // printf("Packet %u App del %u\n", packet->pktID, returnAns);

    packet_destroy(packet);
    
}

/*---------------------------------------------------------------------------*/
static void
app_packet_handler(iotus_packet_t *packet)
{
    printf("message received %u bytes: %s\n", packet_get_payload_size(packet), packet_get_payload_data(packet));
}

/*---------------------------------------------------------------------------*/
static void
send_app_msg(void *ptr) {
    // if(gPkt_created >= MAX_GENERATED_PKT) {
    //     return;
    // }
    // gPkt_created++;

    uint8_t address2[2] = {1,0};
    iotus_node_t *targetNode = nodes_get_node_by_address(IOTUS_ADDRESSES_TYPE_ADDR_SHORT, address2);
    

// TIC();
#if BROADCAST_EXAMPLE == 0
    // uint8_t dest[2];
    // dest[0] = nodeAddr;
    // dest[1] = 0;
    //iotus_node_t *destNode = nodes_update_by_address(IOTUS_ADDRESSES_TYPE_ADDR_SHORT, dest);
    if(iotus_get_layer_assigned_for(IOTUS_CHORE_MSG_TO_SINK) != 0) {
      //Create piggyback instead
      printf("App sending to 1 (piggyback)\n");
      piggyback_create_piece(20, selfMsg, IOTUS_PRIORITY_APPLICATION, targetNode, 29000);
    } else {
      printf("App sending to 1\n");
      if(targetNode != NULL) {
          iotus_initiate_msg(
                  20,
                  selfMsg,
                  PACKET_PARAMETERS_WAIT_FOR_ACK | PACKET_PARAMETERS_ALLOW_PIGGYBACK,
                  5000,
                  targetNode);
      }
    }
#else
    iotus_initiate_msg(
            20,
            selfMsg,
            0,
            5000,
            NODES_BROADCAST);
#endif
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(hello_world_process, ev, data) {
  PROCESS_BEGIN();

  //leds_init();
  //leds_off(LEDS_ALL);


  static struct ctimer sendTimer;
  static struct etimer timer;

  IOTUS_CORE_START(0,0,0,0);//contikiMAC,0);
  iotus_set_interface_functions(app_packet_confirm,app_packet_handler);

  static uint8_t selfAddrValue;
  selfAddrValue = addresses_self_get_pointer(IOTUS_ADDRESSES_TYPE_ADDR_SHORT)[0];

  sprintf((char *)selfMsg, "%02u  %02u  %02u %02u %02u %02u+", selfAddrValue,
                                                             selfAddrValue,
                                                             selfAddrValue,
                                                             selfAddrValue,
                                                             selfAddrValue,
                                                             selfAddrValue);

  piggyback_subscribe(IOTUS_PRIORITY_APPLICATION, receive_piggyback_cb);
  
  /* Start powertracing, once every two seconds. */
  powertrace_start(CLOCK_SECOND * POWER_TRACE_RATE);
  // set the etimer module to generate an event in one second.

#if EXP_ND_LINEAR_NODES == 1
  SENSORS_ACTIVATE(button_sensor);

  for(;;){
    PROCESS_WAIT_EVENT();
    if(ev == sensors_event && data == &button_sensor) {
      printf("Button pressed!\n");
      break;
    }
    if(ev == serial_line_event_message) {
      powertrace_print("ND");
    }
  }
#endif

  etimer_set(&timer, CLOCK_CONF_SECOND*MSG_INTERVAL);
  for(;;) {
    if(selfAddrValue != 1) {
      if(ev == serial_line_event_message) {
        // printf("got %s\n", (uint8_t *)data);
        // powertrace_stop();
        powertrace_print("ND");
        // break;
      } else if(tree_connection_status == TREE_STATUS_CONNECTED) {
        // printf("setting time\n");
        clock_time_t backoff = (CLOCK_SECOND*(2000+(random_rand()%BACKOFF_TIME)))/1000;//ms
        ctimer_set(&sendTimer, backoff, send_app_msg, NULL);
      }
    }

    PROCESS_WAIT_EVENT();
    etimer_restart(&timer);
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
