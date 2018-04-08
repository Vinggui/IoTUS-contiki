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
#include "chores.h"

#ifndef IOTUS_RADIO_FULL_ADDRESS
  #error Please define IOTUS_RADIO_FULL_ADDRESS into platform-conf.h
#endif


#ifndef IOTUS_PACKET_LIST_SIZE
  #error IOTUS_PACKET_LIST_SIZE not defined! Define it into your platform-conf.h
#endif

///////////////////////////////////////////////////////
//             Defines of this module                //
///////////////////////////////////////////////////////
typedef struct packet_piece {
  COMMON_STRUCT_PIECES(struct packet_piece);
  iotus_node_t *nextDestinationNode;
  uint8_t iotusHeader;
  uint16_t firstHeaderBitSize;
  uint16_t lastHeaderSize;
  LIST_STRUCT(additionalInfoList);
} iotus_packet_t;


/*
 * The address type is defined as a single byte (0 to 255 values).
 * Hence, types 0 to N are the given default pre-defined addresses types for this
 * architecture.
 * 
 * After N, values are divided between ranges that are assigned to each layer of protocol,
 * in case they need to use some specific address not already expected by this architecture.
*/
enum packet_types {
  IOTUS_PACKET_TYPE_UNDEFINED = 0,

  IOTUS_PACKET_TYPE___N
};

/* Application layer has twice the range, so that it can include sub-layers of protocols */
#define IOTUS_PACKET_TYPE_RANGE_PER_LAYER               ((256-IOTUS_PACKET_TYPE___N)/6)
#define IOTUS_PACKET_TYPE_SECURITY_ADDR_TYPE_BEGIN      (IOTUS_PACKET_TYPE___N)
#define IOTUS_PACKET_TYPE_DATA_LINK_ADDR_TYPE_BEGIN     (IOTUS_PACKET_TYPE_RANGE_PER_LAYER+IOTUS_PACKET_TYPE_SECURITY_ADDR_TYPE_BEGIN)
#define IOTUS_PACKET_TYPE_ROUTING_ADDR_TYPE_BEGIN       (IOTUS_PACKET_TYPE_RANGE_PER_LAYER+IOTUS_PACKET_TYPE_DATA_LINK_ADDR_TYPE_BEGIN)
#define IOTUS_PACKET_TYPE_TRANSPORT_ADDR_TYPE_BEGIN     (IOTUS_PACKET_TYPE_RANGE_PER_LAYER+IOTUS_PACKET_TYPE_ROUTING_ADDR_TYPE_BEGIN)
#define IOTUS_PACKET_TYPE_APPLICATION_ADDR_TYPE_BEGIN   (IOTUS_PACKET_TYPE_RANGE_PER_LAYER+IOTUS_PACKET_TYPE_TRANSPORT_ADDR_TYPE_BEGIN)



/*
 * Functions of this module
 */
uint8_t
packet_verify_parameter(iotus_packet_t *packet_piece, uint8_t param);

void
packet_set_parameter(iotus_packet_t *packet_piece, uint8_t param);

iotus_node_t *
packet_get_final_destination(iotus_packet_t *packet_piece);

iotus_node_t *
packet_get_next_destination(iotus_packet_t *packetPiece);

Status
packet_set_tx_block(iotus_packet_t *packetPiece, int8_t power, uint8_t channel);

Status
packet_set_rx_block(iotus_packet_t *packetPiece, uint16_t netId, uint8_t linkQuality, uint8_t rssi);

int8_t
packet_get_tx_power(iotus_packet_t *packetPiece);

iotus_packet_t *
packet_create_msg(uint16_t payloadSize, iotus_layer_priority priority,
    uint16_t timeout, const uint8_t* payload, Boolean insertIotusHeader,
    iotus_node_t *finalDestination, void *callbackFunction);

Boolean
packet_destroy(iotus_packet_t *msgPiece);

unsigned int
packet_get_size(iotus_packet_t *packet_piece);

uint16_t
packet_push_bit_header(uint16_t bitSequenceSize, const uint8_t *bitSeq,
  iotus_packet_t *packet_piece);

uint16_t
packet_append_last_header(uint16_t byteSequenceSize, const uint8_t *headerToAppend,
  iotus_packet_t *packet_piece);

uint8_t
packet_read_byte(uint16_t bytePos, iotus_packet_t *packet_piece);

Status
packet_unwrap_appended_byte(iotus_packet_t *packetPiece, uint8_t *buf, uint16_t num);

uint32_t
packet_unwrap_pushed_bit(iotus_packet_t *packetPiece, uint8_t num);

void
packet_parse(iotus_packet_t *packetPiece);

/* This function provides the core access to basic operations into this service */
void
iotus_signal_handler_packet(iotus_service_signal signal, void *data);

#endif /* IOTUS_ARCH_SERVICES_PACKET_SIMPLE_PACKET_H_ */

/* The following stuff ends the \defgroup block at the beginning of
   the file: */

/** @} */
