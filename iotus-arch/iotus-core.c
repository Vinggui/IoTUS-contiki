
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
#include "iotus-core.h"
/* This next include is given by the makefile of the iotus core,
 * just as the next lists ahead. Hence, don't try to go to their definition.
*/
#include IOTUS_DYNAMIC_HEADER_FILE

static const struct iotus_transport_protocol_struct *available_transport_protocols_array[] =
IOTUS_PROTOCOL_TRANSPORT_LIST;
static const struct iotus_routing_protocol_struct *available_routing_protocols_array[] =
IOTUS_PROTOCOL_ROUTING_LIST;
static const struct iotus_data_link_protocol_struct *available_data_link_protocols_array[] =
IOTUS_PROTOCOL_DATA_LINK_LIST;

static const uint8_t iotus_transport_dependecies_table[][IOTUS_DEPENDENCIES_BUFFER_SIZE]=
IOTUS_LAYER_TRANSPORT_SERVICE_ARRAY;
static const uint8_t iotus_routing_dependecies_table[][IOTUS_DEPENDENCIES_BUFFER_SIZE]=
IOTUS_LAYER_ROUTING_SERVICE_ARRAY;
static const uint8_t iotus_data_link_dependecies_table[][IOTUS_DEPENDENCIES_BUFFER_SIZE]=
IOTUS_LAYER_DATA_LINK_SERVICE_ARRAY;


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
