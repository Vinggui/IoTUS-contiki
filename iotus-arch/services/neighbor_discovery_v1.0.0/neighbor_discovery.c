
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


uint16_t ndKeepAlivePeriod;
uint16_t ndAssociation_answer_delay;

// static uint8_t gLayersDoingND = 0;
static uint8_t gNDOperations[ND_PKT_MAX_VALUE-1] = {0};

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
// void
// nd_assign_layer(iotus_layer_priority layer)
// {
//   gLayersDoingND |= (1<<layer);
// }
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
}/*---------------------------------------------------------------------------*/
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

  // printf("nd maint beacon\n");
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
/*
 * Default function required from IoTUS, to initialize, run and finish this service
 */
void iotus_signal_handler_neighbor_discovery(iotus_service_signal signal, void *data)
{
  if(IOTUS_START_SERVICE == signal) {
    SAFE_PRINT("\tService Neighbor D.\n");
    // sprintf((char *)private_nd_control, "### tira ###");

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