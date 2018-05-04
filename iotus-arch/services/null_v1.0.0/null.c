
/**
 * \defgroup description...
 *
 * This...
 *
 * @{
 */

/*
 * null.c
 *
 *  Created on: Feb 27, 2018
 *      Author: vinicius
 */

#include <stdio.h>
#include <stdlib.h>
#include "iotus-core.h"


#define DEBUG IOTUS_DONT_PRINT//IOTUS_PRINT_IMMEDIATELY
#define THIS_LOG_FILE_NAME_DESCRITOR "null"
#include "safe-printer.h"



/*---------------------------------------------------------------------*/
/*
 * Default function required from IoTUS, to initialize, run and finish this service
 */
void iotus_signal_handler_null(iotus_service_signal signal, void *data)
{
  if(IOTUS_START_SERVICE == signal) {
    SAFE_PRINT("\tService null\n");


  }
  /* else if (IOTUS_RUN_SERVICE == signal){
  } else if (IOTUS_END_SERVICE == signal){

  }*/
}