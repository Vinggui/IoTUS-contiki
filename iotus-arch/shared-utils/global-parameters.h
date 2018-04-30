/**
 * \defgroup description...
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
#include "iotus-radio.h"


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
  uint8_t badRxNoMemery;
  radio_status radioStatus;
} iotus_parameters_radio_events_t;

typedef struct iotus_packet_size_limits {
  uint16_t totalSize;
  uint8_t radioHeaders;
  uint8_t datalinkHeaders;
  uint8_t routingHeaders;
  uint8_t transportHeaders;
} iotus_packet_dimensions_t;


extern iotus_parameters_radio_events_t iotus_parameters_radio_events;
extern iotus_packet_dimensions_t iotus_packet_dimensions;

extern iotus_address_type iotus_radio_selected_address_type;


/////////////////////////////////////////////////////
//               QoS parameters                    //
/////////////////////////////////////////////////////
typedef struct iotus_QoS_definitions {
  uint8_t applicationDuration;
  packet_prioritization packetPrioritization;
} iotus_QoS_definitions_t;

extern iotus_QoS_definitions_t iotus_QoS;


///////////////////////////////////////////////
//           Time parameters                 //
///////////////////////////////////////////////

extern timestamp_t iotus_time_zero;

#endif /* IOTUS_ARCH_SERVICES_GLOBAL_PARAMETERS_GLOBAL_PARAMETERS_H_ */

/* The following stuff ends the \defgroup block at the beginning of
   the file: */

/** @} */
