
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
#include "global_parameters.h"
#include "iotus-core.h"
#include "local-packet-def.h"
#include "list.h"
#include "packet.h"
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
  uint16_t timeout;\
  uint8_t priority;\
  void *callbackHandler;\
  uint16_t dataSize;\
  uint8_t *data

struct msg_piece {
  COMMON_STRUCT_PIECES(struct msg_piece);
  uint16_t totalPacketSize;//Will be used to build the final packet, processed by the core
  uint16_t initialBitHeaderSize;
  uint8_t *initialBitHeader; //Will be used to build the final packet, processed by the core
  uint16_t finalBytesHeaderSize;
  uint8_t *finalBytesHeader; //Will be used to build the final packet, processed by the core
  LIST_STRUCT(infoPieces);
};

struct piggyback_piece {
  COMMON_STRUCT_PIECES(struct piggyback_piece);
  uint8_t type;
};

/* Buffer to indicate which functionalities are execute by each layer.
 * This variable is set by each protocol when starting
 */
#define DEFAULT_FUNCTIONALITIES_SIZE  (IOTUS_FINAL_NUMBER_DEFAULT_HEADER_CHORE*2/8 +1)
static uint8_t default_layers_chores_header[DEFAULT_FUNCTIONALITIES_SIZE] = {0};


// Initiate the lists of module
LIST(gPacketMsgList);
LIST(gPacketHeaderList);

/***********************************************************************/
/* This function is created separeted so that this module
 * memory allocation can be easily changed.
 * Also, the piggyback_piece has the common initial as the msg, so
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

  struct piggyback_piece *newPiece = (struct piggyback_piece *)newPiecePointer;

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

static uint8_t
set_piece_data(void *piecePointer, const uint8_t *data)
{
  struct piggyback_piece *piece = (struct piggyback_piece *)piecePointer;
  if(piece == NULL) {
    return 0;
  }
  uint8_t i=0;
  for(;i<piece->dataSize;i++) {
    piece->data[i] = data[i];
  }
  return 1;
}

void
packet_delete_piggyback_piece(void *piecePointer) {
  struct piggyback_piece *piece = (struct piggyback_piece *)piecePointer;
  if(piece->dataSize > 0) {
    free(piece->data);
  }
  free(piecePointer);
}

void
packet_delete_msg_piece(void *piecePointer) {
  struct piggyback_piece *piece = (struct piggyback_piece *)piecePointer;
  if(piece->dataSize > 0) {
    free(piece->data);
  }
  free(piecePointer);
}

/************************************************************************/
void *
packet_create_msg_piece(uint16_t payloadSize, uint8_t allowAggregation,
    uint8_t allowFragmentation, iotus_packets_priority priority,
    uint16_t timeout, const uint8_t* payload,
    const uint8_t *finalDestination, void *callbackFunction){

  void *newMsg = malloc_piece(payloadSize, sizeof(struct msg_piece));
  
  if(!set_piece_data(newMsg, payload)) {
    //Alloc failed, cancel operation
    packet_delete_msg_piece(newMsg);
    return NULL;
  }

  LIST_STRUCT_INIT((struct msg_piece *)newMsg, infoPieces);
  ((struct msg_piece *)newMsg)->initialBitHeaderSize = 0;
  ((struct msg_piece *)newMsg)->finalBytesHeaderSize = 0;

  uint8_t params = 0;
  /* Encode parameters */
  if(allowAggregation)
    params  = PACKET_PARAMETERS_ALLOW_AGGREGATION;
  if(allowFragmentation)
    params |= PACKET_PARAMETERS_ALLOW_FRAGMENTATION;
  params |= (PACKET_PARAMETERS_PRIORITY_FIELD & priority);

  ((struct msg_piece *)newMsg)->initialBitHeader = NULL;
  ((struct msg_piece *)newMsg)->finalBytesHeader = NULL;
  ((struct msg_piece *)newMsg)->totalPacketSize = payloadSize;

  ((struct msg_piece *)newMsg)->params = params;
  ((struct msg_piece *)newMsg)->timeout = timeout;
  ((struct msg_piece *)newMsg)->callbackHandler = callbackFunction;

  //Link the message into the list
  list_push(gPacketMsgList, newMsg);

  return newMsg;
}

