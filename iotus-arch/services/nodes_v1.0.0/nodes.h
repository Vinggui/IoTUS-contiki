/**
 * \defgroup decription...
 *
 * This...
 *
 * @{
 */

/*
 * NODES.h
 *
 *  Created on: Nov 15, 2017
 *      Author: vinicius
 */

#ifndef IOTUS_ARCH_SERVICES_NODES_NODES_H_
#define IOTUS_ARCH_SERVICES_NODES_NODES_H_

#include "iotus-core.h"
#include "list.h"
#include "platform-conf.h"
#include "timestamp.h"


typedef struct nodes {
  struct nodes *next;
  uint8_t *address; //This is the main address used by the network
  timestamp_t timestamp;//Supposed to be the last time heard
  LIST_STRUCT(additionalInfoList);
} iotus_node_t;

/*
 * The address type is defined as a single byte (0 to 255 values).
 * Hence, types 0 to N are the given default pre-defined addresses types for this
 * architecture.
 * 
 * After N, values are divided between ranges that are assigned to each layer of protocol,
 * in case they need to use some specific address not already expected by this architecture.
*/
enum nodes_additional_Info_types {
  IOTUS_NODES_ADD_INFO_TYPE_FULL_ADDR = 0,
  IOTUS_NODES_ADD_INFO_TYPE_SHORT_ADDR,
  IOTUS_NODES_ADD_INFO_TYPE_MINI_ADDR,
  IOTUS_NODES_ADD_INFO_TYPE_EXTENDED_ADDR,
  IOTUS_NODES_ADD_INFO_TYPE_TOPOL_TREE_RANK,


  IOTUS_NODES_ADD_INFO_TYPE___N
};

/* Application layer has twice the range, so that it can include sub-layers of protocols */
#define IOTUS_NODES_ADD_INFO_TYPE_RANGE_PER_LAYER     ((256-IOTUS_NODES_ADD_INFO_TYPE___N)/5)
#define IOTUS_NODES_DATA_LINK_ADD_INFO_TYPE_BEGIN     (IOTUS_NODES_ADD_INFO_TYPE___N)
#define IOTUS_NODES_ROUTING_ADD_INFO_TYPE_BEGIN       (IOTUS_NODES_ADD_INFO_TYPE_RANGE_PER_LAYER+IOTUS_NODES_DATA_LINK_ADD_INFO_TYPE_BEGIN)
#define IOTUS_NODES_TRANSPORT_ADD_INFO_TYPE_BEGIN     (IOTUS_NODES_ADD_INFO_TYPE_RANGE_PER_LAYER+IOTUS_NODES_ROUTING_ADD_INFO_TYPE_BEGIN)
#define IOTUS_NODES_APPLICATION_ADD_INFO_TYPE_BEGIN   (IOTUS_NODES_ADD_INFO_TYPE_RANGE_PER_LAYER+IOTUS_NODES_TRANSPORT_ADD_INFO_TYPE_BEGIN)



extern uint8_t nodes_broadcast_pointer;
#define NODES_BROADCAST   ((iotus_node_t *)&nodes_broadcast_pointer)

/*--------------------------------------------------------------------------*/
uint8_t
nodes_compare_address(uint8_t *addr1, uint8_t *addr2, uint8_t addressesSize);

uint8_t *
nodes_get_address(iotus_node_t *node, uint8_t addressType);

iotus_node_t *
nodes_get_node_by_address(uint8_t addressType, uint8_t *address);

iotus_node_t *
nodes_update_by_address(uint8_t *address, uint8_t addressType);

void
iotus_signal_handler_nodes(iotus_service_signal signal, void *data);

#endif /* IOTUS_ARCH_SERVICES_NODES_NODES_H_ */

/* The following stuff ends the \defgroup block at the beginning of
   the file: */

/** @} */
