
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
#include "iotus-netstack.h"
#include "iotus-core.h"
/* This next include is given by the makefile of the iotus core,
 * just as the next lists ahead. Hence, don't try to go to their definition.
*/
#include IOTUS_DYNAMIC_HEADER_FILE

#define DEBUG IOTUS_DONT_PRINT//IOTUS_PRINT_IMMEDIATELY
#define THIS_LOG_FILE_NAME_DESCRITOR "iotus-core"
#include "safe-printer.h"

//////////////////////////////////////////////////////////////////////////////////////////////
//                           Setup dynamic/static enums and arrays                          //
//////////////////////////////////////////////////////////////////////////////////////////////

struct iotus_transport_protocol_struct const *active_transport_protocol = NULL;
#ifdef IOTUS_COMPILE_MODE_DYNAMIC
  static const struct iotus_transport_protocol_struct *available_transport_protocols_array[] =
  IOTUS_PROTOCOL_TRANSPORT_LIST;
  static const uint8_t iotus_transport_dependecies_table[][IOTUS_DEPENDENCIES_BUFFER_SIZE]=
  IOTUS_LAYER_TRANSPORT_SERVICE_ARRAY;
#else
  active_transport_protocol = &IOTUS_STATIC_PROTOCOL_TRANSPORT;
#endif
#define ACTIVE_TRANSPORT_PROTOCOL(func) if(active_transport_protocol->func)active_transport_protocol->func()



struct iotus_routing_protocol_struct const *active_routing_protocol = NULL;
#ifdef IOTUS_COMPILE_MODE_DYNAMIC
  static const struct iotus_routing_protocol_struct *available_routing_protocols_array[] =
  IOTUS_PROTOCOL_ROUTING_LIST;
  static const uint8_t iotus_routing_dependecies_table[][IOTUS_DEPENDENCIES_BUFFER_SIZE]=
  IOTUS_LAYER_ROUTING_SERVICE_ARRAY;
#else
  active_routing_protocol = &IOTUS_STATIC_PROTOCOL_ROUTING;
#endif
#define ACTIVE_ROUTING_PROTOCOL(func) if(active_routing_protocol->func)active_routing_protocol->func()


struct iotus_data_link_protocol_struct const *active_data_link_protocol = NULL;
#ifdef IOTUS_COMPILE_MODE_DYNAMIC
  static const struct iotus_data_link_protocol_struct *available_data_link_protocols_array[] =
  IOTUS_PROTOCOL_DATA_LINK_LIST;
  static const uint8_t iotus_data_link_dependecies_table[][IOTUS_DEPENDENCIES_BUFFER_SIZE]=
  IOTUS_LAYER_DATA_LINK_SERVICE_ARRAY;
#else
  active_data_link_protocol = &IOTUS_STATIC_PROTOCOL_DATA_LINK;
#endif
#define ACTIVE_DATA_LINK_PROTOCOL(func) if(active_data_link_protocol->func)active_data_link_protocol->func()



struct iotus_radio_driver_struct const *active_radio_driver = NULL;
#ifdef IOTUS_COMPILE_MODE_DYNAMIC
  static const struct iotus_radio_driver_struct *available_radio_drivers_array[] =
  IOTUS_RADIO_DRIVERS_LIST;
#else
  active_radio_driver = &IOTUS_STATIC_RADIO_DRIVERS;
#endif
#define ACTIVE_RADIO_DRIVER(func) if(active_radio_driver->func)active_radio_driver->func()

/* Define the list of functions that are available by the services */
typedef void (*iotus_core_signal_process_function) (iotus_service_signal, void*);
static const iotus_core_signal_process_function iotus_core_signal_process_list[IOTUS_TOTAL_SERVICES_INSTALLED] =
                                                        IOTUS_SERVICE_MODULES_LIST;


#ifdef IOTUS_COMPILE_MODE_DYNAMIC
static uint8_t iotus_services_installed[IOTUS_DEPENDENCIES_BUFFER_SIZE]={0};
#endif

Boolean allowSystemSleep;
uint8_t sleepGreenFlag;
uint16_t netstackFreeTime = 0;

static application_demanding_task gApplicationDemandingTask;

// PROCESS(iotus_core_process, "Core IoTUS Process");


/*---------------------------------------------------------------------------*/
iotus_packet_t *
iotus_initiate_msg(uint16_t payloadSize, const uint8_t* payload, uint8_t params,
    iotus_layer_priority priority, uint16_t timeout, iotus_node_t *finalDestination)
{
  iotus_packet_t *packet;

  if(payloadSize == 0 || payload == NULL ||
    priority < IOTUS_PRIORITY_APPLICATION ||
    finalDestination == NULL) {
    SAFE_PRINTF_LOG_ERROR("Packet input");
    return NULL;
  }

  packet = packet_create_msg(
                payloadSize,
                payload,
                priority,
                timeout,
                TRUE,
                finalDestination);

  if(NULL == packet) {
    return NULL;
  }
  packet_set_parameter(packet,params);
  SAFE_PRINTF_LOG_INFO("Packet created");

  packet_send(packet);
  return packet;
}

