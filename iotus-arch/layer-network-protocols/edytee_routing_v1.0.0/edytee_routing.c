
/**
 * \defgroup decription...
 *
 * This...
 *
 * @{
 */

/*
 * edytee-routing.c
 *
 *  Created on: Nov 18, 2017
 *      Author: vinicius
 */
#include <stdio.h>
#include "contiki.h"
#include "iotus-api.h"
#include "iotus-netstack.h"
#include "layer-packet-manager.h"
#include "lib/random.h"
#include "sys/timer.h"
#include "sys/rtimer.h"
#include "sys/ctimer.h"


#if USE_CSMA_MODULE == 1
  #include "tree_manager.h"
  #include "csma_contikimac.h"
#endif


#define DEBUG IOTUS_PRINT_IMMEDIATELY//IOTUS_DONT_PRINT//IOTUS_PRINT_IMMEDIATELY
#define THIS_LOG_FILE_NAME_DESCRITOR "edyteeRouting"
#include "safe-printer.h"

#define RPL_DAO_PERIOD_TIME                   CONTIKIMAC_ND_PERIOD_TIME
#define RPL_ND_BACKOFF_TIME                   CONTIKIMAC_ND_BACKOFF_TIME
#define RPL_ND_SCAN_TIME                      CONTIKIMAC_ND_SCAN_TIME

//Timer for sending neighbor discovery
static struct ctimer sendNDTimer;
static clock_time_t backOffDifference;
iotus_node_t *rootNode;
iotus_node_t *fatherNode;
static uint8_t private_keep_alive[12];
static uint8_t gPkt_created = 0;


/*---------------------------------------------------------------------------*/
static iotus_netstack_return
send(iotus_packet_t *packet)
{
  if(NODES_BROADCAST == packet->finalDestinationNode) {
    packet->nextDestinationNode = NODES_BROADCAST;
  } else {
    //Get the final static destination
    uint8_t *finalDestLastAddress = nodes_get_address(IOTUS_ADDRESSES_TYPE_ADDR_SHORT,
                                          packet->finalDestinationNode);
    uint8_t address[2] = {1,0};
    fatherNode = nodes_update_by_address(IOTUS_ADDRESSES_TYPE_ADDR_SHORT, address);

    packet->nextDestinationNode = fatherNode;

    uint8_t bitSequence[1];
    bitSequence[0] = finalDestLastAddress[0];
    packet_push_bit_header(8, bitSequence, packet);
  }

#if USE_CSMA_MODULE == 1
  return csma_send_packet(packet);
#else
  return active_data_link_protocol->send(packet);
#endif
}


/*---------------------------------------------------------------------------*/
static void
send_cb(iotus_packet_t *packet, iotus_netstack_return returnAns)
{
  SAFE_PRINTF_LOG_INFO("Frame %p processed %u", packet, returnAns);
  // if(returnAns == MAC_TX_OK) {
    packet_destroy(packet);
  // }
}

/*---------------------------------------------------------------------------*/
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

  printf("got net %s\n", pieces_get_data_pointer(packet));

  if(finalDestAddr == addresses_self_get_pointer(IOTUS_ADDRESSES_TYPE_ADDR_SHORT)[0]) {
    //This is for us...
    active_transport_protocol->receive(packet);
    return RX_PROCESSED;
  } else {
    iotus_packet_t *packetForward = NULL;


    //search for the next node...
    uint8_t ourAddr = addresses_self_get_pointer(IOTUS_ADDRESSES_TYPE_ADDR_SHORT)[0];

    uint8_t nextHop = ourAddr-1;

    if(nextHop != 0) {
        uint8_t address2[2] = {1,0};
        rootNode = nodes_update_by_address(IOTUS_ADDRESSES_TYPE_ADDR_SHORT, address2);
        if(rootNode != NULL) {
          packetForward = iotus_initiate_packet(
                              packet_get_payload_size(packet),
                              packet_get_payload_data(packet),
                              packet->params | PACKET_PARAMETERS_WAIT_FOR_ACK,
                              IOTUS_PRIORITY_ROUTING,
                              ROUTING_PACKETS_TIMEOUT,
                              rootNode,
                              send_cb);

          if(NULL == packetForward) {
            SAFE_PRINTF_LOG_INFO("Packet failed");
            return RX_ERR_DROPPED;
          }

          #if DEBUG != IOTUS_DONT_PRINT
          iotus_netstack_return status = send(packetForward);
          #else
          send(packetForward);
          #endif
          SAFE_PRINTF_LOG_INFO("Packet %u forwarded %u stats %u\n", packet->pktID, packetForward->pktID, status);
          // // if (!(MAC_TX_OK == status ||
          // //     MAC_TX_DEFERRED == status)) {
          // if (MAC_TX_DEFERRED != status) {
          //   send_cb(packetForward, status);
          //   // printf("Packet fwd del %u\n", packetForward->pktID);
          //   // packet_destroy(packetForward);
          // }
        }
    }
    return RX_PROCESSED;
  }

}
/*---------------------------------------------------------------------------*/
static void
control_frames_nd_cb(iotus_packet_t *packet, iotus_netstack_return returnAns)
{
  SAFE_PRINTF_LOG_INFO("nd %p sent %u", packet, returnAns);
  // if(returnAns == MAC_TX_OK) {
  packet_destroy(packet);
  // }
}
/*---------------------------------------------------------------------------*/
static void
send_beacon(void *ptr)
{
  clock_time_t backoff = CLOCK_SECOND*RPL_DAO_PERIOD_TIME - backOffDifference;//ms
  backOffDifference = (CLOCK_SECOND*((random_rand()%RPL_ND_BACKOFF_TIME)))/1000;

  backoff += backOffDifference;
  ctimer_set(&sendNDTimer, backoff, send_beacon, NULL);

  //DAO packets have 4 bytes of base size
  sprintf((char *)private_keep_alive, "%uRnk", treePersonalRank);

  if(IOTUS_PRIORITY_ROUTING == iotus_get_layer_assigned_for(IOTUS_CHORE_ONEHOP_BROADCAST)) {
    iotus_packet_t *packet = iotus_initiate_packet(
                              4,
                              private_keep_alive,
                              PACKET_PARAMETERS_WAIT_FOR_ACK|PACKET_PARAMETERS_ALLOW_PIGGYBACK,
                              IOTUS_PRIORITY_ROUTING,
                              5000,
                              NODES_BROADCAST,
                              control_frames_nd_cb);

    if(NULL == packet) {
      SAFE_PRINTF_LOG_INFO("Packet failed");
      return;
    }

    packet_set_type(packet, IOTUS_PACKET_TYPE_IEEE802154_COMMAND);
   
    SAFE_PRINTF_LOG_INFO("DAO nd %u \n", packet->pktID);
    #if USE_CSMA_MODULE == 1
      csma_send_packet(packet);
    #else
      active_data_link_protocol->send(packet);
    #endif
  } else {
    // if(IOTUS_PRIORITY_ROUTING == iotus_get_layer_assigned_for(IOTUS_CHORE_TREE_BUILDING) ||
    //    IOTUS_PRIORITY_ROUTING == iotus_get_layer_assigned_for(IOTUS_CHORE_NEIGHBOR_DISCOVERY)) {
    SAFE_PRINTF_LOG_INFO("Creating piggy DAO\n");
    piggyback_create_piece(4, private_keep_alive, IOTUS_PRIORITY_ROUTING, NODES_BROADCAST, RPL_DAO_PERIOD_TIME*1000L+RPL_ND_BACKOFF_TIME);
  }
}

