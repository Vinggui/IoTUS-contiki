/**
 * \defgroup decription...
 *
 * This...
 *
 * @{
 */

/*
 * packet.c
 *
 *  Created on: Nov 14, 2017
 *      Author: vinicius
 */

#include "chores.h"
#include "iotus-core.h"
#include "platform-conf.h"

/* Buffer to indicate which functionalities are execute by each layer.
 * This variable is set by each protocol when starting
 */
#define DEFAULT_FUNCTIONALITIES_SIZE  (IOTUS_FINAL_NUMBER_CHORE*2/8 +1)
static uint8_t default_layers_chores_header[DEFAULT_FUNCTIONALITIES_SIZE] = {0};
#define DEFAULT_REQUEST_NEEDED_SIZE  (IOTUS_FINAL_NUMBER_CHORE/8 +1)
static uint8_t default_chores_requested[DEFAULT_REQUEST_NEEDED_SIZE] = {0};


/*---------------------------------------------------------------------*/
/**
 * \brief           Sets the default chore that will be present in every default packet.
 * @param priority  The iotus layer requesting: radio, data link, routing or transport.
 * @param chore      The chore the request is going to take effect upon.
 * \return          Status: Success or failure.
 */
Status
iotus_subscribe_for_chore(iotus_layer_priority priority,
                           iotus_chores chore)
{
  uint8_t posByte = chore/4;
  uint8_t posBit = (chore%4)*2;
  uint8_t layerPriorityForChore = default_layers_chores_header[posByte] & (11<<posBit);

  uint8_t posByteRequested = chore/8;
  uint8_t posBitRequested = chore%8;
  uint8_t choreRequested = default_chores_requested[posByteRequested] & (1<<posBitRequested);

  if(choreRequested > 0) {
    //There is some layer responsible for this chore...
    if(layerPriorityForChore <= (priority<<posBit)) {
      //This request has lower priority (higher value)
      return FAILURE;
    }
    default_layers_chores_header[posByte] &= ~(11<<posBit);
  }
  //This request has higher priority (lower value)
  //substitute this chore to this request
  default_layers_chores_header[posByte] |= (priority<<posBit);
  default_chores_requested[posByteRequested] |= (1<<posBitRequested);
  return SUCCESS;
}

/*---------------------------------------------------------------------*/
/*
 * \brief  Verifies if default chore header is assigned to some layer
 * \param chore       The chore to be verified.
 * \return            The layer assigned to the chore or -1 if no layer is doing it.
 */
int8_t
iotus_get_layer_assigned_for(iotus_chores chore)
{
  uint8_t posByteRequested = chore/8;
  uint8_t posBitRequested = chore%8;
  if(0 == (default_chores_requested[posByteRequested] & (1<<posBitRequested))) {
    return -1;
  }

  uint8_t posByte = chore/4;
  uint8_t posBit = (chore%4)*2;
  uint8_t layerForChore = (default_layers_chores_header[posByte]>>posBit) & 0b00000011;
  return layerForChore;
}

/* The following stuff ends the \defgroup block at the beginning of
   the file: */

/** @} */
