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

#ifndef IOTUS_ARCH_SERVICES_PACKET_SIMPLE_PACKET_H_
#define IOTUS_ARCH_SERVICES_PACKET_SIMPLE_PACKET_H_

#include "platform-conf.h"

#ifndef IOTUS_RADIO_FULL_ADDRESS
  #error Please define IOTUS_RADIO_FULL_ADDRESS into platform-conf.h
#endif

/*
 * Defines of this module
 */
typedef enum IOTUS_PACKET_PRIORITY {IOTUS_PRIORITY_NONE = 0, IOTUS_PRIORITY_DATA_LINK,
    IOTUS_PRIORITY_ROUTING, IOTUS_PRIORITY_TRANSPORT} iotus_packets_priority;

/*
 * List of default known headers functions present into header
 */
typedef enum IOTUS_DEFAULT_HEADER_CHORES {
  IOTUS_DEFAULT_HEADER_CHORE_SOURCE_ADDRESS = 0,
  IOTUS_DEFAULT_HEADER_CHORE_CHECKSUM,
  IOTUS_DEFAULT_HEADER_CHORE_FRAGMENT,
  IOTUS_DEFAULT_HEADER_CHORE_SEQUENCE_NUMBER,

  IOTUS_DEFAULT_HEADER_CHORE_ONEHOP_BROADCAST,


  IOTUS_FINAL_NUMBER_DEFAULT_HEADER_CHORE
} iotus_default_header_chores;

/*
 * Functions of this module
 */
struct msg_piece *
packet_create_msg_piece(uint16_t payloadSize, uint8_t allowAggregation,
    uint8_t allowFragmentation, iotus_packets_priority priority,
    uint16_t timeout, const uint8_t* payload,
    const uint8_t *finalDestination, void *callbackFunction);

void
packet_delete_msg_piece(void *msgPiece);

void
packet_subscribe_default_header_chore(iotus_packets_priority priority,
  iotus_default_header_chores func);

uint8_t
packet_verify_default_header_chore(iotus_packets_priority priority,
  iotus_default_header_chores func);


/* This function provides the core access to basic operations into this service */
void
iotus_signal_handler_packet(iotus_service_signal signal, void *data);

#endif /* IOTUS_ARCH_SERVICES_PACKET_SIMPLE_PACKET_H_ */

/* The following stuff ends the \defgroup block at the beginning of
   the file: */

/** @} */
