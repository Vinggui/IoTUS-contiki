
/**
 * \defgroup description...
 *
 * This...
 *
 * @{
 */

/*
 * tree_manager.c
 *
 *  Created on: Feb 13, 2019
 *      Author: vinicius
 */

#include <stdio.h>
#include <stdlib.h>
#include "addresses.h"
#include "contiki.h"
#include "iotus-api.h"
#include "iotus-core.h"
#include "iotus-netstack.h"
#include "layer-packet-manager.h"
#include "random.h"
#include "timestamp.h"
#include "tree_manager.h"


#define DEBUG IOTUS_DONT_PRINT//IOTUS_PRINT_IMMEDIATELY
#define THIS_LOG_FILE_NAME_DESCRITOR "tree_m"
#include "safe-printer.h"


#define NUMARGS(...)  (sizeof((uint8_t[]){0, ##__VA_ARGS__})/sizeof(uint8_t)-1)
#define STATIC_COORDINATORS_NUM       NUMARGS(STATIC_COORDINATORS)

uint8_t treeRouter = 0;
uint8_t treeRouterNodes[] = {STATIC_COORDINATORS};
uint8_t treePersonalRank = 0xFF;

iotus_node_t *rootNode;
iotus_node_t *fatherNode;


LIST(gTreePieces);

static uint8_t gTreeOperations[TREE_PKT_MAX_VALUE-1] = {0};
static tree_cb_func gLayersCB[IOTUS_MAX_LAYER_NUM-1] = {NULL};
static uint8_t gMsgPayload[30];

// static iotus_layer_priority gLayerControl;

/*---------------------------------------------------------------------------*/
static void
clean_pieces(tree_pkt_types type)
{
  iotus_additional_info_t *h = list_head(gTreePieces);
  while(NULL != h) {
    iotus_additional_info_t *next = list_item_next(h);

    /*
     * The h->type is the operation
     * The first byte is the size of this piece [0]
     * The second byte is the layer of this piece [1]
     */
    uint8_t *payload = pieces_get_data_pointer(h);
    if(h->type == type) {
      pieces_destroy_additional_info(gTreePieces,h);
    }
    h = next;
  }
}
// /*---------------------------------------------------------------------------*/
void
tree_remove_subscription(iotus_layer_priority layer)
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
void
tree_set_operation_msg(iotus_layer_priority layer, tree_pkt_types operation, uint8_t size, uint8_t* payload)
{
  if(size>0) {
    //The type indicates which operation is saved there
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

/*---------------------------------------------------------------------------*/
void
tree_set_layer_operations(iotus_layer_priority layer, tree_pkt_types op)
{
  gTreeOperations[op] |= (1<<layer);
}
/*---------------------------------------------------------------------------*/
void
tree_set_layer_cb(iotus_layer_priority layer, tree_cb_func cb)
{
  gLayersCB[layer] = cb;
}
/*---------------------------------------------------------------------------*/
uint8_t
tree_get_layer_operations(tree_pkt_types op)
{
  return gTreeOperations[op];
}

/*---------------------------------------------------------------------*/
uint8_t *
tree_build_packet_type(tree_pkt_types operation) {
  iotus_additional_info_t *h;
  uint8_t totalSize = 1;

  //First byte is the total size of this msg
  uint8_t *bufferPointer = gMsgPayload + 1;

  h = list_head(gTreePieces);
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
  if(operation != TREE_PKT_ASSOCIANTION_GET) {
    clean_pieces(operation);
  }
  return gMsgPayload;
}
/*---------------------------------------------------------------------*/
/*
 * Default function required from IoTUS, to initialize, run and finish this service
 */
void iotus_signal_handler_tree_manager(iotus_service_signal signal, void *data)
{
  if(IOTUS_START_SERVICE == signal) {
    SAFE_PRINT("\tService Tree\n");


    uint8_t i=0;
    for(; i<STATIC_COORDINATORS_NUM; i++) {
      if(addresses_self_get_pointer(IOTUS_ADDRESSES_TYPE_ADDR_SHORT)[0] == treeRouterNodes[i]) {
        treeRouter = 1;
        // printf("I'm router!\n");
      }
    }

    #if STATIC_TREE == 1
    uint8_t addressRoot[2] = STATIC_ROOT_ADDRESS;
    rootNode = nodes_update_by_address(IOTUS_ADDRESSES_TYPE_ADDR_SHORT, addressRoot);
    node_set_parameter(rootNode, IOTUS_NODES_PARAM_NETWORK_REACHABLE);
    #endif
  }
  //  else if (IOTUS_RUN_SERVICE == signal){
  //   gLayerControl = iotus_get_layer_assigned_for(IOTUS_CHORE_TREE_BUILDING);
  // }
  // else if (IOTUS_END_SERVICE == signal){

  // }
}