
/**
 * \defgroup description...
 *
 * This...
 *
 * @{
 */

/*
 * seqnum.c
 *
 *  Created on: Feb 27, 2018
 *      Author: vinicius
 */

#include <stdio.h>
#include <stdlib.h>
#include "iotus-core.h"


#define DEBUG IOTUS_DONT_PRINT
#define THIS_LOG_FILE_NAME_DESCRITOR "PhaseR"
#include "safe-printer.h"

/*---------------------------------------------------------------------*/
/**
 * Registers the sequence number of a received packet into the associated node
 * @param node The source node of the packet received.
 */
Status
seqnum_register(iotus_node_t *node, uint16_t sequence_number)
{
}

/*---------------------------------------------------------------------*/
/*
 * Default function required from IoTUS, to initialize, run and finish this service
 */
void iotus_signal_handler_seqnum(iotus_service_signal signal, void *data)
{
  if(IOTUS_START_SERVICE == signal) {
    SAFE_PRINT("\tService Phase recorder\n");
  }
  /* else if (IOTUS_RUN_SERVICE == signal){
  } else if (IOTUS_END_SERVICE == signal){

  }*/
}