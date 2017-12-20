
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
static const uint8_t iotus_transport_dependecies_table[][IOTUS_DEPENDENCIES_BUFFER_SIZE]=
IOTUS_LAYER_TRANSPORT_SERVICE_ARRAY;
static struct iotus_transport_protocol_struct const *active_transport_protocol = NULL;
#endif/*IOTUS_CONF_USING_TRANSPORT == 1*/

#if IOTUS_CONF_USING_ROUTING == 1
static const struct iotus_routing_protocol_struct *available_routing_protocols_array[] =
IOTUS_PROTOCOL_ROUTING_LIST;
static const uint8_t iotus_routing_dependecies_table[][IOTUS_DEPENDENCIES_BUFFER_SIZE]=
IOTUS_LAYER_ROUTING_SERVICE_ARRAY;
static struct iotus_routing_protocol_struct const *active_routing_protocol = NULL;
#endif/*IOTUS_CONF_USING_ROUTING == 1*/

#if IOTUS_CONF_USING_DATA_LINK == 1
static const struct iotus_data_link_protocol_struct *available_data_link_protocols_array[] =
IOTUS_PROTOCOL_DATA_LINK_LIST;
static const uint8_t iotus_data_link_dependecies_table[][IOTUS_DEPENDENCIES_BUFFER_SIZE]=
IOTUS_LAYER_DATA_LINK_SERVICE_ARRAY;
static struct iotus_data_link_protocol_struct const *active_data_link_protocol = NULL;
#endif/*IOTUS_CONF_USING_DATA_LINK == 1*/

/* Define the list of functions that are available by the services */
typedef void (*iotus_core_signal_process_function) (iotus_service_signal, void*);
static const iotus_core_signal_process_function iotus_core_signal_process_list[IOTUS_TOTAL_SERVICES_INSTALLED] =
IOTUS_SERVICE_MODULES_LIST;
static uint8_t iotus_services_installed[IOTUS_DEPENDENCIES_BUFFER_SIZE]={0};

PROCESS(iotus_core_process, "Core IoTUS Process");

static void
send_signal_active_services(iotus_service_signal signal, void *data)
{
  int i=0;
  for(;i<IOTUS_TOTAL_SERVICES_INSTALLED;i++) {
    int     x = i/8; //This should walk to the right
    uint8_t y = i%8;
    //printf("Step 1 buffer=%x ind=%i, x=%i pos=%x\n",iotus_services_installed[x],i,x,(1<<(7-y)));
    if(iotus_services_installed[x] & (1<<(7-y))) {
      //printf("step 2\n");
      iotus_core_signal_process_list[i](signal, data);
    }
  }
}


void
iotus_core_start_system (
  iotus_transport_protocols transport,
  iotus_routing_protocols routing,
  iotus_data_link_protocols data_link)
{
  printf("Starting IoTUS core.\n");

  //Sum up the services to be Used
  int i=0;
  for(;i<IOTUS_DEPENDENCIES_BUFFER_SIZE;i++) {
    iotus_services_installed[i]=0;
  }

  #if IOTUS_CONF_USING_TRANSPORT == 1
  i=0;
  //printf("TRANS\n");
  for(;i<IOTUS_DEPENDENCIES_BUFFER_SIZE;i++) {
    iotus_services_installed[i] |= iotus_transport_dependecies_table[transport][i];
    //printf("TRANS=%x",iotus_transport_dependecies_table[transport][i]);
  }
  #endif /*IOTUS_CONF_USING_TRANSPORT == 1*/

  #if IOTUS_CONF_USING_ROUTING == 1
  i=0;
  //printf("Rou\n");
  for(;i<IOTUS_DEPENDENCIES_BUFFER_SIZE;i++) {
    iotus_services_installed[i] |= iotus_routing_dependecies_table[routing][i];
    //printf("v=%x",iotus_routing_dependecies_table[transport][i]);
  }
  #endif/*IOTUS_CONF_USING_ROUTING == 1*/

  #if IOTUS_CONF_USING_DATA_LINK == 1
  i=0;
  //printf("DATA\n ");
  for(;i<IOTUS_DEPENDENCIES_BUFFER_SIZE;i++) {
    iotus_services_installed[i] |= iotus_data_link_dependecies_table[data_link][i];
    //printf("Data=%x",iotus_data_link_dependecies_table[transport][i]);
  }
  #endif /*IOTUS_CONF_USING_DATA_LINK == 1*/

  // Call the start of each service
  send_signal_active_services(IOTUS_START_SERVICE, NULL);

  //Call the start of each protocols
  #if IOTUS_CONF_USING_TRANSPORT == 1
  active_transport_protocol = available_transport_protocols_array[transport];
  active_transport_protocol->start();
  #endif
  #if IOTUS_CONF_USING_ROUTING == 1
  active_routing_protocol = available_routing_protocols_array[routing];
  active_routing_protocol->start();
  #endif
  #if IOTUS_CONF_USING_DATA_LINK == 1
  active_data_link_protocol = available_data_link_protocols_array[data_link];
  active_data_link_protocol->start();
  #endif

  process_start(&iotus_core_process, NULL);
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
    //Time to run protocols
    #if IOTUS_CONF_USING_TRANSPORT == 1
    active_transport_protocol->run();
    #endif/*IOTUS_CONF_USING_TRANSPORT == 1*/
    #if IOTUS_CONF_USING_ROUTING == 1
    active_routing_protocol->run();
    #endif/*IOTUS_CONF_USING_ROUTING == 1*/
    #if IOTUS_CONF_USING_DATA_LINK == 1
    active_data_link_protocol->run();
    #endif/*IOTUS_CONF_USING_DATA_LINK == 1*/
    PROCESS_PAUSE();
  }
  // any process must end with this, even if it is never reached.
  PROCESS_END();
}



/* The following stuff ends the \defgroup block at the beginning of
   the file: */

/** @} */
