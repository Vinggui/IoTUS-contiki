
/**
 * \defgroup decription...
 *
 * This...
 *
 * @{
 */

/*
 * edytee-routing.c
 *
 *  Created on: Nov 18, 2017
 *      Author: vinicius
 */

#include "contiki.h"

PROCESS(edytee_routing_process, "EDyTEE Routing Protocol");

/* Implementation of the IoTus core process */
PROCESS_THREAD(edytee_routing_process, ev, data)
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


/* The following stuff ends the \defgroup block at the beginning of
   the file: */

/** @} */

