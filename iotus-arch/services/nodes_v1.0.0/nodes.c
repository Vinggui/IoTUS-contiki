
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
#include "platform-conf.h"

#define DEBUG 1
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else /* DEBUG */
#define PRINTF(...)
#endif /* DEBUG */

struct nodes {
  struct nodes *next;
  uint8_t *address;
  unsigned long timeout_seconds;
  clock_time_t timestamp;//Supposed to be the last time heard
  LIST_STRUCT(additionalInfo);
};

// Initiate the lists of module
LIST(gNodesList);


/***********************************************************************/

uint8_t
iotus_nodes_compare_address(uint8_t *add1, uint8_t *add2){

}

uint8_t *
iotus_nodes_get_address(void *node) {

}

void *
iotus_nodes_get_node_by_address(uint8_t addressSize, uint8_t *address) {

}

static void *
create_node(void) {
  struct nodes *newChunk = (struct nodes *)malloc(sizeof(struct nodes));

  if(newChunk == NULL) {
    /* Failed to alloc memory */
    PRINTF("Failed to allocate memory for node.");
    return NULL;
  }

  uint8_t *addressPointer = (uint8_t *)malloc(iotus_radio_address_size);
  if(addressPointer == NULL) {
    /* Failed to alloc memory */
    PRINTF("Failed to allocate memory for new node address.");
    free(newChunk);
    return NULL;
  }

  list_push(gNodesList, newChunk);
  return newChunk;
}


void *
iotus_nodes_update_by_address(uint8_t *address, uint16_t timestamp)
{
  //Verify if this node already exists
  struct nodes *h;
  for(h = list_head(gNodesList); h != NULL; h = list_item_next(h)) {
    if(iotus_nodes_get_node_by_address(address) == h) {
      //Node found
      break;
    }
  }
  if(NULL == h) {
    //node was not found
    h = (struct nodes *)create_node();

    //Only new nodes need to update the address
    int i;
    for(i=0; i<iotus_radio_address_size; i++) {
      h->address[i] = address[i];
    }
  }

  h->timestamp = 0; // TODO FIX THIS

  return h;
}


/*
 * Default function required from IoTUS, to initialize, run and finish this service
 */
void
iotus_signal_handler_global_parameters(iotus_service_signal signal, void *data)
{
  if(IOTUS_START_SERVICE == signal) {
    PRINTF("\tService Packet\n");

    // Initiate the lists of module
    list_init(gNodesList);

  }
  /* else if (IOTUS_RUN_SERVICE == signal){

  } else if (IOTUS_END_SERVICE == signal){

  }*/
}