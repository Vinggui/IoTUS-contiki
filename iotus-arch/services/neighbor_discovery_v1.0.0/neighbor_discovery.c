
/**
 * \defgroup description...
 *
 * This...
 *
 * @{
 */

/*
 * null.c
 *
 *  Created on: Feb 27, 2018
 *      Author: vinicius
 */
#include <string.h>
#include "addresses.h"
#include "contiki.h"
#include "iotus-api.h"
#include "iotus-core.h"
#include "iotus-netstack.h"
#include "layer-packet-manager.h"
#include "neighbor_discovery.h"



#define DEBUG IOTUS_PRINT_IMMEDIATELY//IOTUS_DONT_PRINT//
#define THIS_LOG_FILE_NAME_DESCRITOR "NDisc"
#include "safe-printer.h"


/*
 * Each piece has the first byte as the layer.
 * Second byte as the size of the piece inserted..
*/


uint16_t ndKeepAlivePeriod;
uint16_t ndAssociation_answer_delay;

LIST(gNDPieces);

// static uint8_t gLayersDoingND = 0;
static uint8_t gNDOperations[ND_PKT_MAX_VALUE-1] = {0};
static nd_cb_func gLayersCB[IOTUS_MAX_LAYER_NUM-1] = {NULL};
static uint8_t gMsgPayload[20];



// /*---------------------------------------------------------------------------*/
void
nd_remove_subscription(iotus_layer_priority layer)
{
  iotus_additional_info_t *h = list_head(gNDPieces);
  while(NULL != h) {
    iotus_additional_info_t *next = list_item_next(h);

    /*
     * The h->type is the operation
     * The first byte is the size of this piece [0]
     * The second byte is the layer of this piece [1]
     */
    uint8_t *payload = pieces_get_data_pointer(h);
    if(payload[1] == layer) {
      pieces_destroy_additional_info(gNDPieces,h);
    }
    h = next;
  }

  uint8_t i=0;
  for(; i<ND_PKT_MAX_VALUE-1; i++) {
    gNDOperations[i] &= ~(1<<layer);
  }
  gLayersCB[layer] = NULL;
}

// /*---------------------------------------------------------------------------*/
// uint8_t
// nd_create_pkt(uint16_t payloadSize, const uint8_t* payload, iotus_layer_priority layer,
//               nd_pkt_types pkt_type,
//               uint16_t timeout, iotus_node_t *finalDestination, packet_sent_cb func_cb)
// {
//   if(gNDOperations[pkt_type]) {
//     uint8_t address3[2] = {1,0};
//     rootNode = nodes_update_by_address(IOTUS_ADDRESSES_TYPE_ADDR_SHORT, address3);
//     SAFE_PRINTF_LOG_INFO("Creating piggy routing\n");
//     piggyback_create_piece(12, private_keep_alive, IOTUS_PRIORITY_ROUTING, rootNode, ROUTING_PACKETS_TIMEOUT);
//   } else {
//     iotus_packet_t *packet = iotus_initiate_packet(
//                               12,
//                               private_nd_control,
//                               PACKET_PARAMETERS_WAIT_FOR_ACK,
//                               IOTUS_PRIORITY_ROUTING,
//                               5000,
//                               NODES_BROADCAST,
//                               send_cb);

//     if(NULL == packet) {
//       SAFE_PRINTF_LOG_INFO("Packet failed");
//       return;
//     }

//     packet_set_type(packet, IOTUS_PACKET_TYPE_IEEE802154_BEACON);
   
//     SAFE_PRINTF_LOG_INFO("Packet nd %u \n", packet->pktID);
//     active_data_link_protocol->send(packet);
//   }
// }

