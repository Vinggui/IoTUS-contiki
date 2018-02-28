



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

#define DEBUG 1
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else /* DEBUG */
#define PRINTF(...)
#endif /* DEBUG */

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

    if(packet_verify_default_header_chore(
      IOTUS_PRIORITY_DATA_LINK, IOTUS_DEFAULT_HEADER_CHORE_CHECKSUM)) {
      PRINTF("Deu - CS\n");
    }
    if(packet_verify_default_header_chore(
      IOTUS_PRIORITY_DATA_LINK, IOTUS_DEFAULT_HEADER_CHORE_ONEHOP_BROADCAST)) {
      PRINTF("Deu - BC\n");

      void *packet = packet_create_msg_piece(6, TRUE,
        FALSE, IOTUS_PRIORITY_DATA_LINK, 5000, (const uint8_t *)"Teste",
        (const uint8_t *)"01", NULL);

      uint8_t testeHeader[1] = {0b00000111};
      uint16_t teste = iotus_packet_push_bit_header(3, testeHeader, packet);

      if(teste > 0) {
        PRINTF("Bits pushed ok! new size %u\n",teste);
      }

      uint8_t testeHeaderFullBytes[3] = {0xbe, 0xef, 0xFF};
      teste = iotus_packet_append_byte_header(3, testeHeaderFullBytes, packet);

      if(teste > 0) {
        PRINTF("Bytes appended ok! new size %u\n",teste);
      }

      //testing reading
      teste = iotus_packet_read_byte(7, packet);
      PRINTF("Packet byte 7 is: %02x",teste);
    }


    etimer_reset(&timer);
  }



  PROCESS_END();
}


static void
start(void)
{
  printf("\tContikiMAC\n");
  process_start(&contikiMAC_proc, NULL);

  packet_subscribe_default_header_chore(IOTUS_PRIORITY_DATA_LINK, IOTUS_DEFAULT_HEADER_CHORE_ONEHOP_BROADCAST);
}


static void
run(void)
{
}

static void
close(void)
{}

const struct iotus_data_link_protocol_struct contikiMAC_protocol = {
  start,
  run,
  close
};

/* The following stuff ends the \defgroup block at the beginning of
   the file: */

/** @} */
