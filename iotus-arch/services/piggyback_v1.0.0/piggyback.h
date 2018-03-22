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


typedef struct piggyback_piece {
  COMMON_STRUCT_PIECES(struct piggyback_piece);
  uint8_t extendedSize;
} iotus_piggyback_t;

#define IOTUS_PIGGYBACK_LAYER                                 0b00000011
#define IOTUS_PIGGYBACK_ATTACHMENT_TYPE_FINAL_DEST            0b00000100
#define IOTUS_PIGGYBACK_ATTACHMENT_WITH_EXTENDED_SIZE         0b00001000
#define IOTUS_PIGGYBACK_ATTACHMENT_SIZE_MASK                  0b11110000

void
piggyback_delete_piece(iotus_piggyback_t *piece);

iotus_piggyback_t *
piggyback_create_piece(uint16_t headerSize, const uint8_t* headerData,
    uint8_t targetLayer, iotus_node_t *destinationNode, int32_t timeout);

uint16_t
piggyback_apply(iotus_packet_t *packet_piece);

void
iotus_signal_handler_piggyback(iotus_service_signal signal, void *data);

#endif /* IOTUS_ARCH_SERVICES_PIGGYBACK_PIGGYBACK_H_ */

/* The following stuff ends the \defgroup block at the beginning of
   the file: */

/** @} */
