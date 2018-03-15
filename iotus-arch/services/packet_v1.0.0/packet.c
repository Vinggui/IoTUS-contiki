
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
#include "global-parameters.h"
#include "iotus-core.h"
#include "packet-defs.h"
#include "list.h"
#include "packet.h"
#include "packet-default-additional-info.h"
#include "packet-default-chores.h"
#include "pieces.h"
#include "platform-conf.h"
#include "nodes.h"

#define DEBUG 1
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else /* DEBUG */
#define PRINTF(...)
#endif /* DEBUG */

/* Buffer to indicate which functionalities are execute by each layer.
 * This variable is set by each protocol when starting
 */
#define DEFAULT_FUNCTIONALITIES_SIZE  (IOTUS_FINAL_NUMBER_DEFAULT_HEADER_CHORE*2/8 +1)
static uint8_t default_layers_chores_header[DEFAULT_FUNCTIONALITIES_SIZE] = {0};


// Initiate the lists of module
LIST(gPacketMsgList);

/*---------------------------------------------------------------------*/
/*
 * 
 * \param 
 * \return Packet final size
 */
void
packet_delete_msg(iotus_packet_t *piece) {
  if(piece->dataSize > 0) {
    free(piece->data);
  }
  free(piece);
}


/*---------------------------------------------------------------------*/
/*
 * 
 * \param 
 * \return Packet final size
 */
/*
 * \brief Function to allow other services to verify the status of a parameter into a msg
 * \param packet_piece Packet to be read.
 * \param param Parameter to be verified
 * \return Boolean.
 */
uint8_t
packet_verify_parameter(iotus_packet_t *packet_piece, uint8_t param) {
  if(NULL == packet_piece) {
    PRINTF("PACKET: Null pointer to verify parameter");
    return 0;
  }

  if(packet_piece->params & param)
    return TRUE;
  return FALSE;
}


/*---------------------------------------------------------------------*/
/*
 * 
 * \param 
 * \return Packet final size
 */
/*
 * \brief Function to allow other services to set a parameter into a msg
 * \param packet_piece Packet to be set.
 * \param param Parameter to be written
 * \return Boolean.
 */
void
packet_set_parameter(iotus_packet_t *packet_piece, uint8_t param) {
  if(NULL == packet_piece) {
    PRINTF("PACKET: Null pointer to set parameter");
    return;
  }

  /* Encode parameters */
  packet_piece->params |= param;
}

/*---------------------------------------------------------------------*/
/*
 * Return the pointer to the node of final destination
 * \param packet_piece       The pointer to the packet to be searched.
 * \return                The pointer to the node of the final destination.
 */
Iotus_node *
packet_get_final_destination(iotus_packet_t *packet_piece)
{
  if(NULL == packet_piece) {
    PRINTF("PACKET: Null pointer to set parameter");
    return NULL;
  }
  PRINTF("GOT HERE");
  return packet_piece->finalDestinationNode;
}

/*---------------------------------------------------------------------*/
/*
 * 
 * \param 
 * \return Packet final size
 */
iotus_packet_t *
packet_create_msg(uint16_t payloadSize, iotus_packets_priority priority,
    uint16_t timeout, const uint8_t* payload,
    Iotus_node *finalDestination, void *callbackFunction){

  iotus_packet_t *newMsg = (iotus_packet_t *)pieces_malloc(payloadSize, sizeof(iotus_packet_t));
  
  if(!pieces_set_data(newMsg, payload)) {
    //Alloc failed, cancel operation
    packet_delete_msg(newMsg);
    return NULL;
  }

  LIST_STRUCT_INIT(newMsg, infoPieces);
  (newMsg)->initialBitHeaderSize = 0;
  (newMsg)->finalBytesHeaderSize = 0;

  newMsg->initialBitHeader = NULL;
  newMsg->finalBytesHeader = NULL;
  newMsg->nextDestinationNode = NULL;
  newMsg->totalPacketSize = payloadSize;

  newMsg->params |= (PACKET_PARAMETERS_PRIORITY_FIELD & priority);
  //((struct packet_piece *)newMsg)->timeout = timeout;
  newMsg->callbackHandler = callbackFunction;

  newMsg->finalDestinationNode = finalDestination;


  //Link the message into the list, sorting...
  pieces_insert_timeout_priority(gPacketMsgList, newMsg);

  return newMsg;
}

/*---------------------------------------------------------------------*/
/*
 * 
 * \param 
 * \return Packet final size
 */
/*
 * \brief Function to get the total size of a packet
 * \param packet_piece Packet to be read.
 * \return Total size.
 */
uint16_t
packet_get_size(iotus_packet_t *packet_piece) {
  return (packet_piece->initialBitHeaderSize +7)/8 +
         (packet_piece->finalBytesHeaderSize) +
         (packet_piece->dataSize);
}


/*---------------------------------------------------------------------*/
/*
 * 
 * \param 
 * \return Packet final size
 */
/*
 * Sets the default chore that will be present in every default packet.
 */
