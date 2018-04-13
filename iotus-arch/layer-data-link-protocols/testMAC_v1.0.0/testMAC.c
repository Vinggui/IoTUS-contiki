/**
 * \defgroup description...
 *
 * This...
 *
 * @{
 */

/*
 * testMAC.c
 *
 *  Created on: Nov 18, 2017
 *      Author: vinicius
 */
#include <stdio.h>
#include "contiki.h"
#include "iotus-data-link.h"
#include "iotus-radio.h"
#include "packet.h"
#include "packet-defs.h"
#include "piggyback.h"
#include "nodes.h"
#include "timestamp.h"


#define DEBUG IOTUS_PRINT_IMMEDIATELY
#define THIS_LOG_FILE_NAME_DESCRITOR "contikiMAC"
#include "safe-printer.h"

timestamp_t firstTimer;

PROCESS(contikiMAC_proc, "ContikiMAC Protocol");

/* Implementation of the IoTus core process */
PROCESS_THREAD(contikiMAC_proc, ev, data)
{
  /* variables are declared static to ensure their values are kept
   * between kernel calls.
   */
  static struct etimer timer;

  /* Any process must start with this. */
  PROCESS_BEGIN();
  etimer_set(&timer, CLOCK_SECOND*2);

  /* Initiate the lists of module */

  //Main loop here
  for(;;) {
    PROCESS_WAIT_EVENT();

    if(IOTUS_PRIORITY_DATA_LINK == iotus_get_layer_assigned_for(IOTUS_CHORE_SET_ADDR_FOR_RADIO)) {
      piggyback_create_piece(
        8,(const uint8_t *)"Tides!!!",0,
        NODES_BROADCAST, 3000);
    }
    if(IOTUS_PRIORITY_DATA_LINK == iotus_get_layer_assigned_for(IOTUS_CHORE_ONEHOP_BROADCAST)) {
      iotus_packet_t *packet = packet_create_msg(10, IOTUS_PRIORITY_DATA_LINK, 5000,
        (const uint8_t *)"BUeste msg", TRUE,
        NODES_BROADCAST, NULL);

      if(NULL == packet) {
        continue;
      }

      uint8_t testeHeader[1] = {0b00000111};
      uint16_t teste = packet_push_bit_header(3, testeHeader, packet);

      if(teste > 0) {
        SAFE_PRINTF_CLEAN("Bits pushed ok! new size %u\n",teste);
      }

      uint8_t testeHeaderFullBytes[3] = {'G', 'o', 'D'};
      teste = packet_append_last_header(3, testeHeaderFullBytes, packet);

      if(teste > 0) {
        SAFE_PRINTF_CLEAN("Bytes appended ok! new size %u\n",teste);
      }

      //testing reading
      teste = packet_read_byte(7, packet);
      SAFE_PRINTF_CLEAN("Packet byte 7 is: %02x\n",teste);

      //testing piggyback
      packet_set_parameter(packet, PACKET_PARAMETERS_ALLOW_PIGGYBACK);
      teste = piggyback_apply(packet);
      SAFE_PRINTF_CLEAN("Packet after piggyback: %u\n",teste);


      //Testing timestamp
      unsigned long elapsed = timestamp_elapsed(&firstTimer);
      SAFE_PRINTF_CLEAN("Elapsed time is: %lu\n",elapsed);

      packet->nextDestinationNode = NODES_BROADCAST;

      active_radio_driver->send(packet);
    }


    etimer_reset(&timer);
  }



  PROCESS_END();
}

static void
receive(iotus_packet_t *packet)
{
  SAFE_PRINTF_CLEAN("Rcv pkt len: %u\n",packet->data.size);
  if(packet->iotusHeader & PACKET_IOTUS_HDR_IS_BROADCAST) {
    SAFE_PRINTF_CLEAN("Rcv pkt broadcast\n");
  }
  SAFE_PRINTF_CLEAN("msg data %x",pieces_get_data_pointer(packet)[0]);
  
}

static void
start(void)
{
  SAFE_PRINTF_CLEAN("\tContikiMAC\n");
  process_start(&contikiMAC_proc, NULL);

  iotus_subscribe_for_chore(IOTUS_PRIORITY_DATA_LINK,
    IOTUS_CHORE_SET_ADDR_PANID);
  iotus_subscribe_for_chore(IOTUS_PRIORITY_DATA_LINK,
   IOTUS_CHORE_SET_ADDR_SHORT_LONG);
  iotus_subscribe_for_chore(IOTUS_PRIORITY_DATA_LINK,
   IOTUS_CHORE_SET_ADDR_FOR_RADIO);
  iotus_subscribe_for_chore(IOTUS_PRIORITY_DATA_LINK,
   IOTUS_CHORE_ONEHOP_BROADCAST);

  iotus_subscribe_for_chore(IOTUS_PRIORITY_RADIO,
   IOTUS_CHORE_INSERT_PKT_PREV_SRC_ADDRESS);
  iotus_subscribe_for_chore(IOTUS_PRIORITY_RADIO,
   IOTUS_CHORE_INSERT_PKT_NEXT_DST_ADDRESS);

  timestamp_mark(&firstTimer,0);
}

static void
post_start(void)
{
  radio_value_t value;
  if(IOTUS_PRIORITY_DATA_LINK == 
    iotus_get_layer_assigned_for(IOTUS_CHORE_SET_ADDR_PANID)) {
    //Don`t do anything. it`s already hardcoded.
  }
  if(IOTUS_PRIORITY_DATA_LINK == 
    iotus_get_layer_assigned_for(IOTUS_CHORE_SET_ADDR_SHORT_LONG)) {
    //Don`t do anything. it`s already hardcoded.
  }
  if(IOTUS_PRIORITY_DATA_LINK == 
    iotus_get_layer_assigned_for(IOTUS_CHORE_SET_ADDR_FOR_RADIO)) {
    //Set radio address...
    active_radio_driver->get_value(RADIO_CONST_ADDRESSES_OPTIONS, &value);
    if(RADIO_ADDR_OPTIONS_MATCH(IOTUS_ADDRESSES_TYPE_ADDR_SHORT,value)) {
      active_radio_driver->set_value(RADIO_PARAM_ADDRESS_USE_TYPE,
                              IOTUS_ADDRESSES_TYPE_ADDR_SHORT);
    }
  }


  uint8_t broadcastAddr[4] = {0xff,0xff,0xff,0xff};
  addresses_set_value(IOTUS_ADDRESSES_TYPE_ADDR_BROADCAST,broadcastAddr);

  //Drop packets not destined to us
  active_radio_driver->set_value(RADIO_PARAM_RX_MODE,
    RADIO_RX_MODE_ADDRESS_SOFTWARE_FILTER);
  active_radio_driver->on();
}

static void
run(void)
{
}

static void
close(void)
{
}

const struct iotus_data_link_protocol_struct testMAC_protocol = {
  "testMAC",
  start,
  post_start,
  run,
  close,
  NULL,
  NULL,
  receive
};

/* The following stuff ends the \defgroup block at the beginning of
   the file: */

/** @} */
