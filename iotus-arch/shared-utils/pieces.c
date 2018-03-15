
/**
 * \defgroup decription...
 *
 * This...
 *
 * @{
 */

/*
 * pieces.c
 *
 *  Created on: Nov 14, 2017
 *      Author: vinicius
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "clock.h"
#include "list.h"
#include "pieces.h"
#include "platform-conf.h"

#define DEBUG 1
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else /* DEBUG */
#define PRINTF(...)
#endif /* DEBUG */


/*---------------------------------------------------------------------*/
/*
 * 
 * \param 
 * \return Packet final size
 */
/* This function is created separated so that this module
 * memory allocation can be easily changed.
 * Also, the generic_piece has the common initial as the msg, so
 * just use as the cast type to set common fields.
 */
iotus_generic_piece_t *
pieces_malloc(uint16_t dataSize, uint16_t pieceSize) {
  iotus_generic_piece_t *newPiece = (iotus_generic_piece_t *)malloc(pieceSize);

  if(newPiece == NULL) {
    /* Failed to alloc memory */
    PRINTF("Failed to allocate memory for piece.");
    return NULL;
  }

  newPiece->dataSize = dataSize;
  if(dataSize > 0) {
    uint8_t *dataPointer = (uint8_t *)malloc(dataSize);
    if(dataPointer == NULL) {
      /* Failed to alloc memory */
      PRINTF("Failed to allocate memory for new msg payload.");
      free(newPiece);
      return NULL;
    }
    newPiece->data = dataPointer;
  }

  newPiece->params = 0;
  newPiece->priority = 0;
  newPiece->callbackHandler = NULL;
  newPiece->finalDestinationNode = NULL;
  return newPiece;
}

/*---------------------------------------------------------------------*/
/*
 * 
 * \param 
 * \return Packet final size
 */
/*
static void
set_msg_piece_nextDestination(void *piecePointer, const uint8_t *dest)
{
  struct msg_piece* piece = (struct msg_piece*)piecePointer;
  uint8_t i=0;
  for(;i<IOTUS_RADIO_FULL_ADDRESS;i++) {
   
    piece->nextDestination[i] = dest[i];
  }
}*/

/*---------------------------------------------------------------------*/
/*
 * 
 * \param 
 * \return True if operation succeeded to copy, False otherwise.
 */
Boolean
pieces_set_data(void *piecePointer, const uint8_t *data)
{
  iotus_generic_piece_t *piece = (iotus_generic_piece_t *)piecePointer;
  if(piece == NULL) {
    return FALSE;
  }
  uint8_t i=0;
  for(;i<piece->dataSize;i++) {
    piece->data[i] = data[i];
  }
  return TRUE;
}

/*---------------------------------------------------------------------*/
/*
 * \brief Get the the additional info from a list.
 * \param   list          The pointer of the list to be verified.
 * \param   type          The type of information to be returned.
 * \return                The pointer to the type requested.
 * \retval  NULL           If the type was not found. 
 */
iotus_additional_info_t *
pieces_get_additional_info_by_type(list_t list, uint8_t type)
{
  if(NULL == list) {
    return NULL;
  }
  iotus_additional_info_t *addInfo;
  for(addInfo=list_head(list); addInfo != NULL; addInfo=list_item_next(addInfo)) {
    if(type == addInfo->type) {
      return addInfo;
    }
  }
  return NULL;
}

/*---------------------------------------------------------------------*/
/**
 * \brief 
 * \param   list    [description]
 * \param   item    [description]
 * \return          [description]
 */
void
pieces_insert_timeout_priority(list_t list, void *item)
{
  /* Link the item into the list, sorting insertion. */
  iotus_generic_piece_t *piece = (iotus_generic_piece_t *)list_head(list);
  if(NULL == piece) {
    //This list is empty
    list_push(list, item);
    return;
  }
  iotus_generic_piece_t *prevPiece = NULL;
  for(; piece!=NULL; piece=list_item_next(piece)) {
    if(timestamp_remainder(&(piece->timeout)) >= timestamp_remainder(&(((iotus_generic_piece_t *)item)->timeout))) {
      //insert it here
      list_insert(list, prevPiece, item);
      return;
      break;
    }
    prevPiece = piece;
  }
  return;
}


/* The following stuff ends the \defgroup block at the beginning of
   the file: */

/** @} */