// /*---------------------------------------------------------------------------*/
void
nd_set_operation_msg(iotus_layer_priority layer, nd_pkt_types operation, uint8_t size, uint8_t* payload)
{
  if(size>0) {
    //Ths type indicates which operation is saved there
    uint8_t *data = pieces_modify_additional_info_var(gNDPieces, operation, size+2, TRUE);
    if(NULL == data) {
      SAFE_PRINTF_LOG_ERROR("Association piece no assigned");
      return;
    }

    data[0] = size+2;
    data[1] = layer;
    memcpy(data+2, payload, size);
  }
}
// ---------------------------------------------------------------------------
// uint8_t
// nd_get_assigned_layer(iotus_layer_priority layer)
// {
//   return (gLayersDoingND & (1<<layer));
// }
/*---------------------------------------------------------------------------*/
void
nd_set_layer_operations(iotus_layer_priority layer, nd_pkt_types op)
{
  gNDOperations[op] |= (1<<layer);
}
/*---------------------------------------------------------------------------*/
void
nd_set_layer_cb(iotus_layer_priority layer, nd_cb_func cb)
{
  gLayersCB[layer] = cb;
}
/*---------------------------------------------------------------------------*/
uint8_t
nd_get_layer_operations(nd_pkt_types op)
{
  return gNDOperations[op];
}
/*---------------------------------------------------------------------------*/
// static void
// send_cb(iotus_packet_t *packet, iotus_netstack_return returnAns)
// {
//   SAFE_PRINTF_LOG_INFO("Frame %p processed %u", packet, returnAns);
//   // if(returnAns == MAC_TX_OK) {
//     packet_destroy(packet);
//   // }
// }

/*---------------------------------------------------------------------------*/
// static void
// send_beacon(void *ptr)
// {
  // static uint8_t selfAddrValue;
  // selfAddrValue = addresses_self_get_pointer(IOTUS_ADDRESSES_TYPE_ADDR_SHORT)[0];

  // clock_time_t backoff = CLOCK_SECOND*gMaintenancePeriod - backOffDifference;//ms
  // backOffDifference = (CLOCK_SECOND*((random_rand()%ND_BACKOFF_TIME)))/1000;

  // backoff += backOffDifference;
  // ctimer_set(&sendNDTimer, backoff, send_beacon, NULL);

  // printf("nd maint beacon\n");https://ez.analog.com/wireless-sensor-networks/ad6lowpan/w/documents/558/faq-node-joining-process-in-6lowpan---nd-rpl
  // iotus_packet_t *packet = iotus_initiate_packet(
  //                           12,
  //                           private_nd_control,
  //                           PACKET_PARAMETERS_WAIT_FOR_ACK,
  //                           IOTUS_PRIORITY_ROUTING,
  //                           5000,
  //                           NODES_BROADCAST,
  //                           send_cb);

  // if(NULL == packet) {
  //   SAFE_PRINTF_LOG_INFO("Packet failed");
  //   return;
  // }

  // packet_set_type(packet, IOTUS_PACKET_TYPE_IEEE802154_BEACON);
 
  // SAFE_PRINTF_LOG_INFO("Packet nd %u \n", packet->pktID);
  // active_data_link_protocol->send(packet);
// }

/*---------------------------------------------------------------------*/
uint8_t *
build_packet_type(uint8_t operation) {
  iotus_additional_info_t *h;
  uint8_t totalSize = 0;

  //First byte is the total size of this msg
  uint8_t *bufferPointer = gMsgPayload + 1;

  h = list_head(gNDPieces);
  while(NULL != h) {
    if(h->type == operation) {
      uint8_t *ptr = pieces_get_data_pointer(h);
      memcpy(bufferPointer, ptr, ptr[0]);
      bufferPointer += ptr[0];
      totalSize += ptr[0];
    }
    h = list_item_next(h);
  }

  //Fix the total size now...
  gMsgPayload[0] = totalSize;

  return gMsgPayload;
}

/*---------------------------------------------------------------------*/
/*
 * Default function required from IoTUS, to initialize, run and finish this service
 */
void iotus_signal_handler_neighbor_discovery(iotus_service_signal signal, void *data)
{
  if(IOTUS_START_SERVICE == signal) {
    SAFE_PRINT("\tService Neighbor D.\n");
    // sprintf((char *)private_nd_control, "### tira ###");
    list_init(gNDPieces);
  }
  // else if (IOTUS_RUN_SERVICE == signal){
    // if(gAmIRouter) {
    //   printf("root\n");
    //   backOffDifference = (CLOCK_SECOND*((random_rand()%ND_BACKOFF_TIME)))/1000;
    //   clock_time_t backoff = CLOCK_SECOND*gMaintenancePeriod + backOffDifference;//ms
    //   ctimer_set(&sendNDTimer, backoff, send_beacon, NULL);
    // } else {
    //   printf("not root\n");
    // }
  // } else if (IOTUS_END_SERVICE == signal){

  // }
}