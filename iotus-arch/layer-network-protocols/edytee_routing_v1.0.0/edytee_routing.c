
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


#if USE_CSMA_MODULE == 1
  #include "csma_contikimac.h"
#endif


#define DEBUG IOTUS_DONT_PRINT//IOTUS_PRINT_IMMEDIATELY
#define THIS_LOG_FILE_NAME_DESCRITOR "edyteeRouting"
#include "safe-printer.h"

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

          iotus_netstack_return status = send(packetForward);
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
start(void)
{
  SAFE_PRINTF_LOG_INFO("Starting edytee routing\n");
}

/*---------------------------------------------------------------------------*/
static void
post_start(void)
{
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
