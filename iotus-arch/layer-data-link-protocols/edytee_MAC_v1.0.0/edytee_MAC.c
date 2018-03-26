



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

PROCESS(edytee_MAC_proc, "EDyTEE MAC Protocol");

/* Implementation of the IoTus core process */
PROCESS_THREAD(edytee_MAC_proc, ev, data)
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

    PRINTF("Running EDyTEE MAC");


    etimer_reset(&timer);
  }

  PROCESS_END();
}


static void
start(void)
{
  printf("\tEdytee MAC\n");
  process_start(&edytee_MAC_proc, NULL);
}


static void
run(void)
{
}

static void
close(void)
{}

const struct iotus_data_link_protocol_struct edytee_MAC_protocol = {
  start,
  run,
  close
};

/* The following stuff ends the \defgroup block at the beginning of
   the file: */

/** @} */