/*
 * Function to create header pieces to be attached
 * @Params headerSize Size of the header in bytes
 * @Params headerData The information to be attached
 * @Params isBroadcastable Indicate if this packet will be able lto be broadcast
 * @Result Pointer to the struct with the final packet to be transmited (read by framer layer).
 */
void *
packet_create_piggyback_piece(uint16_t headerSize, const uint8_t* headerData,
    uint8_t type, const uint8_t *nextDestination)
{
  void *newHeader = malloc_piece(headerSize, sizeof(struct piggyback_piece));
  if(headerSize) {
    if(!set_piece_data(newHeader, headerData)) {
      //Alloc failed, cancel operation
      packet_delete_piggyback_piece(newHeader);
      return NULL;
    }
  }

  uint8_t params;
  /* Encode parameters */

  //set_piece_parameters(newHeader, params);
  //set_piggyback_piece_type(newHeader, type);

  //Link the header into the list
  list_push(gPacketHeaderList, newHeader);
  return newHeader;
}

/*
 * Function to get the total size of a packet
 * @Params msg_piece Packet to be read.
 * @Result Total size.
 */
uint16_t
iotus_packet_packet_size(void *msg_piece) {
  return (((struct msg_piece *)msg_piece)->initialBitHeaderSize +7)/8 +
         (((struct msg_piece *)msg_piece)->finalBytesHeaderSize) +
         (((struct msg_piece *)msg_piece)->dataSize);
}


/*
 * Sets the default chore that will be present in every default packet.
 */
void
packet_subscribe_default_header_chore(iotus_packets_priority priority,
  iotus_default_header_chores func)
{
  uint8_t posByte = func/4;
  uint8_t posBit = (func%4)*2;
  uint8_t chore = default_layers_chores_header[posByte] & (11<<posBit);
  if(chore > 0) {
    //There is some layer responsible for this chore...
    if(chore <= (priority<<posBit)) {
      //This request has lower priority (higher value)
      return;
    }
    default_layers_chores_header[posByte] &= ~(11<<posBit);
  }
  //This request has higher priority (lower value)
  //substitute this chore to this request
  default_layers_chores_header[posByte] |= (priority<<posBit);
}

/*
 * Verifies if default chore header is assigned to some layer.
 * @Return 1 for true, 0 for false.
 */
uint8_t
packet_verify_default_header_chore(iotus_packets_priority priority,
  iotus_default_header_chores func)
{
  uint8_t posByte = func/4;
  uint8_t posBit = (func%4)*2;
  uint8_t chore = default_layers_chores_header[posByte] & (11<<posBit);
  if(chore == (priority<<posBit)) {
    return 1;
  }
  return 0;
}


/*
 * Function to push bits into the header
 * @Params bitSequenceSize The amount of bit that will be push into.
 * @Params bitSeq An array of bytes containing the bits
 * @Params msg_piece Msg to apply this push
 * @Result Packet final size
 */
