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
#include "pieces.h"
#include "nodes.h"
#include "packet-default-additional-info.h"
#include "packet-default-chores.h"

#ifndef IOTUS_RADIO_FULL_ADDRESS
  #error Please define IOTUS_RADIO_FULL_ADDRESS into platform-conf.h
#endif

/*
 * Defines of this module
 */
typedef enum IOTUS_PACKET_PRIORITY {IOTUS_PRIORITY_NONE = 0, IOTUS_PRIORITY_DATA_LINK,
    IOTUS_PRIORITY_ROUTING, IOTUS_PRIORITY_TRANSPORT} iotus_packets_priority;


typedef struct packet_piece {
  COMMON_STRUCT_PIECES(struct packet_piece);
  Iotus_node *nextDestinationNode;\
  uint16_t totalPacketSize;//Will be used to build the final packet, processed by the core
  uint16_t initialBitHeaderSize;
  uint8_t *initialBitHeader; //Will be used to build the final packet, processed by the core
  uint16_t finalBytesHeaderSize;
  uint8_t *finalBytesHeader; //Will be used to build the final packet, processed by the core
  LIST_STRUCT(infoPieces);
} iotus_packet_t;

/*
 * Functions of this module
 */
uint8_t
packet_verify_parameter(iotus_packet_t *packet_piece, uint8_t param);

void
packet_set_parameter(iotus_packet_t *packet_piece, uint8_t param);

Iotus_node *
packet_get_final_destination(iotus_packet_t *packet_piece);

iotus_packet_t *
packet_create_msg(uint16_t payloadSize, iotus_packets_priority priority,
    uint16_t timeout, const uint8_t* payload,
    Iotus_node *finalDestination, void *callbackFunction);

void
packet_delete_msg(iotus_packet_t *msgPiece);

uint16_t
packet_get_size(iotus_packet_t *packet_piece);

void
packet_subscribe_for_chore(iotus_packets_priority priority,
  iotus_default_header_chores func);

uint8_t
packet_get_layer_assigned_for(iotus_default_header_chores func);

uint16_t
packet_push_bit_header(uint16_t bitSequenceSize, const uint8_t *bitSeq,
  iotus_packet_t *packet_piece);

uint16_t
packet_append_byte_header(uint16_t byteSequenceSize, const uint8_t *headerToAppend,
  iotus_packet_t *packet_piece);

uint8_t
packet_read_byte(uint16_t bytePos, iotus_packet_t *packet_piece);


/* This function provides the core access to basic operations into this service */
void
iotus_signal_handler_packet(iotus_service_signal signal, void *data);

#endif /* IOTUS_ARCH_SERVICES_PACKET_SIMPLE_PACKET_H_ */

/* The following stuff ends the \defgroup block at the beginning of
   the file: */

/** @} */
