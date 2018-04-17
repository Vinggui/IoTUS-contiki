
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


#define DEBUG IOTUS_PRINT_IMMEDIATELY
#define THIS_LOG_FILE_NAME_DESCRITOR "Seqnum"
#include "safe-printer.h"

/*---------------------------------------------------------------------*/
uint8_t
seqnum_acquire(iotus_nodes_t *node)
{
  if(node == NULL) {
    return 0;
  }
  uint8_t *seq = pieces_modify_additional_info_var(
                                  node->additionalInfoList,
                                  IOTUS_NODES_ADD_INFO_TYPE_LAST_SEQ_NUM,
                                  1,
                                  TRUE);

  return ++(*seq);
}

/*---------------------------------------------------------------------*/
/*
 * Default function required from IoTUS, to initialize, run and finish this service
 */
void iotus_signal_handler_seqnum(iotus_service_signal signal, void *data)
{
  if(IOTUS_START_SERVICE == signal) {
    SAFE_PRINT("\tService sequence num\n");


  }
  /* else if (IOTUS_RUN_SERVICE == signal){
  } else if (IOTUS_END_SERVICE == signal){

  }*/
}