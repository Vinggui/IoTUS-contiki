/**
 * \defgroup decription...
 *
 * This...
 *
 * @{
 */

/*
 * packet.h
 *
 *  Created on: Nov 15, 2017
 *      Author: vinicius
 */

#ifndef IOTUS_ARCH_SERVICES_GLOBAL_PARAMETERS_GLOBAL_PARAMETERS_H_
#define IOTUS_ARCH_SERVICES_GLOBAL_PARAMETERS_GLOBAL_PARAMETERS_H_

#include "iotus-core.h"
#include "platform-conf.h"
#include "timestamp.h"


////////////////////////////////////////////////////
//           System general parameters            //
////////////////////////////////////////////////////

typedef enum {
  IOTUS_PACKET_PRIORITIZE_ENERGY_EFFICIENCY = 0,
  IOTUS_PACKET_PRIORITIZE_LOW_LATENCY
} packet_prioritization;


////////////////////////////////////////////
//             Radio parameters           //
////////////////////////////////////////////

/* This parameter should be written only by the radio layer */
typedef struct iotus_radio_events {
  uint8_t collisions;
  uint8_t transmitTime;
} iotus_parameters_radio_events_t;

/* Most of these parameter should be set by data link layer */
typedef struct iotus_radio_parameters {
  uint8_t channel;
  uint8_t txPower;
  uint8_t listenTime;
  uint8_t maxReTxTimes; 
} iotus_parameters_radio_setup;

extern uint8_t iotus_radio_address_size;
extern uint16_t iotus_radio_max_message;


/////////////////////////////////////////////////////
//               QoS parameters                    //
/////////////////////////////////////////////////////

extern packet_prioritization iotus_packet_prioritization;


///////////////////////////////////////////////
//           Time parameters                 //
///////////////////////////////////////////////

extern timestamp_t iotus_time_zero;

#endif /* IOTUS_ARCH_SERVICES_GLOBAL_PARAMETERS_GLOBAL_PARAMETERS_H_ */

/* The following stuff ends the \defgroup block at the beginning of
   the file: */

/** @} */