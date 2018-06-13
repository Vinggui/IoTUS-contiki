
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
#include "lib/mmem.h"
#include "timestamp.h"


#define DEBUG IOTUS_DONT_PRINT//IOTUS_PRINT_IMMEDIATELY
#define THIS_LOG_FILE_NAME_DESCRITOR "NDisc"
#include "safe-printer.h"



#if IOTUS_USING_MALLOC == 0
static struct mmem gPayload
#endif /* IOTUS_USING_MALLOC == 0 */

/*---------------------------------------------------------------------*/
/*
 * Default function required from IoTUS, to initialize, run and finish this service
 */
void iotus_signal_handler_neighbor_discovery(iotus_service_signal signal, void *data)
{
  if(IOTUS_START_SERVICE == signal) {
    SAFE_PRINT("\tService Neighbor D.\n");


  }
  /* else if (IOTUS_RUN_SERVICE == signal){
  } else if (IOTUS_END_SERVICE == signal){

  }*/
}