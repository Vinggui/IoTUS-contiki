
/**
 * \defgroup decription...
 *
 * This...
 *
 * @{
 */

/*
 * piggyback.c
 *
 *  Created on: Feb 27, 2018
 *      Author: vinicius
 */

#include <stdio.h>
#include <stdlib.h>
#include "iotus-core.h"
#include "piggyback.h"
#include "platform-conf.h"
#include "global_parameters.h"

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
uint8_t iotus_radio_address_size = IOTUS_RADIO_FULL_ADDRESS;





/***********************************************************************/


/*
 * Default function required from IoTUS, to initialize, run and finish this service
 */
void
iotus_signal_handler_piggyback(iotus_service_signal signal, void *data)
{
  /*
  if(IOTUS_START_SERVICE == signal) {

  } else if (IOTUS_RUN_SERVICE == signal){

  } else if (IOTUS_END_SERVICE == signal){

  }*/
}