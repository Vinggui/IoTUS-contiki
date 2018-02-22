



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

      packet_create_msg_piece(6, TRUE,
        FALSE, IOTUS_PRIORITY_DATA_LINK, 5000, "Teste",
        "01", NULL);


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
