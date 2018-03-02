
/**
 * \defgroup decription...
 *
 * This...
 *
 * @{
 */

/*
 * global_parameters.c
 *
 *  Created on: Feb 27, 2018
 *      Author: vinicius
 */

#include <stdio.h>
#include <stdlib.h>
#include "iotus-core.h"
#include "global_parameters.h"
#include "platform-conf.h"

#define DEBUG 1
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else /* DEBUG */
#define PRINTF(...)
#endif /* DEBUG */

/***********************************************************************
                              QoS parameters
 ***********************************************************************/
packet_prioritization iotus_packet_prioritization;

/***********************************************************************
                              Radio Parameters
***********************************************************************/
uint16_t iotus_radio_max_message = IOTUS_RADIO_MAX_PACKET_SIZE;





/***********************************************************************/


/*
 * Default function required from IoTUS, to initialize, run and finish this service
 */
void
iotus_signal_handler_global_parameters(iotus_service_signal signal, void *data)
{
  /*
  if(IOTUS_START_SERVICE == signal) {

  } else if (IOTUS_RUN_SERVICE == signal){

  } else if (IOTUS_END_SERVICE == signal){

  }*/
}