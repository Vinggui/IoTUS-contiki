
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
#include "platform-conf.h"
#include "global-parameters.h"
#include "timestamp.h"



/////////////////////////////////////////////////////
//               QoS parameters                    //
/////////////////////////////////////////////////////
packet_prioritization iotus_packet_prioritization;

////////////////////////////////////////////
//             Radio parameters           //
////////////////////////////////////////////
uint16_t iotus_radio_max_message = IOTUS_RADIO_MAX_PACKET_SIZE;
uint8_t iotus_radio_address_size = IOTUS_RADIO_FULL_ADDRESS;


///////////////////////////////////////////////
//           Time parameters                 //
///////////////////////////////////////////////
// The time when the system want to use as reference
timestamp_t iotus_time_zero;



/***********************************************************************/


/* The following stuff ends the \defgroup block at the beginning of
   the file: */

/** @} */