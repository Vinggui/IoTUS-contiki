
/**
 * \defgroup decription...
 *
 * This...
 *
 * @{
 */

/*
 * edytee-transport.c
 *
 *  Created on: Nov 18, 2017
 *      Author: vinicius
 */


PROCESS(edytee_transport, "EDyTEE Transport Protocol");

/* Implementation of the IoTus core process */
PROCESS_THREAD(edytee_transport, ev, data)
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

struct iotus_transport_protocol_struct edytee_transport_transport_protocol = {
  start,
  run,
  close
};

/* The following stuff ends the \defgroup block at the beginning of
   the file: */

/** @} */
