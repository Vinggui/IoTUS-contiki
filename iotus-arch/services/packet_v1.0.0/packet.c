
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "iotus-core.h"
#include "list.h"
#include "platform-conf.h"

#define DEBUG 1
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else /* DEBUG */
#define PRINTF(...)
#endif /* DEBUG */

#define COMMON_STRUCT_PIECES(structName) \
  structName *next;\
  uint8_t params;\
  uint16_t dataSize;\
  uint8_t *data

struct msg_piece {
  COMMON_STRUCT_PIECES(struct msg_piece);
  void *callbackHandler;
  uint16_t timeout;
  uint8_t destination[IOTUS_RADIO_FULL_ADDRESS];
};

struct header_piece {
  COMMON_STRUCT_PIECES(struct header_piece);
  uint8_t type;
};

/* This function is created separeted so that this module
 * memory allocation can be easily changed.
 * Also, the header_piece has the common initial as the msg, so
 * just use as the cast type to set common fields.
 */
static void*
malloc_piece(uint16_t dataSize, uint16_t pieceSize) {
  void *newPiecePointer = malloc(pieceSize);

  if(newPiecePointer == NULL) {
    /* Failed to alloc memory */
    PRINTF("Failed to allocate memory for piece.");
    return NULL;
  }

  struct header_piece *newPiece = (struct header_piece *)newPiecePointer;

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

/* In case memory allocation needs to be changed, this function
 * centralizes every operation to set a specific field */
static void
set_piece_parameters(void *piece, uint8_t params)
{
  struct header_piece* pieceStruct = (struct header_piece*)piece;
  pieceStruct->params = params;
}

static void
set_msg_piece_timeout(void *piece, uint16_t timeout)
{
  struct msg_piece* pieceStruct = (struct msg_piece*)piece;
  /* Encode the parameters */
  pieceStruct->timeout = timeout;
}

static void
set_msg_piece_destination(void *piecePointer, const uint8_t *dest)
{
  struct msg_piece* piece = (struct msg_piece*)piecePointer;
  uint8_t i=0;
  for(;i<IOTUS_RADIO_FULL_ADDRESS;i++) {
    piece->destination[i] = dest[i];
  }
}

static void
set_msg_piece_callback_function(void *piecePointer, void *function)
{
  struct msg_piece* piece = (struct msg_piece*)piecePointer;
  piece->callbackHandler = function;
}

static void
set_header_piece_type(void *piecePointer, uint8_t type)
{
  struct header_piece* piece = (struct header_piece*)piecePointer;
  piece->type = type;
}

static void
set_piece_data(void *piecePointer, const uint8_t *data)
{
  struct header_piece* piece = (struct header_piece*)piecePointer;
  uint8_t i=0;
  for(;i<piece->dataSize;i++) {
    piece->data[i] = data[i];
  }
}

void
packet_delete_piece(void *piecePointer) {
  struct header_piece *piece = (struct header_piece*)piecePointer;
  if(piece->dataSize > 0) {
    free(piece->data);
  }
  free(piecePointer);
}

void*
packet_create_msg_piece(uint16_t payloadSize, uint8_t allowAggregation,
    uint8_t allowFragmentation, uint16_t timeout, const uint8_t* payload,
    const uint8_t *destination, void *callbackFunction)
{
  void* newMsg = malloc_piece(payloadSize, sizeof(struct msg_piece));
  set_piece_data(newMsg, payload);

  uint8_t params;
  /* Encode parameters */
  if(allowAggregation)
    params  = 0b10000000;
  if(allowFragmentation)
    params |= 0b01000000;

  set_piece_parameters(newMsg, params);
  set_msg_piece_timeout(newMsg, timeout);
  set_msg_piece_destination(newMsg, destination);
  set_msg_piece_callback_function(newMsg, callbackFunction);
  return newMsg;
}

void*
packet_create_header_piece(uint16_t headerSize, uint8_t isSingleBit,
    uint8_t msg_piece_to_attach, const uint8_t* header,
    uint8_t type)
{
  void* newHeader = malloc_piece(headerSize, sizeof(struct header_piece));
  if(headerSize)
    set_piece_data(newHeader, header);

  uint8_t params;
  /* Encode parameters */
  if(isSingleBit)
    params  = 0b10000000;

  set_piece_parameters(newHeader, params);
  set_header_piece_type(newHeader, type);
  return newHeader;
}


static void
start(void)
{
  // Initiate the lists of module
  LIST(packetMsgList);
  list_init(packetMsgList);
  LIST(packetHeaderList);
  list_init(packetHeaderList);
}


static void
run(void)
{
}

static void
close(void)
{}

struct iotus_service_module_struct packet_service_module = {
  start,
  run,
  close
};

/* The following stuff ends the \defgroup block at the beginning of
   the file: */

/** @} */
