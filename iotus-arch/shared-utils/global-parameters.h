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
  uint16_t collisions;
  uint16_t transmission;
  uint16_t receptions;
  uint16_t lastTransmitTime;//in ms
  int16_t lastRSSI;
  int16_t lastLinkQuality;
  uint8_t badSynch; //When packet len is bigger then allowed
  uint8_t badRxPacketLong; //When packet len is bigger then allowed
  uint8_t badRxPacketShort; //When packet len is bigger then allowed
  uint8_t badRxChecksumFail; //When packet len is bigger then allowed
} iotus_parameters_radio_events_t;

extern iotus_parameters_radio_events_t iotus_parameters_radio_events;
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
