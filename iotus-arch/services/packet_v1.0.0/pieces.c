
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
#include "pieces.h"
#include "platform-conf.h"

#define DEBUG 1
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else /* DEBUG */
#define PRINTF(...)
#endif /* DEBUG */

struct generic_piece {
  COMMON_STRUCT_PIECES(struct generic_piece);
};

/* This function is created separeted so that this module
 * memory allocation can be easily changed.
 * Also, the generic_piece has the common initial as the msg, so
 * just use as the cast type to set common fields.
 */
void *
pieces_malloc(uint16_t dataSize, uint16_t pieceSize) {
  void *newPiecePointer = malloc(pieceSize);

  if(newPiecePointer == NULL) {
    /* Failed to alloc memory */
    PRINTF("Failed to allocate memory for piece.");
    return NULL;
  }

  struct generic_piece *newPiece = (struct generic_piece *)newPiecePointer;

  newPiece->dataSize = dataSize;
  if(dataSize > 0) {
    uint8_t *dataPointer = (uint8_t *)malloc(dataSize);
    if(dataPointer == NULL) {
      /* Failed to alloc memory */
      PRINTF("Failed to allocate memory for new msg payload.");
      free(newPiecePointer);
      return NULL;
    }
    newPiece->data = dataPointer;
  }
  return newPiecePointer;
}

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

uint8_t
pieces_set_data(void *piecePointer, const uint8_t *data)
{
  struct generic_piece *piece = (struct generic_piece *)piecePointer;
  if(piece == NULL) {
    return 0;
  }
  uint8_t i=0;
  for(;i<piece->dataSize;i++) {
    piece->data[i] = data[i];
  }
  return 1;
}

/* The following stuff ends the \defgroup block at the beginning of
   the file: */

/** @} */
