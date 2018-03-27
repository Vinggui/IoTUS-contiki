/**
 * \defgroup decription...
 *
 * This...
 *
 * @{
 */

/*
 * edytee-MAC.c
 *
 *  Created on: Nov 18, 2017
 *      Author: vinicius
 */
#include <stdio.h>
#include "contiki.h"
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

    if(IOTUS_PRIORITY_DATA_LINK == packet_get_layer_assigned_for(IOTUS_DEFAULT_HEADER_CHORE_CHECKSUM)) {
      SAFE_PRINTF_CLEAN("Deu - CS\n\n\n");
      piggyback_create_piece(
        8,(const uint8_t *)"merda!!!",0,
        NODES_BROADCAST, 3000);
    }
    if(IOTUS_PRIORITY_DATA_LINK == packet_get_layer_assigned_for(IOTUS_DEFAULT_HEADER_CHORE_ONEHOP_BROADCAST)) {
      SAFE_PRINTF_CLEAN("Deu - BC\n");

      iotus_packet_t *packet = packet_create_msg(6, IOTUS_PRIORITY_DATA_LINK, 5000,
        (const uint8_t *)"Teste",
        NODES_BROADCAST, NULL);

      uint8_t testeHeader[1] = {0b00000111};
      uint16_t teste = packet_push_bit_header(3, testeHeader, packet);

      if(teste > 0) {
        SAFE_PRINTF_CLEAN("Bits pushed ok! new size %u\n",teste);
      }

      uint8_t testeHeaderFullBytes[3] = {0xbe, 0xef, 0xFF};
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


    }


    etimer_reset(&timer);
  }



  PROCESS_END();
}


static void
start(void)
{
  SAFE_PRINTF_CLEAN("\tContikiMAC\n");
  process_start(&contikiMAC_proc, NULL);

  packet_subscribe_for_chore(IOTUS_PRIORITY_DATA_LINK, IOTUS_DEFAULT_HEADER_CHORE_CHECKSUM);
  packet_subscribe_for_chore(IOTUS_PRIORITY_DATA_LINK, IOTUS_DEFAULT_HEADER_CHORE_ONEHOP_BROADCAST);
  timestamp_mark(&firstTimer,0);
}


static void
run(void)
{
}

static void
close(void)
{
}

const struct iotus_data_link_protocol_struct contikiMAC_protocol = {
  start,
  run,
  close
};

/* The following stuff ends the \defgroup block at the beginning of
   the file: */

/** @} */
