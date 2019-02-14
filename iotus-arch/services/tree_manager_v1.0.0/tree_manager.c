
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


uint8_t gAmIRouter = 0;
uint8_t gRouterNodes[] = STATIC_COORDINATORS;
static struct ctimer sendNDTimer;
static clock_time_t backOffDifference;
iotus_node_t *rootNode;
iotus_node_t *fatherNode;
static uint8_t private_tree_control[12];

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
      if(addresses_self_get_pointer(IOTUS_ADDRESSES_TYPE_ADDR_SHORT)[0] == gRouterNodes[i]) {
        gAmIRouter = 1;
      }
    }

    #if STATIC_TREE == 1
    uint8_t addressRoot[2] = STATIC_ROOT_ADDRESS;
    rootNode = nodes_update_by_address(IOTUS_ADDRESSES_TYPE_ADDR_SHORT, addressRoot);
    #endif
  }
  /* else if (IOTUS_RUN_SERVICE == signal){
  } else if (IOTUS_END_SERVICE == signal){

  }*/
}