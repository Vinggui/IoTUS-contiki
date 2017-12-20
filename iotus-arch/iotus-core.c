
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
#include <stdio.h>
#include <stdlib.h>
#include "contiki.h"
#include "iotus-core.h"
/* This next include is given by the makefile of the iotus core,
 * just as the next lists ahead. Hence, don't try to go to their definition.
*/
#include IOTUS_DYNAMIC_HEADER_FILE

#define DEBUG 1
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else /* DEBUG */
#define PRINTF(...)
#endif /* DEBUG */

#if IOTUS_CONF_USING_TRANSPORT == 1
static const struct iotus_transport_protocol_struct *available_transport_protocols_array[] =
IOTUS_PROTOCOL_TRANSPORT_LIST;
#endif
#if IOTUS_CONF_USING_ROUTING == 1
static const struct iotus_routing_protocol_struct *available_routing_protocols_array[] =
IOTUS_PROTOCOL_ROUTING_LIST;
#endif
#if IOTUS_CONF_USING_DATA_LINK == 1
static const struct iotus_data_link_protocol_struct *available_data_link_protocols_array[] =
IOTUS_PROTOCOL_DATA_LINK_LIST;
#endif

#if IOTUS_CONF_USING_TRANSPORT == 1
static const uint8_t iotus_transport_dependecies_table[][IOTUS_DEPENDENCIES_BUFFER_SIZE]=
IOTUS_LAYER_TRANSPORT_SERVICE_ARRAY;
#endif
#if IOTUS_CONF_USING_ROUTING == 1
static const uint8_t iotus_routing_dependecies_table[][IOTUS_DEPENDENCIES_BUFFER_SIZE]=
IOTUS_LAYER_ROUTING_SERVICE_ARRAY;
#endif
#if IOTUS_CONF_USING_DATA_LINK == 1
static const uint8_t iotus_data_link_dependecies_table[][IOTUS_DEPENDENCIES_BUFFER_SIZE]=
IOTUS_LAYER_DATA_LINK_SERVICE_ARRAY;
#endif

static const struct iotus_service_module_struct *iotus_service_modules_list[] =
IOTUS_SERVICE_MODULES_LIST;
static uint8_t iotus_services_used[IOTUS_DEPENDENCIES_BUFFER_SIZE]={0};

PROCESS(iotus_core_process, "Core IoTUS Process");


void start_new_comm_stack (iotus_transport_protocols transport, iotus_routing_protocols routing, iotus_data_link_protocols data_link)
{
  printf("Starting IoTUS core.\n");

  //Sum up the services to be Used
  int i=0;
  for(;i<IOTUS_DEPENDENCIES_BUFFER_SIZE;i++) {
    iotus_services_used[i]=0;
  }

  #if IOTUS_CONF_USING_TRANSPORT == 1
  i=0;
  //printf("TRANS\n");
  for(;i<IOTUS_DEPENDENCIES_BUFFER_SIZE;i++) {
    iotus_services_used[i] |= iotus_transport_dependecies_table[transport][i];
    //printf("TRANS=%x",iotus_transport_dependecies_table[transport][i]);
  }
  #endif
  #if IOTUS_CONF_USING_ROUTING == 1
  i=0;
  //printf("Rou\n");
  for(;i<IOTUS_DEPENDENCIES_BUFFER_SIZE;i++) {
    iotus_services_used[i] |= iotus_routing_dependecies_table[routing][i];
    //printf("v=%x",iotus_routing_dependecies_table[transport][i]);
  }
  #endif
  #if IOTUS_CONF_USING_DATA_LINK == 1
  i=0;
  //printf("DATA\n ");
  for(;i<IOTUS_DEPENDENCIES_BUFFER_SIZE;i++) {
    iotus_services_used[i] |= iotus_data_link_dependecies_table[data_link][i];
    //printf("Data=%x",iotus_data_link_dependecies_table[transport][i]);
  }
  #endif

  // Call the start of each service
  i=0;
  for(;i<IOTUS_TOTAL_SERVICES_USED;i++) {
    uint8_t x=i/8; //This should walk to the right
    uint8_t y=i%8;
    //printf("1 buffer=%x ind=%i, x=%i pos=%x\n",iotus_services_used[x],i,x,(1<<(7-y)));
    if(iotus_services_used[x] & (1<<(7-y))) {
      //printf("2\n");
      iotus_service_modules_list[i]->start();
    }
  }
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
