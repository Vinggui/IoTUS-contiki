/**
 * \defgroup description...
 *
 * This...
 *
 * @{
 */

/*
 * layer-packet-manager.h
 *
 *  Created on: Nov 15, 2017
 *      Author: vinicius
 */

#ifndef IOTUS_ARCH_SHARED_UTILS_LAYER_PACKET_MANAGER_H_
#define IOTUS_ARCH_SHARED_UTILS_LAYER_PACKET_MANAGER_H_

#include "iotus-core.h"
#include "packet.h"

iotus_packet_t * iotus_initiate_packet(uint16_t payloadSize, const uint8_t* payload, uint8_t params,
    iotus_layer_priority priority, uint16_t timeout, iotus_node_t *finalDestination,
    packet_sent_cb func_cb);

void
iotus_retransmit_msg(iotus_packet_t *packet);

void
return_packet_to_application(iotus_packet_t *packetSelected);

#endif /* IOTUS_ARCH_SHARED_UTILS_LAYER_PACKET_MANAGER_H_*/

/* The following stuff ends the \defgroup block at the beginning of
   the file: */

/** @} */
