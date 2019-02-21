/**
 * \defgroup description...
 *
 * This...
 *
 * @{
 */

/*
 * neighbor_discovery.h
 *
 *  Created on: Nov 15, 2017
 *      Author: vinicius
 */

#ifndef IOTUS_ARCH_SERVICES_NEIGHBOR_DISCOVERY_H_
#define IOTUS_ARCH_SERVICES_NEIGHBOR_DISCOVERY_H_

#include "iotus-core.h"

#define ND_ASSOCIANTION_DELAY     2//sec

/* Similar to 802.15.4 MAC
* Beacons
* Association request packet
* Association get packet
* Association Answer packet
* Association confirm
*/
typedef enum {
  ND_PKT_BEACONS = 1,
  ND_PKT_ASSOCIANTION_REQ,
  ND_PKT_ASSOCIANTION_GET,
  ND_PKT_ASSOCIANTION_ANS,
  ND_PKT_ASSOCIANTION_CON,
  ND_PKT_KEEP_ALIVE_CON,

  //Do not erase this last option
  ND_PKT_MAX_VALUE
} nd_pkt_types;

typedef void (*nd_cb_func)(struct packet_piece *packet, uint8_t type, uint8_t size, uint8_t *data);


extern uint16_t ndKeepAlivePeriod;
extern uint16_t ndAssociation_answer_delay;

void
nd_remove_subscription(iotus_layer_priority layer);

void
nd_set_association_request_data(iotus_layer_priority layer, uint8_t size, uint8_t* payload);

void
nd_set_layer_cb(iotus_layer_priority layer, nd_cb_func cb);

void
nd_set_layer_operations(iotus_layer_priority layer, nd_pkt_types op);

uint8_t
nd_get_layer_operations(nd_pkt_types op);

void
iotus_signal_handler_neighbor_discovery(iotus_service_signal signal, void *data);

#endif /* IOTUS_ARCH_SERVICES_NEIGHBOR_DISCOVERY_H_ */

/* The following stuff ends the \defgroup block at the beginning of
   the file: */

/** @} */
