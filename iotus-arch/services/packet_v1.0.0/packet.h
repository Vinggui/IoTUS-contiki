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
  struct packet_piece *next;
  struct mmem data;
  iotus_node_t *finalDestinationNode;
  iotus_node_t *nextDestinationNode;
  iotus_node_t *prevSourceNode;
  void (*confirm_cb)(struct packet_piece *packet, iotus_netstack_return returnAns);
  //uint8_t iotusHeader;
  LIST_STRUCT(additionalInfoList);
  timestamp_t timeout;
  iotus_layer_priority priority;
  uint16_t firstHeaderBitSize;
  uint16_t lastHeaderSize;
  uint8_t params;
  uint8_t pktID;
  uint8_t type;
} iotus_packet_t;


typedef void (* packet_sent_cb)(iotus_packet_t *packet, iotus_netstack_return returnAns);
typedef void (* packet_handler)(iotus_packet_t *packet);

/*
 * The address type is defined as a single byte (0 to 255 values).
 * Hence, types 0 to N are the given default pre-defined addresses types for this
 * architecture.
 * 
 * After N, values are divided between ranges that are assigned to each layer of protocol,
 * in case they need to use some specific address not already expected by this architecture.
*/
enum packet_types {
  IOTUS_PACKET_TYPE_IEEE802154_BEACON = 0,
  IOTUS_PACKET_TYPE_IEEE802154_DATA,
  IOTUS_PACKET_TYPE_IEEE802154_ACK,
  IOTUS_PACKET_TYPE_IEEE802154_COMMAND,

  IOTUS_PACKET_TYPE___N
};

/* Application layer has twice the range, so that it can include sub-layers of protocols */
#define IOTUS_PACKET_TYPE_RANGE_PER_LAYER               ((256-IOTUS_PACKET_TYPE___N)/6)
#define IOTUS_PACKET_TYPE_SECURITY_ADDR_TYPE_BEGIN      (IOTUS_PACKET_TYPE___N)
#define IOTUS_PACKET_TYPE_DATA_LINK_ADDR_TYPE_BEGIN     (IOTUS_PACKET_TYPE_RANGE_PER_LAYER+IOTUS_PACKET_TYPE_SECURITY_ADDR_TYPE_BEGIN)
#define IOTUS_PACKET_TYPE_ROUTING_ADDR_TYPE_BEGIN       (IOTUS_PACKET_TYPE_RANGE_PER_LAYER+IOTUS_PACKET_TYPE_DATA_LINK_ADDR_TYPE_BEGIN)
#define IOTUS_PACKET_TYPE_TRANSPORT_ADDR_TYPE_BEGIN     (IOTUS_PACKET_TYPE_RANGE_PER_LAYER+IOTUS_PACKET_TYPE_ROUTING_ADDR_TYPE_BEGIN)
#define IOTUS_PACKET_TYPE_APPLICATION_ADDR_TYPE_BEGIN   (IOTUS_PACKET_TYPE_RANGE_PER_LAYER+IOTUS_PACKET_TYPE_TRANSPORT_ADDR_TYPE_BEGIN)

/////////////////////////////////////////////////////
//                     MACROS                      //
/////////////////////////////////////////////////////
#define packet_set_type(packet, typeValue)      packet->type=typeValue
#define packet_get_type(packet)                 packet->type

//////////////////////////////////////////////////////////////////////////
//                      Functions of this service                       //
//////////////////////////////////////////////////////////////////////////
uint8_t
packet_get_parameter(iotus_packet_t *packet_piece, uint8_t param);

void
packet_set_parameter(iotus_packet_t *packet_piece, uint8_t param);

iotus_node_t *
packet_get_final_destination(iotus_packet_t *packet_piece);

iotus_node_t *
packet_get_next_destination(iotus_packet_t *packetPiece);

Status
packet_set_next_destination(iotus_packet_t *packetPiece, iotus_node_t *node);

Status
packet_set_final_destination(iotus_packet_t *packetPiece, iotus_node_t *node);

Boolean
packet_holds_broadcast(iotus_packet_t *packetiece);

Status
packet_set_tx_channel(iotus_packet_t *packetPiece, uint8_t channel);

int16_t
packet_get_tx_channel(iotus_packet_t *packetPiece);

Status
packet_set_tx_power(iotus_packet_t *packetPiece, int8_t power);

int8_t
packet_get_tx_power(iotus_packet_t *packetPiece);

Status
packet_set_sequence_number(iotus_packet_t *packetPiece, uint8_t sequence);

int16_t
packet_get_sequence_number(iotus_packet_t *packetPiece);

Status
packet_set_rx_netID(iotus_packet_t *packetPiece, uint16_t netID);

Status
packet_set_rx_linkQuality_RSSI(iotus_packet_t *packetPiece, uint8_t linkQuality, uint8_t rssi);

packet_rcv_block_output_t *
packet_get_rx_block(iotus_packet_t *packetPiece);

iotus_packet_t *
packet_create_msg(uint16_t payloadSize, const uint8_t* payload,
    iotus_layer_priority priority, uint16_t timeout, Boolean AllowOptimization,
    iotus_node_t *finalDestination);

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

uint8_t
packet_unwrap_pushed_byte(iotus_packet_t *packetPiece);

uint32_t
packet_unwrap_pushed_bit(iotus_packet_t *packetPiece, int8_t num);

uint16_t
packet_get_payload_size(iotus_packet_t *packetPiece);

uint8_t *
packet_get_payload_data(iotus_packet_t *packetPiece);

void
packet_parse(iotus_packet_t *packetPiece);

Boolean
packet_has_space(iotus_packet_t *packetPiece, uint16_t space);

// void
// packet_send(iotus_packet_t *packetSelected);

// void
// packet_poll_by_priority(uint8_t num);

// void
// packet_poll_by_node(iotus_node_t *node, uint8_t num);

// void
// packet_continue_deferred_packet(iotus_packet_t *packet);

// void
// packet_deliver_upstack(iotus_packet_t *packet);

void
packet_optimize_build(iotus_packet_t *packet, uint16_t freeSpace);

void
packet_set_confirmation_cb(iotus_packet_t *packet, packet_sent_cb func_cb);

void
packet_confirm_transmission(iotus_packet_t *packet, iotus_netstack_return status);

/* This function provides the core access to basic operations into this service */
void
iotus_signal_handler_packet(iotus_service_signal signal, void *data);

#endif /* IOTUS_ARCH_SERVICES_PACKET_SIMPLE_PACKET_H_ */

/* The following stuff ends the \defgroup block at the beginning of
   the file: */

/** @} */