void
packet_subscribe_for_chore(iotus_packets_priority priority,
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

/*---------------------------------------------------------------------*/
/*
 * \brief  Verifies if default chore header is assigned to some layer
 * \param func      
 * \return Packet final size
 */
/*
 * .
 * @Return 1 for true, 0 for false.
 */
uint8_t
packet_get_layer_assigned_for(iotus_default_header_chores func)
{
  uint8_t posByte = func/4;
  uint8_t posBit = (func%4)*2;
  uint8_t chore = (default_layers_chores_header[posByte]>>posBit) & 0b00000011;
  return chore;
}


/*---------------------------------------------------------------------*/
/*
 * 
 * \param 
 * \return Packet final size
 */
/*
 * \brief Function to push bits into the header
 * \param bitSequenceSize The amount of bit that will be push into.
 * \param bitSeq An array of bytes containing the bits
 * \param packet_piece Msg to apply this push
 * \return Packet final size
 */
uint16_t
packet_push_bit_header(uint16_t bitSequenceSize, const uint8_t *bitSeq,
  iotus_packet_t *packet_piece) {

  uint16_t i;
  uint16_t newSizeInBYTES = 0;
  uint16_t oldSizeInBYTES = ((packet_piece->initialBitHeaderSize)+7)/8;//round up
  
  //Verify if the msg piece already has something
  if(NULL == packet_piece->initialBitHeader) {
    packet_piece->initialBitHeader = (uint8_t *)malloc((bitSequenceSize+7)/8);
    newSizeInBYTES = (bitSequenceSize+7)/8;

    if(packet_piece->initialBitHeader == NULL) {
      /* Failed to alloc memory */
      PRINTF("Failed to allocate memory for initialBitHeader.");
      return 0;
    }
  } else {
    //Verify if a new byte is required
    uint16_t freeSpaceInBITS = (oldSizeInBYTES*8)-(packet_piece->initialBitHeaderSize);
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
        newBuff[i] = (packet_piece->initialBitHeader)[i-newSizeInBYTES];
      }

      //Delete the old buffer
      free((void *)(packet_piece->initialBitHeader));
      packet_piece->initialBitHeader = newBuff;
    }
  }

  //Insert the new bits information
  uint16_t byteToPush = (newSizeInBYTES + oldSizeInBYTES) - ((packet_piece->initialBitHeaderSize)/8);
  uint8_t bitToPush = ((packet_piece->initialBitHeaderSize)%8);
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
      (packet_piece->initialBitHeader)[byteToPush] &= ~(1<<bitToPush);
    } else {
      (packet_piece->initialBitHeader)[byteToPush] |= (1<<bitToPush);
    }
  }
  packet_piece->initialBitHeaderSize += bitSequenceSize;//counted in bits

  return packet_get_size(packet_piece);
}


/*---------------------------------------------------------------------*/
/*
 * 
 * \param 
 * \return Packet final size
 */
/*
 * \brief Function to append full bytes headers into the tail (inversed)
 * \param bytesSize The amount of bytes that will be appended.
 * \param byteSeq An array of bytes in its normal sequence
 * \param packet_piece Msg to apply this append
 * \return Packet final size
 */
uint16_t
packet_append_byte_header(uint16_t byteSequenceSize, const uint8_t *headerToAppend,
  iotus_packet_t *packet_piece) {
  int i;//Verify if the msg piece already has something
  uint16_t totalFinalSize = 0;
  if(NULL == packet_piece->finalBytesHeader) {
    packet_piece->finalBytesHeader = (uint8_t *)malloc(byteSequenceSize);

    if(packet_piece->finalBytesHeader == NULL) {
      /* Failed to alloc memory */
      PRINTF("Failed to allocate memory for finalBytesHeader.");
      return 0;
    }
    totalFinalSize = byteSequenceSize;
  } else {
    //Reallocate new buffer for this system
    totalFinalSize = byteSequenceSize + packet_piece->finalBytesHeaderSize;

    uint8_t *newBuff = (uint8_t *)malloc(totalFinalSize);

    if(newBuff == NULL) {
      /* Failed to alloc memory */
      PRINTF("Failed to allocate memory to expand finalBytesHeader.");
      return 0;
    }

    //transfer the old buffer to the new one! (left to the right)
    for(i=0; i < (packet_piece->finalBytesHeaderSize); i++) {
      newBuff[i] = (packet_piece->finalBytesHeader)[i];
    }

    //Delete the old buffer
    free((void *)(packet_piece->finalBytesHeader));
    packet_piece->finalBytesHeader = newBuff;
  }

  //update size
  packet_piece->finalBytesHeaderSize = totalFinalSize;
  
  //Insert the new bytes, backwards...
  for(i=0; i < byteSequenceSize; i++) {
    (packet_piece->finalBytesHeader)[totalFinalSize - i - 1] = ((uint8_t *)headerToAppend)[i];
  }

  return packet_get_size(packet_piece);
}

/*---------------------------------------------------------------------*/
/*
 * 
 * \param 
 * \return Packet final size
 */
/*
 * \brief  Function to read any byte of a message, given its position
 * \param bytePos The position of the byte
 * \param packet_piece Packet to be read.
 * \return Byte read.
 */
uint8_t
packet_read_byte(uint16_t bytePos, iotus_packet_t *packet_piece) {
  if(packet_get_size(packet_piece) <= bytePos) {
    return 0;
  }
  //paddingRemover starts with the size of the initial Bit header size in Bytes...
  uint16_t paddingRemover = ((packet_piece->initialBitHeaderSize +7)/8);
  if (bytePos < paddingRemover) {
    //This byte is one of the bit headers
    return (packet_piece->initialBitHeader)[
                          bytePos
                          ];
  }
  if(bytePos < (paddingRemover + packet_piece->dataSize)) {
    //BytePos is located into payload (data) section
    return (packet_piece->data)[bytePos - paddingRemover];
  }

  paddingRemover += packet_piece->dataSize;
  return (packet_piece->finalBytesHeader)[bytePos - paddingRemover];
}

/*---------------------------------------------------------------------*/
/*
 * 
 * \param 
 * \return Packet final size
 */
/*
 * \brief Default function required from IoTUS, to initialize, run and finish this service
 */
void
iotus_signal_handler_packet(iotus_service_signal signal, void *data)
{
  if(IOTUS_START_SERVICE == signal) {
    PRINTF("\tService Packet\n");

    // Initiate the lists of module
    list_init(gPacketMsgList);

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