uint16_t
iotus_packet_push_bit_header(uint16_t bitSequenceSize, const uint8_t *bitSeq,
  void *msg_piece) {

  uint16_t i;
  uint16_t newSizeInBYTES = 0;
  uint16_t oldSizeInBYTES = ((((struct msg_piece *)msg_piece)->initialBitHeaderSize)+7)/8;//round up
  
  //Verify if the msg piece already has something
  if(NULL == ((struct msg_piece *)msg_piece)->initialBitHeader) {
    ((struct msg_piece *)msg_piece)->initialBitHeader = (uint8_t *)malloc((bitSequenceSize+7)/8);
    newSizeInBYTES = (bitSequenceSize+7)/8;

    if(((struct msg_piece *)msg_piece)->initialBitHeader == NULL) {
      /* Failed to alloc memory */
      PRINTF("Failed to allocate memory for initialBitHeader.");
      return 0;
    }
  } else {
    //Verify if a new byte is required
    uint16_t freeSpaceInBITS = (oldSizeInBYTES*8)-(((struct msg_piece *)msg_piece)->initialBitHeaderSize);
    if(freeSpaceInBITS < bitSequenceSize) {
      //Reallocate new buffer for this system
      newSizeInBYTES = (bitSequenceSize - freeSpaceInBITS + 7)/8;//round up

      uint8_t *newBuff = (uint8_t *)malloc(newSizeInBYTES + oldSizeInBYTES);

      if(newBuff == NULL) {
        /* Failed to alloc memory */
        PRINTF("Failed to allocate memory to expand initialBitHeader.");
        return 0;
      }

      //transfer the old buffer to the new one, backwards!
      for(i=(newSizeInBYTES + oldSizeInBYTES - 1); i >= oldSizeInBYTES; i--) {
        newBuff[i] = (((struct msg_piece *)msg_piece)->initialBitHeader)[i-newSizeInBYTES];
      }

      //Delete the old buffer
      free((void *)(((struct msg_piece *)msg_piece)->initialBitHeader));
      ((struct msg_piece *)msg_piece)->initialBitHeader = newBuff;
    }
  }

  //Insert the new bits information
  uint16_t byteToPush = (newSizeInBYTES + oldSizeInBYTES) - ((((struct msg_piece *)msg_piece)->initialBitHeaderSize)/8);
  uint8_t bitToPush = ((((struct msg_piece *)msg_piece)->initialBitHeaderSize)%8);
  uint16_t byteToRead = 1 + (bitSequenceSize/8);
  for(i=0; i < bitSequenceSize; i++) {
    //Verify and change the byte to be push into...
    if(bitToPush > 8) {
      bitToPush = 0;
      byteToPush--;
    }
    //Read the bits from the source
    uint8_t bitShifted = (1<<(i%8));
    if((i%8) == 0) {
      byteToRead--;
    }

    uint8_t bitRead = (bitSeq[byteToRead] & bitShifted);
    if(bitRead == 0) {
      (((struct msg_piece *)msg_piece)->initialBitHeader)[byteToPush] &= ~(1<<bitToPush);
    } else {
      (((struct msg_piece *)msg_piece)->initialBitHeader)[byteToPush] |= (1<<bitToPush);
    }
  }
  ((struct msg_piece *)msg_piece)->initialBitHeaderSize += bitSequenceSize;//counted in bits

  return iotus_packet_packet_size(msg_piece);
}


/*
 * Function to append full bytes headers into the tail (inversed)
 * @Params bytesSize The amount of bytes that will be appended.
 * @Params byteSeq An array of bytes in its normal sequence
 * @Params msg_piece Msg to apply this append
 * @Result Packet final size
 */
uint16_t
iotus_packet_append_byte_header(uint16_t byteSequenceSize, const uint8_t *headerToAppend,
  void *msg_piece) {
  int i;//Verify if the msg piece already has something
  uint16_t totalFinalSize = 0;
  if(NULL == ((struct msg_piece *)msg_piece)->finalBytesHeader) {
    ((struct msg_piece *)msg_piece)->finalBytesHeader = (uint8_t *)malloc(byteSequenceSize);

    if(((struct msg_piece *)msg_piece)->finalBytesHeader == NULL) {
      /* Failed to alloc memory */
      PRINTF("Failed to allocate memory for finalBytesHeader.");
      return 0;
    }
    totalFinalSize = byteSequenceSize;
  } else {
    //Reallocate new buffer for this system
    totalFinalSize = byteSequenceSize + ((struct msg_piece *)msg_piece)->finalBytesHeaderSize;

    uint8_t *newBuff = (uint8_t *)malloc(totalFinalSize);

    if(newBuff == NULL) {
      /* Failed to alloc memory */
      PRINTF("Failed to allocate memory to expand finalBytesHeader.");
      return 0;
    }

    //transfer the old buffer to the new one! (left to the right)
    for(i=0; i < (((struct msg_piece *)msg_piece)->finalBytesHeaderSize); i++) {
      newBuff[i] = (((struct msg_piece *)msg_piece)->finalBytesHeader)[i];
    }

    //Delete the old buffer
    free((void *)(((struct msg_piece *)msg_piece)->finalBytesHeader));
    ((struct msg_piece *)msg_piece)->finalBytesHeader = newBuff;
  }

  //update size
  ((struct msg_piece *)msg_piece)->finalBytesHeaderSize = totalFinalSize;
  
  //Insert the new bytes, backwards...
  for(i=0; i < byteSequenceSize; i++) {
    (((struct msg_piece *)msg_piece)->finalBytesHeader)[totalFinalSize - i - 1] = ((uint8_t *)headerToAppend)[i];
  }

  return iotus_packet_packet_size(msg_piece);
}


