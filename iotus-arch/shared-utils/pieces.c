
/**
 * \defgroup description...
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
#include "iotus-core.h"
#include "lib/memb.h"
#include "lib/mmem.h"
#include "list.h"
#include "pieces.h"
#include "platform-conf.h"


#define DEBUG IOTUS_PRINT_IMMEDIATELY
#define THIS_LOG_FILE_NAME_DESCRITOR "pieces"
#include "safe-printer.h"

#if IOTUS_USING_MALLOC == 0
MEMB(iotus_additional_info_handlers_mem, iotus_additional_info_t, IOTUS_ADDITIONAL_HANDLERS_SIZE);
#endif

/*---------------------------------------------------------------------*/
/* \brief     This function is created separated so that this module
 *            memory allocation can be easily changed.
 *            Also, the generic_piece has the common initial as the msg, so
 *            just use as the cast type to set common fields.
 * 
 * @param m         When using mmem, the buffer to request allocation.
 * @param allocSize When using malloc, the size to be allocated.
 * @param data      The data field to be used.
 * @param dataSize  The size of this data field.
 * \return          The pointer to this piece.
 */
iotus_generic_piece_t *
pieces_malloc(struct memb *m, uint16_t allocSize,
         const uint8_t *data, uint16_t dataSize) {
  #if IOTUS_USING_MALLOC == 0
  iotus_generic_piece_t *newPiece = (iotus_generic_piece_t *)memb_alloc(m);
  allocSize = allocSize;
  #else
  m=m;
  iotus_generic_piece_t *newPiece = (iotus_generic_piece_t *)malloc(allocSize);
  #endif

  if(newPiece == NULL) {
    /* Failed to alloc memory */
    SAFE_PRINTF_LOG_ERROR("Allocate memory");
    return NULL;
  }


#if IOTUS_USING_MALLOC == 0
  if(mmem_alloc(&(newPiece->data), dataSize) == 0) {
    SAFE_PRINTF_LOG_ERROR("Alloc mmem");
    memb_free(m, newPiece);
    return NULL;
  }
#else /* IOTUS_USING_MALLOC == 0 */
  newPiece->data.ptr = malloc(dataSize);
  if(newPiece->data.ptr == NULL) {
    SAFE_PRINTF_LOG_ERROR("Alloc malloc");
    free(newPiece);
    return NULL;
  }
  newPiece->data.size = dataSize;
#endif /* IOTUS_USING_MALLOC == 0 */
  if(NULL != data) {
    memcpy(newPiece->data.ptr, data, dataSize);
  }
  
  newPiece->params = 0;
  newPiece->priority = 0;
  newPiece->callbackHandler = NULL;
  newPiece->finalDestinationNode = NULL;
  return newPiece;
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
pieces_get_additional_info(list_t list, uint8_t type)
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
 * \brief     Destroy the piece itself.
 * \param m   The pointer to the memb.
 * \param h   The pointer to this piece.
 * \return    True for confirmed operation.
 */
Boolean
pieces_destroy(struct memb *m, void *h)
{
  if(h == NULL) {
    SAFE_PRINTF_LOG_ERROR("Pointer null");
    return FALSE;
  }

  iotus_generic_piece_t *piece = (iotus_generic_piece_t *)h;
  

  #if IOTUS_USING_MALLOC == 0
  mmem_free(&(piece->data));
  memb_free(m,h);
  #else
  m=m;
  free(piece->data.ptr);
  free(h);
  #endif
  
  return TRUE;
}

/*---------------------------------------------------------------------*/
/**
 * \brief     Destroy the list of additional information that a piece may contain.
 * \param h   The pointer to this list.
 */
void
pieces_destroy_additional_info(list_t list, iotus_additional_info_t *item)
{
  if(list == NULL || item == NULL) {
    SAFE_PRINTF_LOG_ERROR("Add info Destroy");
    return;
  }
  list_remove(list,item);
  if(item->isBuffered == TRUE) {
    #if IOTUS_USING_MALLOC == 0
    mmem_free(&(item->data));
    #else
    free(item->data.ptr);
    #endif
    
  }

  #if IOTUS_USING_MALLOC == 0
  memb_free(&iotus_additional_info_handlers_mem, item);
  #else
  free(item);
  #endif
}

/*---------------------------------------------------------------------*/
/**
 * \brief     Destroy the list of additional information that a piece may contain.
 * \param h   The pointer to this list.
 */
void
pieces_clean_additional_info_list(list_t list)
{
  iotus_additional_info_t *h;
  while(NULL != (h =list_pop(list))) {
    pieces_destroy_additional_info(list,h);
  }
}


/*---------------------------------------------------------------------*/
/**
 * \brief                Allocate, set and link the additional information required.
 *                       Hence, the information has to be given by its pointer and size only.
 * \param list           The list where this info will be linked.
 * \param varSize       The size of this value
 * \param createBuffer   If the value needs to be created of not.
 * \return               The pointer to this additional information var.
 */
void *
pieces_modify_additional_info_var(list_t list, uint8_t type,
                                  uint16_t varSize,
                                  Boolean createBuffer)
{
  if(NULL == list) {
    return NULL;
  }

  //Verify if this list already has this info...
  iotus_additional_info_t *addInfo = pieces_get_additional_info(list,type);
  if(NULL != addInfo) {
    return pieces_get_data_pointer(addInfo);
  } 

  //create it...
  #if IOTUS_USING_MALLOC == 0
  addInfo = memb_alloc(&iotus_additional_info_handlers_mem);
  if(NULL == addInfo) {
    SAFE_PRINTF_LOG_ERROR("Alloc fail");
    return NULL;
  }
  #else
  addInfo = malloc(sizeof(iotus_additional_info_t));
  if(NULL == addInfo) {
    SAFE_PRINTF_LOG_ERROR("Alloc fail");
    return NULL;
  }
  #endif
  

  if(TRUE == createBuffer) {
    addInfo->isBuffered = TRUE;

    #if IOTUS_USING_MALLOC == 0
    if(mmem_alloc(&(addInfo->data), varSize) == 0) {
      SAFE_PRINTF_LOG_ERROR("Alloc mmem");
      memb_free(&iotus_additional_info_handlers_mem, addInfo);
      return NULL;
    }
    //memcpy(MMEM_PTR(&(addInfo->data)), var, varSize);
    #else
    void *varPointer = malloc(varSize);
    if(varPointer == NULL) {
      SAFE_PRINTF_LOG_ERROR("Alloc malloc");
      free(addInfo);
      return NULL;
    }
    //memcpy(varPointer, var, varSize);
    addInfo->data.ptr = varPointer;
    #endif
  } else {
    addInfo->isBuffered = FALSE;
    addInfo->data.size = varSize;
    addInfo->data.ptr = NULL;
  }

  addInfo->type = type;
  list_push(list, addInfo);
  return pieces_get_data_pointer(addInfo);
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
