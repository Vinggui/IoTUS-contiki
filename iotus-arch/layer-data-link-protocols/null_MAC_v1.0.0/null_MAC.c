
/**
 * \defgroup decription...
 *
 * This...
 *
 * @{
 */

/*
 * null-MAC.c
 *
 *  Created on: Nov 18, 2017
 *      Author: vinicius
 */
#include <stdio.h>
#include "contiki.h"
#include "iotus-data-link.h"

#define DEBUG 1
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else /* DEBUG */
#define PRINTF(...)
#endif /* DEBUG */

PROCESS(null_MAC_proc, "Null MAC Protocol");

/* Implementation of the IoTus core process */
PROCESS_THREAD(null_MAC_proc, ev, data)
{
  /* variables are declared static to ensure their values are kept
   * between kernel calls.
   */

  /* Any process must start with this. */
  PROCESS_BEGIN();

  /* Initiate the lists of module */

  //Main loop here
  //for(;;) {
  //  PROCESS_PAUSE();
  //}
  // any process must end with this, even if it is never reached.
  PROCESS_END();
}


static void
start(void)
{
  printf("Starting null MAC\n");
}


static void
run(void)
{
}

static void
close(void)
{}

const struct iotus_data_link_protocol_struct null_MAC_protocol = {
  "nullMAC",
  start,
  NULL,
  run,
  close
};

/* The following stuff ends the \defgroup block at the beginning of
   the file: */

/** @} */