/*---------------------------------------------------------------------------*/
static void
send_signal_to_services(iotus_service_signal signal, void *data)
{
  int i=0;
  for(;i<IOTUS_TOTAL_SERVICES_INSTALLED;i++) {
    #ifdef IOTUS_COMPILE_MODE_DYNAMIC
    int     x = i/8; //This should walk to the right
    uint8_t y = i%8;
    //PRINTF("Step 1 buffer=%x ind=%i, x=%i pos=%x\n",iotus_services_installed[x],i,x,(1<<(7-y)));
    if(iotus_services_installed[x] & (1<<(7-y))) {
      //PRINTF("step 2\n");
      iotus_core_signal_process_list[i](signal, data);
    }
    #else
    //In static mode, all services are used
    iotus_core_signal_process_list[i](signal, data);
    #endif
  }
}

/*---------------------------------------------------------------------------*/
void
iotus_core_start_system (
  #ifdef IOTUS_COMPILE_MODE_DYNAMIC
  iotus_transport_protocols transport,
  iotus_routing_protocols routing,
  iotus_data_link_protocols data_link,
  iotus_radio_drivers radio_driver
  #else
  void
  #endif
  )
{
  SAFE_PRINTF_LOG_INFO("Starting IoTUS core.\n");

  //Sum up the services to be Used
  int i;

  allowSystemSleep = FALSE;
  sleepGreenFlag = 0;
  gApplicationDemandingTask = NULL;


  #ifdef IOTUS_COMPILE_MODE_DYNAMIC
  for(i=0;i<IOTUS_DEPENDENCIES_BUFFER_SIZE;i++) {
    iotus_services_installed[i]=0;
  }
  #endif


  #ifdef IOTUS_COMPILE_MODE_DYNAMIC
    #if IOTUS_CONF_USING_TRANSPORT == 1
    //PRINTF("TRANS\n");
    for(i=0;i<IOTUS_DEPENDENCIES_BUFFER_SIZE;i++) {
      iotus_services_installed[i] |= iotus_transport_dependecies_table[transport][i];
      //PRINTF("TRANS=%x",iotus_transport_dependecies_table[transport][i]);
    }
    #endif /*IOTUS_CONF_USING_TRANSPORT == 1*/

    #if IOTUS_CONF_USING_ROUTING == 1
    //PRINTF("Rou\n");
    for(i=0;i<IOTUS_DEPENDENCIES_BUFFER_SIZE;i++) {
      iotus_services_installed[i] |= iotus_routing_dependecies_table[routing][i];
      //PRINTF("v=%x",iotus_routing_dependecies_table[transport][i]);
    }
    #endif/*IOTUS_CONF_USING_ROUTING == 1*/

    #if IOTUS_CONF_USING_DATA_LINK == 1
    //PRINTF("DATA\n ");
    for(i=0;i<IOTUS_DEPENDENCIES_BUFFER_SIZE;i++) {
      iotus_services_installed[i] |= iotus_data_link_dependecies_table[data_link][i];
      //PRINTF("Data=%x",iotus_data_link_dependecies_table[transport][i]);
    }
    #endif /*IOTUS_CONF_USING_DATA_LINK == 1*/
  #endif /* ifdef IOTUS_COMPILE_MODE_DYNAMIC */

  ////////////////////////////////////////////
  //             Start Radio                //
  ////////////////////////////////////////////
  #ifdef IOTUS_COMPILE_MODE_DYNAMIC
  active_radio_driver = available_radio_drivers_array[radio_driver];
  #endif /* ifdef IOTUS_COMPILE_MODE_DYNAMIC */
  ACTIVE_RADIO_DRIVER(start);

  /////////////////////////////////////////////
  //      Call the start of each service     //
  /////////////////////////////////////////////
  send_signal_to_services(IOTUS_START_SERVICE, NULL);

  /////////////////////////////////////////////
  //     Call the start of each protocols    //
  /////////////////////////////////////////////
  //#if IOTUS_CONF_USING_TRANSPORT == 1
    #ifdef IOTUS_COMPILE_MODE_DYNAMIC
    active_transport_protocol = available_transport_protocols_array[transport];
    #endif /* ifdef IOTUS_COMPILE_MODE_DYNAMIC */
    ACTIVE_TRANSPORT_PROTOCOL(start);
  //#endif /* IOTUS_CONF_USING_TRANSPORT == 1 */

  //#if IOTUS_CONF_USING_ROUTING == 1
    #ifdef IOTUS_COMPILE_MODE_DYNAMIC
    active_routing_protocol = available_routing_protocols_array[routing];
    #endif /* ifdef IOTUS_COMPILE_MODE_DYNAMIC */
    ACTIVE_ROUTING_PROTOCOL(start);
  //#endif /* IOTUS_CONF_USING_ROUTING == 1 */

  //#if IOTUS_CONF_USING_DATA_LINK == 1
    #ifdef IOTUS_COMPILE_MODE_DYNAMIC
    active_data_link_protocol = available_data_link_protocols_array[data_link];
    #endif /* ifdef IOTUS_COMPILE_MODE_DYNAMIC */
    ACTIVE_DATA_LINK_PROTOCOL(start);
  //#endif /* IOTUS_CONF_USING_DATA_LINK == 1 */

  ///////////////////////////////////////////
  //     execute post_start functions      //
  ///////////////////////////////////////////
  ACTIVE_RADIO_DRIVER(post_start);

  /* Call the post start functions */
  #if IOTUS_CONF_USING_TRANSPORT == 1
  ACTIVE_TRANSPORT_PROTOCOL(post_start);
  #endif
  #if IOTUS_CONF_USING_ROUTING == 1
  ACTIVE_ROUTING_PROTOCOL(post_start);
  #endif
  #if IOTUS_CONF_USING_DATA_LINK == 1
  ACTIVE_DATA_LINK_PROTOCOL(post_start);
  #endif

  // process_start(&iotus_core_process, NULL);
}