/*---------------------------------------------------------------------------*/
static void
start(void)
{
  SAFE_PRINTF_LOG_INFO("Starting edytee routing\n");

  iotus_subscribe_for_chore(IOTUS_PRIORITY_DATA_LINK, IOTUS_CHORE_APPLY_PIGGYBACK);
  iotus_subscribe_for_chore(IOTUS_PRIORITY_DATA_LINK, IOTUS_CHORE_NEIGHBOR_DISCOVERY);
  iotus_subscribe_for_chore(IOTUS_PRIORITY_DATA_LINK, IOTUS_CHORE_ONEHOP_BROADCAST);
  iotus_subscribe_for_chore(IOTUS_PRIORITY_DATA_LINK, IOTUS_CHORE_FLOODING);
  iotus_subscribe_for_chore(IOTUS_PRIORITY_DATA_LINK, IOTUS_CHORE_TREE_BUILDING);
}

/*---------------------------------------------------------------------------*/
static void
post_start(void)
{

  // if(IOTUS_PRIORITY_ROUTING == iotus_get_layer_assigned_for(IOTUS_CHORE_APPLY_PIGGYBACK)) {
  //   SAFE_PRINTF_LOG_INFO("Network assign piggyback");
  // }
  // if(IOTUS_PRIORITY_ROUTING == iotus_get_layer_assigned_for(IOTUS_CHORE_NEIGHBOR_DISCOVERY)) {
  //   SAFE_PRINTF_LOG_INFO("Network assign ND");
  // }

  #if EXP_CONTIKIMAC_802_15_4 == 1
  if(treeRouter &&
     addresses_self_get_pointer(IOTUS_ADDRESSES_TYPE_ADDR_SHORT)[0] == 1) {
    //This is the root...
    
    // backOffDifference = (CLOCK_SECOND*((random_rand()%RPL_ND_BACKOFF_TIME)))/1000;
    // clock_time_t backoff = CLOCK_SECOND*RPL_DAO_PERIOD_TIME + backOffDifference;//ms
    // ctimer_set(&sendNDTimer, backoff, send_beacon, NULL);

    if(IOTUS_PRIORITY_ROUTING == iotus_get_layer_assigned_for(IOTUS_CHORE_NEIGHBOR_DISCOVERY)) {
      SAFE_PRINTF_LOG_INFO("Network assign ND");
    }
    if(IOTUS_PRIORITY_ROUTING == iotus_get_layer_assigned_for(IOTUS_CHORE_TREE_BUILDING)) {
      SAFE_PRINTF_LOG_INFO("Network assign tree");
      treePersonalRank = 1;
    }
  }
#endif /* EXP_CONTIKIMAC_802_15_4 */
}

static void
close(void)
{
  
}

struct iotus_network_protocol_struct edytee_routing_protocol = {
  start,
  post_start,
  close,
  send,
  send_cb,
  input_packet
};

/* The following stuff ends the \defgroup block at the beginning of
   the file: */

/** @} */
