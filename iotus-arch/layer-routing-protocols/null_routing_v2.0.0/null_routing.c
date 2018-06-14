
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

// {source -1, final destination -1}
int routing_table[8][8] =
{
  //1=>
  {0,2,3,2,2,3,3,3},
  //2=>
  {1,0,0,4,5,0,0,0},
  //3=>
  {1,0,0,0,0,6,7,8},
  //4=>
  {0,2,0,0,5,0,0,0},
  //5=>
  {0,2,0,4,0,0,0,0},
  //6=>
  {0,0,3,0,0,0,7,8},
  //7=>
  {0,0,3,0,0,6,0,8},
  //8=>
  {0,0,3,0,0,6,7,0}
};

//Timer for sending neighbor discovery
static struct timer sendND;
static iotus_node_t *rootNode;

static iotus_netstack_return
input_packet(iotus_packet_t *packet)
{
  SAFE_PRINTF_CLEAN("Got packet: ");
  int i;
  for (i = 0; i < packet_get_payload_size(packet); ++i)
  {
    SAFE_PRINTF_CLEAN("%02x ", packet_get_payload_data(packet)[i]);
  }
  SAFE_PRINTF_CLEAN("\n");
  return RX_SEND_UP_STACK;
}

static iotus_netstack_return
send(iotus_packet_t *packet)
{
  //Get the final static destination
  //uint8_t finalDestLastAddress = nodes_get_address(IOTUS_ADDRESSES_TYPE_ADDR_SHORT,
  //                                      packet->finalDestinationNode);
  //if()
  packet->nextDestinationNode = packet->finalDestinationNode;
  //active_data_link_protocol->send(packet);
  return ROUTING_TX_OK;
}


static void
send_cb(iotus_packet_t *packet, iotus_netstack_return returnAns)
{
  SAFE_PRINTF_LOG_INFO("Frame processed %u", returnAns);
}

static void
start(void)
{
  printf("Starting null routing\n");

  iotus_subscribe_for_chore(IOTUS_PRIORITY_ROUTING, IOTUS_CHORE_NEIGHBOR_DISCOVERY);

  uint8_t address[2] = {1,0};
  rootNode = nodes_update_by_address(IOTUS_ADDRESSES_TYPE_ADDR_SHORT, address);
}


static void
post_start(void)
{
  if(IOTUS_PRIORITY_ROUTING == iotus_get_layer_assigned_for(IOTUS_CHORE_NEIGHBOR_DISCOVERY)) {
    timer_set(&sendND, CLOCK_SECOND*NEIGHBOR_DISCOVERY_INTERVAL);
    piggyback_create_piece(12, "123456789012", IOTUS_PRIORITY_ROUTING, rootNode, NEIGHBOR_DISCOVERY_INTERVAL*1000);
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

      piggyback_create_piece(12, "123456789012", IOTUS_PRIORITY_ROUTING, rootNode, NEIGHBOR_DISCOVERY_INTERVAL*1000);
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
