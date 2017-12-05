
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


PROCESS(null_MAC, "Null MAC Protocol");

/* Implementation of the IoTus core process */
PROCESS_THREAD(null_MAC, ev, data)
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
}


static void
run(void)
{
}

static void
close(void)
{}

struct iotus_data_link_protocol_struct null_MAC_data_link_protocol = {
  start,
  run,
  close
};

/* The following stuff ends the \defgroup block at the beginning of
   the file: */

/** @} */
