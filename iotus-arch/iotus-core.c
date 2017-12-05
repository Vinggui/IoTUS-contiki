
/**
 * \defgroup decription...
 *
 * This...
 *
 * @{
 */

/*
 * iotus-core.c
 *
 *  Created on: Oct 23, 2017
 *      Author: vinicius
 */

#include "contiki.h"

static const struct iotus_module_struct available_transport_protocols_array = IOTUS_PROTOCOL_TRANSPORT_LIST;
static const struct iotus_module_struct available_routing_protocols_array = IOTUS_PROTOCOL_ROUTING_LIST;
static const struct iotus_module_struct available_data_link_protocols_array = IOTUS_PROTOCOL_DATA_LINK_LIST;


PROCESS(iotus_core_process, "Core IoTUS Process");


void start_new_comm_stack (void)
{
  //TODO Create the start of the iotus core system
}



/* Implementation of the IoTus core process */
PROCESS_THREAD(iotus_core_process, ev, data)
{
  /* variables are declared static to ensure their values are kept
   * between kernel calls.
   */
  //static struct stimer refreshingIICTimer;

  /* Any process must start with this. */
  PROCESS_BEGIN();

  /* Initiate the lists of module */

  //PROCESS_PAUSE();

  //Main loop here
  for(;;) {
    PROCESS_PAUSE();
  }
  // any process must end with this, even if it is never reached.
  PROCESS_END();
}



/* The following stuff ends the \defgroup block at the beginning of
   the file: */

/** @} */
