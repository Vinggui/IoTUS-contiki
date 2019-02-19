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

#define NUMARGS(...)  (sizeof((uint8_t[]){0, ##__VA_ARGS__})/sizeof(uint8_t)-1)

#define STATIC_TREE                   1//1 for true. 0 for false

#define STATIC_COORDINATORS           1,2//Use comma to add more routers
#define STATIC_ROOT_ADDRESS           {1,0}//two bytes address (short)

typedef enum {
  TREE_STATUS_DISCONNECTED,
  TREE_STATUS_BUILDING,
  TREE_STATUS_CONNECTED,
  TREE_STATUS_RECONNECTING
} tree_manager_conn_status;

extern uint8_t treeRouter;
extern uint8_t treeRouterNodes[];
extern uint8_t treePersonalRank;
extern iotus_node_t *rootNode;
extern iotus_node_t *fatherNode;

void
iotus_signal_handler_tree_manager(iotus_service_signal signal, void *data);

#endif /* IOTUS_ARCH_SERVICES_NULL_NULL_H_ */

/* The following stuff ends the \defgroup block at the beginning of
   the file: */

/** @} */