/*
 * Function to append piggyback informations into the tail of the msg (inversed)
 * @Params bytesSize The amount of bytes that will be appended.
 * @Params byteSeq An array of bytes in its normal sequence
 * @Params msg_piece Msg to apply this append
 * @Result Packet final size
 */
uint16_t
iotus_packet_apply_piggyback(void *msg_piece) {
  if(!(((struct msg_piece *)msg_piece)->params & PACKET_PARAMETERS_ALLOW_PIGGYBACK)) {
    //No piggyback allowed for this packet
    return 0;
  }
  //Look for header pieces that match this packet conditions
  struct piggyback_piece *h;
  struct piggyback_piece *nextH;
  uint16_t packetOldSize = iotus_packet_packet_size(msg_piece);
  

  h = list_head(gPacketHeaderList);
  while(h != NULL) {
    nextH = list_item_next(h);
    //if(COMPARE DESTINATION ADDRESS)
    if(!(((struct msg_piece *)msg_piece)->params & PACKET_PARAMETERS_ALLOW_FRAGMENTATION)) {
      /*
        We will only get in here if this packet do NOT accept fragmentation. 
        Because this list is already ordered into latency sequence,
        no need to sort or anything
      */
      if((iotus_radio_max_message - packetOldSize) >= h->dataSize) {
        //This will fit...
        uint16_t newSize = iotus_packet_append_byte_header(h->dataSize, h->data, msg_piece);
        if(newSize != packetOldSize) {
          //Operation success
          list_remove(gPacketHeaderList, h);
          //TODO CALL the CB function of the header
          packet_delete_piggyback_piece(h);
        }
      }
    } else {
    //TODO This packet allows fragmentation...
      PRINTF("Packet piggyback with fragmentation not done... TODO");
    }

    h = nextH;
  }

  return iotus_packet_packet_size(msg_piece);
}

/*
 * Function to read any byte of a message, given its position
 * @Params bytePos The position of the byte
 * @Params msg_piece Packet to be read.
 * @Result Byte read.
 */
uint8_t
iotus_packet_read_byte(uint16_t bytePos, void *msg_piece) {
  if(iotus_packet_packet_size(msg_piece) <= bytePos) {
    return 0;
  }
  //paddingRemover starts with the size of the initial Bit header size in Bytes...
  uint16_t paddingRemover = ((((struct msg_piece *)msg_piece)->initialBitHeaderSize +7)/8);
  if (bytePos < paddingRemover) {
    //This byte is one of the bit headers
    return (((struct msg_piece *)msg_piece)->initialBitHeader)[
                          bytePos
                          ];
  }
  if(bytePos < (paddingRemover + ((struct msg_piece *)msg_piece)->dataSize)) {
    //BytePos is located into payload (data) section
    return (((struct msg_piece *)msg_piece)->data)[bytePos - paddingRemover];
  }

  paddingRemover += ((struct msg_piece *)msg_piece)->dataSize;
  return (((struct msg_piece *)msg_piece)->finalBytesHeader)[bytePos - paddingRemover];
}

/*
 * Default function required from IoTUS, to initialize, run and finish this service
 */
void
iotus_signal_handler_packet(iotus_service_signal signal, void *data)
{
  if(IOTUS_START_SERVICE == signal) {
    PRINTF("\tService Packet\n");

    // Initiate the lists of module
    list_init(gPacketMsgList);
    list_init(gPacketHeaderList);

    //Reset the default packet buffer
    int i = 0;
    for(;i<DEFAULT_FUNCTIONALITIES_SIZE; i++) {
      default_layers_chores_header[i] = 0;
    }
  } else if (IOTUS_RUN_SERVICE == signal){

  } else if (IOTUS_END_SERVICE == signal){

  }
}

/* The following stuff ends the \defgroup block at the beginning of
   the file: */

/** @} */
