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

void *
packet_create_msg_piece(uint8_t headerSize, uint16_t payloadSize,
    uint8_t allowAggregation, uint8_t allowFragmentation,
    uint16_t timeout, uint8_t txPwr,
    const uint8_t* header, const uint8_t* payload);

void
packet_delete_msg_piece(void *msgPiece)*

#endif /* IOTUS_ARCH_SERVICES_PACKET_SIMPLE_PACKET_H_ */


/* The following stuff ends the \defgroup block at the beginning of
   the file: */

/** @} */
