
/**
 * \defgroup description...
 *
 * This...
 *
 * @{
 */

/*
 * nodes.c
 *
 *  Created on: Feb 27, 2018
 *      Author: vinicius
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "addresses.h"
#include "global-parameters.h"
#include "lib/memb.h"
#include "iotus-core.h"
#include "nodes.h"
#include "pieces.h"
#include "platform-conf.h"
#include "timestamp.h"


#define DEBUG IOTUS_DONT_PRINT//IOTUS_PRINT_IMMEDIATELY
#define THIS_LOG_FILE_NAME_DESCRITOR "nodes"
#include "safe-printer.h"

// Initiate the lists of module
MEMB(iotus_nodes_mem, iotus_node_t, IOTUS_NODES_LIST_SIZE);
LIST(gNodesList);

uint8_t nodes_broadcast_pointer;
uint8_t nodes_self_pointer;

/*---------------------------------------------------------------------*/
/*
 * \brief Get the node address by its type.
 * \param   node          The pointer of the node to be queried.
 * \param   addressType   The type of address to be returned.
 * \return  The pointer to the address requested.
 * \retval  NULL           If the address type is not found. 
 */
uint8_t *
nodes_get_address(iotus_address_type addressType, iotus_node_t *node) {
  if(node == NULL ||
    addressType == 0) {
    SAFE_PRINTF_LOG_ERROR("null node");
    return NULL;
  }
  if(node == NODES_BROADCAST) {
    return addresses_self_get_pointer(IOTUS_ADDRESSES_TYPE_ADDR_BROADCAST);
  }
  iotus_additional_info_t *addressInfo = pieces_get_additional_info(
                                              node->additionalInfoList,
                                              addressType);
  if(NULL != addressInfo) {
    //Found this address type...
    return pieces_get_data_pointer(addressInfo);
  }
  return NULL;
}


/*---------------------------------------------------------------------*/
/*
 * \brief Get the node by its address, given its address type.
 * \param   node          The pointer of the node to be queried.
 * \param   addressType   The type of address to be returned.
 * \retval  The pointer to the node, if the address is found. Null otherwise.
 */
iotus_node_t *
nodes_get_node_by_address(iotus_address_type addressType, uint8_t *address)
{
  if(NULL == address ||
     addressType == IOTUS_ADDRESSES_TYPE_ADDR_PANID ||
     addressType == 0) {
    return NULL;
  }

  //verify if this address is ours...
  uint8_t *selfAddress = addresses_self_get_pointer(addressType);
  if(selfAddress != NULL) {
    if(TRUE == addresses_compare(
                  selfAddress,
                  address,
                  ADDRESSES_GET_TYPE_SIZE(addressType))
                  ) {
      //It`s our
      return NODES_SELF;
    }
  }

  //Verify if this node already exists
  iotus_node_t *node;
  for(node = list_head(gNodesList); node != NULL; node = list_item_next(node)) {
    iotus_additional_info_t *addressInfo = pieces_get_additional_info(node->additionalInfoList, addressType);
    if(NULL != addressInfo) {
      //Found this address type...
      if(TRUE == addresses_compare(
                    pieces_get_data_pointer(addressInfo),
                    address,
                    ADDRESSES_GET_TYPE_SIZE(addressType))
                    ) {
        //Node found
        break;
      }
    }
  }
  return node;
}

/*---------------------------------------------------------------------*/
Boolean
nodes_set_address(iotus_node_t *node, iotus_address_type addressType, uint8_t *address)
{
  uint8_t *addrPointer = pieces_modify_additional_info_var(
                              node->additionalInfoList,
                              addressType,
                              ADDRESSES_GET_TYPE_SIZE(addressType),
                              TRUE);
  if(NULL == addrPointer) {
    SAFE_PRINTF_LOG_ERROR("Set address fail");
    return FALSE;
  }
  memcpy(addrPointer, address, ADDRESSES_GET_TYPE_SIZE(addressType));
  return TRUE;
}

/*---------------------------------------------------------------------*/
/**
 * \return
 */
static iotus_node_t *
create_node(void) {
  iotus_node_t *newChunk = (iotus_node_t *)memb_alloc(&iotus_nodes_mem);

  if(newChunk == NULL) {
    /* Failed to alloc memory */
    SAFE_PRINT("Allocate memory.");
    return NULL;
  }

  newChunk->params = 0;
  LIST_STRUCT_INIT(newChunk, additionalInfoList);

  list_push(gNodesList, newChunk);
  return newChunk;
}


/*---------------------------------------------------------------------*/
/*
 * \brief Get the elapsed time between the provided values and now.
 * \param time    The pointer of the timestamp to be set.
 */
iotus_node_t *
nodes_update_by_address(iotus_address_type addressType, uint8_t *address)
{
  //Verify if this node already exists
  iotus_node_t *h = nodes_get_node_by_address(addressType, address);
  if(NULL == h) {
    //node was not found
    h = create_node();

    //Only new nodes need to update the address
    nodes_set_address(h, addressType, address);
  }

  // h->timestamp = clock_time();

  return h;
}

/*---------------------------------------------------------------------*/
/*
 * \brief Destroy the node object.
 * \param node    The pointer of the node to be destroyed.
 */
void
nodes_destroy(iotus_node_t *node)
{
  pieces_clean_additional_info_list(node->additionalInfoList);
  list_remove(gNodesList,node);
  memb_free(&iotus_nodes_mem, node);
}

/*---------------------------------------------------------------------*/
/*
 * Default function required from IoTUS, to initialize, run and finish this service
 */
void iotus_signal_handler_nodes(iotus_service_signal signal, void *data)
{
  if(IOTUS_START_SERVICE == signal) {
    SAFE_PRINT("\tService Packet\n");

    // Initiate the lists of module
    list_init(gNodesList);

  }
  /* else if (IOTUS_RUN_SERVICE == signal){
  //TODO: Delete nodes not heard after some time
  //TODO: update sequence numbers
  } else if (IOTUS_END_SERVICE == signal){

  }*/
}