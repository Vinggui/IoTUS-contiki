
/**
 * \defgroup decription...
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
#include "global-parameters.h"
#include "iotus-core.h"
#include "nodes.h"
#include "pieces.h"
#include "platform-conf.h"
#include "timestamp.h"


#define DEBUG IOTUS_PRINT_IMMEDIATELY
#define THIS_LOG_FILE_NAME_DESCRITOR "nodes"
#include "safe-printer.h"

// Initiate the lists of module
LIST(gNodesList);

uint8_t nodes_broadcast_pointer;

/*---------------------------------------------------------------------*/
/*
 * \brief Compare two addresses, considering the type requested.
 * \param addr1    The pointer of the first address.
 * \param addr2    The pointer of the second address.
 * \return         True if they match.
 */
uint8_t
nodes_compare_address(uint8_t *addr1, uint8_t *addr2, uint8_t addressesSize)
{
  uint8_t i;
  for(i=0; i<addressesSize; i++) {
    if(addr1 != addr2) {
      return TRUE;
    }
  }
  return FALSE;
}

/*---------------------------------------------------------------------*/
/*
 * \brief Get the node address by its type.
 * \param   node          The pointer of the node to be queried.
 * \param   addressType   The type of address to be returned.
 * \return  The pointer to the address requested.
 * \retval  NULL           If the address type is not found. 
 */
uint8_t *
nodes_get_address(iotus_node_t *node, uint8_t addressType) {
  if(addressType == IOTUS_NODES_ADD_INFO_TYPE_FULL_ADDR) {
    return node->address;
  } else {
    iotus_additional_info_t *addressInfo = pieces_get_additional_info_by_type(node->additionalInfoList, addressType);
    if(NULL != addressInfo) {
      //Found this address type...
      return addressInfo->data;
    }
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
nodes_get_node_by_address(uint8_t addressType, uint8_t *address) {
  if(NULL == address) {
    return NULL;
  }

  //Verify if this node already exists
  iotus_node_t *node;
  for(node = list_head(gNodesList); node != NULL; node = list_item_next(node)) {
    if(addressType == IOTUS_NODES_ADD_INFO_TYPE_FULL_ADDR) {
      if(TRUE == nodes_compare_address(node->address, address, iotus_radio_address_size)) {
        //Node found
        break;
      }
    } else {
      iotus_additional_info_t *addressInfo = pieces_get_additional_info_by_type(node->additionalInfoList, addressType);
      if(NULL != addressInfo) {
        //Found this address type...
        if(TRUE == nodes_compare_address(addressInfo->data, address, addressInfo->dataSize)) {
          //Node found
          break;
        }
      }
    }
  }
  return node;
}

/*---------------------------------------------------------------------*/
/**
 * \return
 */
static iotus_node_t *
create_node(void) {
  struct nodes *newChunk = (struct nodes *)malloc(sizeof(struct nodes));

  if(newChunk == NULL) {
    /* Failed to alloc memory */
    SAFE_PRINT("Allocate memory.");
    return NULL;
  }

  uint8_t *addressPointer = (uint8_t *)malloc(iotus_radio_address_size);
  if(addressPointer == NULL) {
    /* Failed to alloc memory */
    SAFE_PRINT("Allocate memory");
    free(newChunk);
    return NULL;
  }

  list_push(gNodesList, newChunk);
  return newChunk;
}


/*---------------------------------------------------------------------*/
/*
 * \brief Get the elapsed time between the provided values and now.
 * \param time    The pointer of the timestamp to be set.
 */
iotus_node_t *
nodes_update_by_address(uint8_t *address, uint8_t addressType)
{
  //Verify if this node already exists
  iotus_node_t *h = nodes_update_by_address(address, addressType);
  if(NULL == h) {
    //node was not found
    h = create_node();

    //Only new nodes need to update the address
    int i;
    for(i=0; i<iotus_radio_address_size; i++) {
      h->address[i] = address[i];
    }
  }

  timestamp_mark(&(h->timestamp),0);

  return h;
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

  } else if (IOTUS_END_SERVICE == signal){

  }*/
}