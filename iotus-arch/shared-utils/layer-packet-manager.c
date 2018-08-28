/**
 * \defgroup decription...
 *
 * This...
 *
 * @{
 */

/*
 * layer-packet-manager.c
 *
 *  Created on: Aug 27, 2018
 *      Author: vinicius
 */

#include "iotus-core.h"
#include "iotus-netstack.h"
#include "layer-packet-manager.h"
#include "platform-conf.h"

#define DEBUG IOTUS_DONT_PRINT//IOTUS_PRINT_IMMEDIATELY
#define THIS_LOG_FILE_NAME_DESCRITOR "pktMng"
#include "safe-printer.h"


static packet_sent_cb gApplicationConfirmationCB;
static packet_handler gApplicationPacketHandler;

/*---------------------------------------------------------------------*/
/*
 * \brief Define the functions to handle packet flow with application
 * \param packet_
 * \param param Parameter to be verified
 * \return Boolean.
 */
void
iotus_set_interface_functions(packet_sent_cb confirmationFunc, packet_handler appHandler) {
  gApplicationConfirmationCB = confirmationFunc;
  gApplicationPacketHandler = appHandler;
}

/*---------------------------------------------------------------------*/
static inline iotus_netstack_return
packet_send(iotus_packet_t *packetSelected)
{
  /*
   * Call the building packet of each layer.
   * 
   * This sequence of If and Else has to follow the exactly priority per layer,
   * that is, when the routing layer creates a frame (with routing priority),
   * only the lower layers have to process it's frame. If Data link create a frame,
   * it can send tha packet right away.
  */
  if(active_transport_protocol->build_to_send != NULL) {
    return active_transport_protocol->build_to_send(packetSelected);
  } else if(active_network_protocol->build_to_send != NULL) {
    return active_network_protocol->build_to_send(packetSelected);
  } else {
    return active_data_link_protocol->send(packetSelected);
  }
}

/*---------------------------------------------------------------------------*/
iotus_packet_t *
iotus_initiate_msg(uint16_t payloadSize, const uint8_t* payload, uint8_t params,
    iotus_layer_priority priority, uint16_t timeout, iotus_node_t *finalDestination)
{
  iotus_packet_t *packet;

  if(payloadSize == 0 || payload == NULL ||
    finalDestination == NULL) {
    SAFE_PRINTF_LOG_ERROR("Packet input");
    return NULL;
  }

  packet = packet_create_msg(
                payloadSize,
                payload,
                priority,
                timeout,
                TRUE,
                finalDestination);

  if(NULL == packet) {
    return NULL;
  }

  packet_set_parameter(packet,params);
  packet_set_confirmation_cb(packet, gApplicationConfirmationCB);
  SAFE_PRINTF_LOG_INFO("Packet created %u", packet->pktID);

  iotus_netstack_return status = packet_send(packet);
  if (MAC_TX_DEFERRED == status) {
    return packet;
  } else {
    gApplicationConfirmationCB(packet, status);
  }
  return packet;
}


/*---------------------------------------------------------------------*/
// static void
// return_packet_on_layers(iotus_packet_t *packetSelected, iotus_netstack_return returnAns)
// {
//   //Call the return functions of each layer
//   if(packetSelected->priority >= IOTUS_PRIORITY_ROUTING &&
//      returnAns < ROUTING_TX_OK) {
//     if(active_routing_protocol->sent_cb != NULL) {
//       active_routing_protocol->sent_cb(packetSelected, returnAns);
//     }
//   }
//   if(packetSelected->priority >= IOTUS_PRIORITY_TRANSPORT &&
//      returnAns < TRANSPORT_TX_OK) {
//     if(active_transport_protocol->sent_cb != NULL) {
//       active_transport_protocol->sent_cb(packetSelected, returnAns);
//     }
//   }
//   if(packetSelected->priority >= IOTUS_PRIORITY_APPLICATION) {
//     if(gApplicationConfirmationCB != NULL) {
//       gApplicationConfirmationCB(packetSelected, returnAns);
//     }
//   }
// }
/* The following stuff ends the \defgroup block at the beginning of
   the file: */

/** @} */