/*---------------------------------------------------------------------------*/
/**
 * Function for the data link layer, informing how long this system can sleep
 * \param max_duration The maximum duration in ms.
 */
void
iotus_core_netstack_idle_for(iotus_layer_priority layer, uint16_t maxDuration)
{
  if(layer < 8) {
    sleepGreenFlag |= (1<<layer);
    maxDuration=maxDuration;
  }
}

/*---------------------------------------------------------------------------*/
/**
 * Function for the application layer, informing if the system can go to sleep
 * \param time_needed The necessary time needed by this demanding task.
 * \param task        The function to be called whenever possible.
 */
void
iotus_set_demanding_task(uint16_t time_needed, application_demanding_task task)
{
  if(time_needed == 0xFFFF) {
    SAFE_PRINTF_LOG_INFO("No time definition");
  }
  iotus_QoS.applicationDuration = time_needed;
  gApplicationDemandingTask = task;
}

/*---------------------------------------------------------------------------*/
/* Implementation of the IoTus core process */
// PROCESS_THREAD(iotus_core_process, ev, data)
// {
  /* variables are declared static to ensure their values are kept
   * between kernel calls.
   */
  //static struct stimer refreshingIICTimer;

  /* Any process must start with this. */
  // PROCESS_BEGIN();

  /* Initiate the lists of module */

  //PROCESS_PAUSE();

  //Main loop here
  // for(;;) {

  //   //Time to run protocols
  //   ACTIVE_RADIO_DRIVER(run);
  //   #if IOTUS_CONF_USING_DATA_LINK == 1
  //     ACTIVE_DATA_LINK_PROTOCOL(run);
  //   #else
  //     sleepGreenFlag = (1<<IOTUS_PRIORITY_DATA_LINK);
  //   #endif/*IOTUS_CONF_USING_DATA_LINK == 1*/
  //   #if IOTUS_CONF_USING_ROUTING == 1
  //     ACTIVE_ROUTING_PROTOCOL(run);
  //   #else
  //     sleepGreenFlag = (1<<IOTUS_PRIORITY_ROUTING);
  //   #endif/*IOTUS_CONF_USING_ROUTING == 1*/
  //   #if IOTUS_CONF_USING_TRANSPORT == 1
  //     ACTIVE_TRANSPORT_PROTOCOL(run);
  //   #else
  //     sleepGreenFlag = (1<<IOTUS_PRIORITY_TRANSPORT);
  //   #endifIOTUS_CONF_USING_TRANSPORT == 1
  //   // Run each services
  //   send_signal_to_services(IOTUS_RUN_SERVICE, NULL);
  //   if(netstackFreeTime > iotus_QoS.applicationDuration) {
  //     //Execute the demanding application task
      
  //   }
  //   if(allowSystemSleep &&
  //     sleepGreenFlag == 0b00011110) {//radio=0, data_link=1, routing=2, transport=3, app=4
  //     /* 
  //      * Time management is now necessary.
  //      * The stack will take as long as it needs to operate
  //      * and then give the rest of the time to the application layer.
  //      */
  //     //sleep();
  //     sleepGreenFlag = 0;
  //   }
    //Reset the sleep flag

    // PROCESS_WAIT_EVENT();
  // }
  // any process must end with this, even if it is never reached.
  // PROCESS_END();
// }


/* The following stuff ends the \defgroup block at the beginning of
   the file: */

/** @} */
