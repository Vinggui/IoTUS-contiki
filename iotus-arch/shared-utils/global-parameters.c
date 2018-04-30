
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
iotus_QoS_definitions_t iotus_QoS;

///////////////////////////////////////////////////////
//                   Radio related                   //
///////////////////////////////////////////////////////
iotus_packet_dimensions_t iotus_packet_dimensions = {0};

iotus_parameters_radio_events_t iotus_parameters_radio_events;

iotus_address_type iotus_radio_selected_address_type;

///////////////////////////////////////////////
//           Time parameters                 //
///////////////////////////////////////////////
// The time when the system want to use as reference
timestamp_t iotus_time_zero;



/***********************************************************************/


/* The following stuff ends the \defgroup block at the beginning of
   the file: */

/** @} */