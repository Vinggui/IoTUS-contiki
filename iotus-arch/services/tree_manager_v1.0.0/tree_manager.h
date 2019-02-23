/**
 * \defgroup description...
 *
 * This...
 *
 * @{
 */

/*
 * tree_manager.h
 *
 *  Created on: Fev 13, 2019
 *      Author: vinicius
 */

#ifndef IOTUS_ARCH_SERVICES_NULL_NULL_H_
#define IOTUS_ARCH_SERVICES_NULL_NULL_H_

#include "iotus-core.h"

#define STATIC_TREE                   1//1 for true. 0 for false


/* Similar to 802.15.4 MAC
* Beacons
* Association request packet
* Association get packet
* Association Answer packet
* Association confirm
*/
typedef enum {
  TREE_PKT_ASSOCIANTION_GET = 1,  //Data get ("DIS")
  TREE_PKT_ASSOCIANTION_ANS,  //Data info ("DIO")
  TREE_PKT_ADVERTISEMENT,     //Data Advertisement
  TREE_PKT_ADVERTISEMENT_ACK, //Data Advertisement Ack ("DAO-ACK")

  //Do not erase this last option
  TREE_PKT_MAX_VALUE
} tree_pkt_types;

// #define STATIC_COORDINATORS           1,2,3//Use comma to add more routers
// #define STATIC_ROOT_ADDRESS           {1,0}//two bytes address (short)

typedef enum {
  TREE_STATUS_DISCONNECTED,
  TREE_STATUS_SCANNING,
  TREE_STATUS_BUILDING,
  TREE_STATUS_CONNECTED,
  TREE_STATUS_RECONNECTING
} tree_manager_conn_status;


typedef void (*tree_cb_func)(struct packet_piece *packet, uint8_t type, uint8_t size, uint8_t *data);

extern uint8_t treeRouter;
extern uint8_t treeRouterNodes[];
extern uint8_t treePersonalRank;
extern iotus_node_t *rootNode;
extern iotus_node_t *fatherNode;
extern tree_manager_conn_status tree_connection_status;

void
tree_set_layer_operations(iotus_layer_priority layer, tree_pkt_types op);

void
tree_set_layer_cb(iotus_layer_priority layer, tree_cb_func cb);

uint8_t
tree_get_layer_operations(tree_pkt_types op);

uint8_t *
tree_build_packet_type(tree_pkt_types operation);

void
iotus_signal_handler_tree_manager(iotus_service_signal signal, void *data);

#endif /* IOTUS_ARCH_SERVICES_NULL_NULL_H_ */

/* The following stuff ends the \defgroup block at the beginning of
   the file: */

/** @} */
