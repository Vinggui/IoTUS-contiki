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

#ifndef IOTUS_ARCH_SERVICES_PIGGYBACK_PIGGYBACK_H_
#define IOTUS_ARCH_SERVICES_PIGGYBACK_PIGGYBACK_H_

#include "iotus-core.h"
#include "platform-conf.h"
#include "timestamp.h"


#ifndef IOTUS_PIGGYBACK_LIST_SIZE
  #error IOTUS_PIGGYBACK_LIST_SIZE not defined! Define it into your platform-conf.h
#endif


#define PIGGYBACK_MAX_FRAME_SIZE                  0x0FFF
#define PIGGYBACK_SINGLE_HEADER_FRAME             0x000F
#define PIGGYBACK_MAX_ATTACHED_PIECES_POWER       4//=16 Has to be a power of two

typedef struct piggyback_piece {
  struct piggyback_piece *next;
  struct mmem data;
  iotus_layer_priority priority;
  iotus_node_t *finalDestinationNode;
  timestamp_t timeout;
  uint8_t params;
  uint8_t pktID;
  uint8_t extendedSize;
} iotus_piggyback_t;

#define IOTUS_PIGGYBACK_LAYER                                 0b11000000
#define IOTUS_PIGGYBACK_ATTACHMENT_TYPE_FINAL_DEST            0b00100000
#define IOTUS_PIGGYBACK_ATTACHMENT_WITH_EXTENDED_SIZE         0b00010000
#define IOTUS_PIGGYBACK_ATTACHMENT_SIZE_MASK                  0b00001111



#define IOTUS_PIGGYBACK_GENERAL_HDR_NUMBER_PIECES             0b00001111

Boolean
piggyback_destroy(iotus_piggyback_t *piece);

void
piggyback_confirm_sent(iotus_packet_t *packet, uint8_t status);

iotus_piggyback_t *
piggyback_create_piece(uint16_t headerSize, const uint8_t* headerData,
    uint8_t targetLayer, iotus_node_t *destinationNode, int16_t timeout);


void
piggyback_unwrap_payload(iotus_packet_t *packet);

uint16_t
piggyback_apply(iotus_packet_t *packet_piece, uint16_t availableSpace);

void
iotus_signal_handler_piggyback(iotus_service_signal signal, void *data);

#endif /* IOTUS_ARCH_SERVICES_PIGGYBACK_PIGGYBACK_H_ */

/* The following stuff ends the \defgroup block at the beginning of
   the file: */

/** @} */
