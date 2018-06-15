
/**
 * \defgroup description...
 *
 * This...
 *
 * @{
 */

/*
 * null-routing.c
 *
 *  Created on: Nov 18, 2017
 *      Author: vinicius
 */
#include <stdio.h>
#include "contiki.h"
#include "iotus-netstack.h"
#include "piggyback.h"
#include "sys/timer.h"

#define DEBUG IOTUS_PRINT_IMMEDIATELY
#define THIS_LOG_FILE_NAME_DESCRITOR "nullRouting"
#include "safe-printer.h"

#define NEIGHBOR_DISCOVERY_INTERVAL       15//sec
#define ROUTING_PACKETS_TIMEOUT           5000//msec

// Next dest table using final value{source, final destination}
int routing_table[9][9] =
{
  //0=>
  {0,   0,   0,   0,   0,   0,   0,   0,   0},
  //1=>
  {0,   0,   2,   3,   2,   2,   3,   3,   3},
  //2=>
  {0,   1,   0,   3,   4,   5,   3,   3,   3},
  //3=>
  {0,   1,   2,   0,   2,   2,   6,   7,   8},
  //4=>
  {0,   2,   2,   2,   0,   5,   2,   2,   2},
  //5=>
  {0,   2,   2,   2,   4,   0,   2,   2,   2},
  //6=>
  {0,   3,   3,   3,   3,   3,   0,   7,   8},
  //7=>
  {0,   3,   3,   3,   3,   3,   6,   0,   8},
  //8=>
  {0,   3,   3,   3,   3,   3,   6,   7,   0}
};

//Timer for sending neighbor discovery
static struct timer sendND;
static iotus_node_t *rootNode;


static iotus_netstack_return
send(iotus_packet_t *packet)
{
  //Get the final static destination
   uint8_t *finalDestLastAddress = nodes_get_address(IOTUS_ADDRESSES_TYPE_ADDR_SHORT,
                                        packet->finalDestinationNode);

  uint8_t nextHop = routing_table[addresses_self_get_pointer(IOTUS_ADDRESSES_TYPE_ADDR_SHORT)[0]][finalDestLastAddress[0]];
  
  if(nextHop == 0) {
    //This is for ourself. Cancel...
    return ROUTING_TX_ERR;
  }

  printf("Final %u next %u\n",finalDestLastAddress[0],nextHop);

  uint8_t addressNext[2] = {nextHop,0};
  iotus_node_t *nextHopNode = nodes_update_by_address(IOTUS_ADDRESSES_TYPE_ADDR_SHORT, addressNext);

  if(NULL == nextHopNode) {
    SAFE_PRINTF_LOG_ERROR("No next hop");
    return ROUTING_TX_ERR;
  }

  packet->nextDestinationNode = nextHopNode;

  uint8_t bitSequence[1];
  bitSequence[0] = finalDestLastAddress[0];
  packet_push_bit_header(8, bitSequence, packet);
  
  //active_data_link_protocol->send(packet);
  return ROUTING_TX_OK;
}

static iotus_netstack_return
input_packet(iotus_packet_t *packet)
{
  // SAFE_PRINTF_CLEAN("Got packet: ");
  // int i;
  // for (i = 0; i < packet_get_payload_size(packet); ++i)
  // {
  //   SAFE_PRINTF_CLEAN("%02x ", packet_get_payload_data(packet)[i]);
  // }
  // SAFE_PRINTF_CLEAN("\n");

  uint8_t finalDestAddr = packet_unwrap_pushed_byte(packet);

  if(finalDestAddr == addresses_self_get_pointer(IOTUS_ADDRESSES_TYPE_ADDR_SHORT)[0]) {
    //This is for us...
    return RX_SEND_UP_STACK;
  } else {
    iotus_packet_t *packetForward = NULL;

    //search for the next node...
    uint8_t nextHop = routing_table[addresses_self_get_pointer(IOTUS_ADDRESSES_TYPE_ADDR_SHORT)[0]][finalDestAddr];

    if(nextHop != 0) {

      uint8_t dest[2];
      dest[0] = nextHop;
      dest[1] = 0;
      iotus_node_t *destNode = nodes_update_by_address(IOTUS_ADDRESSES_TYPE_ADDR_SHORT, dest);
      if(destNode != NULL) {

        packetForward = packet_create_msg(
                          packet_get_payload_size(packet),
                          packet_get_payload_data(packet),
                          IOTUS_PRIORITY_ROUTING,
                          ROUTING_PACKETS_TIMEOUT,
                          TRUE,
                          destNode);

        if(NULL == packetForward) {
          SAFE_PRINTF_LOG_INFO("Packet failed");
          return RX_ERR_DROPPED;
        }
        packet_set_parameter(packetForward, packet->params);
        SAFE_PRINTF_LOG_INFO("Packet %p forwarded into %p", packet, packetForward);
      }

      send(packetForward);
    }
    return RX_PROCESSED;
  }

}


static void
send_cb(iotus_packet_t *packet, iotus_netstack_return returnAns)
{
  SAFE_PRINTF_LOG_INFO("Frame %p processed %u", packet, returnAns);
}

static void
start(void)
{
  printf("Starting null routing\n");

  iotus_subscribe_for_chore(IOTUS_PRIORITY_ROUTING, IOTUS_CHORE_NEIGHBOR_DISCOVERY);

  uint8_t rootValue = 0;
  int i;
  for(i=0; i<3; i++) {
    rootValue = routing_table[addresses_self_get_pointer(IOTUS_ADDRESSES_TYPE_ADDR_SHORT)[0]][i];
    if(rootValue != 0) {
      break;
    }
  }

  uint8_t address[2] = {rootValue,0};
  rootNode = nodes_update_by_address(IOTUS_ADDRESSES_TYPE_ADDR_SHORT, address);
}


static void
post_start(void)
{
  if(IOTUS_PRIORITY_ROUTING == iotus_get_layer_assigned_for(IOTUS_CHORE_NEIGHBOR_DISCOVERY)) {
    timer_set(&sendND, CLOCK_SECOND*NEIGHBOR_DISCOVERY_INTERVAL);
    piggyback_create_piece(30, (uint8_t *)"123456789012345678901234567890", IOTUS_PRIORITY_ROUTING, rootNode, NEIGHBOR_DISCOVERY_INTERVAL*1000);
  }
}

static void
run(void)
{
  iotus_core_netstack_idle_for(IOTUS_PRIORITY_ROUTING, 0XFFFF);
  //Test which layer is supposed to do neighbor discovery
  if(IOTUS_PRIORITY_ROUTING == iotus_get_layer_assigned_for(IOTUS_CHORE_NEIGHBOR_DISCOVERY)) {
    if(timer_expired(&sendND)) {
      timer_restart(&sendND);

      piggyback_create_piece(30, (uint8_t *)"123456789012345678901234567890", IOTUS_PRIORITY_ROUTING, rootNode, NEIGHBOR_DISCOVERY_INTERVAL*1000);
    }
  }
  
}

static void
close(void)
{}

struct iotus_routing_protocol_struct null_routing_protocol = {
  start,
  post_start,
  run,
  close,
  send,
  send_cb,
  input_packet
};
/* The following stuff ends the \defgroup block at the beginning of
   the file: */

/** @} */
