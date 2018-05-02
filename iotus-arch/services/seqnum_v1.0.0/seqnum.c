
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
#include "nodes.h"
#include "pieces.h"


#define DEBUG IOTUS_PRINT_IMMEDIATELY//IOTUS_DONT_PRINT
#define THIS_LOG_FILE_NAME_DESCRITOR "Seqnum"
#include "safe-printer.h"

/*---------------------------------------------------------------------*/
/**
 * Registers the sequence number of a received packet into the associated node
 * @param node The source node of the packet received.
 */
Status
seqnum_register(iotus_node_t *node, uint16_t sequence_number)
{
  if(NULL == node) {
    return FAILURE;
  }
  uint16_t *sqnPointer = pieces_modify_additional_info_var(
                              node->additionalInfoList,
                              IOTUS_NODES_ADD_INFO_TYPE_LAST_SEQ_NUM,
                              2,
                              TRUE);
  if(NULL == sqnPointer) {
    SAFE_PRINTF_LOG_ERROR("Set sqn fail");
    return FAILURE;
  }
  *sqnPointer = sequence_number;
  return SUCCESS;
}

/*---------------------------------------------------------------------*/
/**
 * Returns the last sequence number received by a node.
 * @param node The source node of the packet received.
 */
uint16_t
seqnum_get_last(iotus_node_t *node)
{
  if(NULL == node) {
    return 0;
  }

#if SEQNUM_MAX_AGE > 0
  //Check the timestamp first
  if(SEQNUM_MAX_AGE < timestamp_elapsed(node->timestamp)) {
    return 0;
  }
#endif

  uint16_t *sqnPointer = pieces_get_additional_info_var(
                              node->additionalInfoList,
                              IOTUS_NODES_ADD_INFO_TYPE_LAST_SEQ_NUM);
  if(NULL == sqnPointer) {
    return 0;
  }
  return *sqnPointer;
}

/*---------------------------------------------------------------------*/
uint8_t
seqnum_acquire(iotus_node_t *node)
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
    SAFE_PRINT("\tService seqnum\n");
  }
  /* else if (IOTUS_RUN_SERVICE == signal){
  } else if (IOTUS_END_SERVICE == signal){

  }*/
}