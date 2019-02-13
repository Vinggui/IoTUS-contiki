
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
#include "contiki.h"
#include "iotus-core.h"
#include "timestamp.h"
#include "tree_manager.h"


#define DEBUG IOTUS_DONT_PRINT//IOTUS_PRINT_IMMEDIATELY
#define THIS_LOG_FILE_NAME_DESCRITOR "tree_m"
#include "safe-printer.h"


uint8_t amIRouter = 0;
uint8_t routerNodes[] = STATIC_COORDINATORS;

/*---------------------------------------------------------------------*/
/*
 * Default function required from IoTUS, to initialize, run and finish this service
 */
void iotus_signal_handler_tree_manager(iotus_service_signal signal, void *data)
{
  if(IOTUS_START_SERVICE == signal) {
    SAFE_PRINT("\tService Tree\n");


  }
  /* else if (IOTUS_RUN_SERVICE == signal){
  } else if (IOTUS_END_SERVICE == signal){

  }*/
}