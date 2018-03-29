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
#include "lib/memb.h"
#include "list.h"
#include "packet.h"
#include "packet-default-additional-info.h"
#include "chores.h"
#include "pieces.h"
#include "platform-conf.h"
#include "nodes.h"


#define DEBUG IOTUS_PRINT_IMMEDIATELY
#define THIS_LOG_FILE_NAME_DESCRITOR "packet"
#include "safe-printer.h"


// Initiate the lists of module
MEMB(iotus_packet_struct_mem, iotus_packet_t, IOTUS_PACKET_LIST_SIZE);
LIST(gPacketMsgList);

/*---------------------------------------------------------------------*/
/*
 * 
 * \param 
 * \return Packet final size
 */
Boolean
packet_destroy(iotus_packet_t *piece) {
  list_remove(gPacketMsgList, piece);

  return pieces_destroy(&iotus_packet_struct_mem, piece);
}


/*---------------------------------------------------------------------*/
/*
 * \brief verify if a certain parameter is defined
 * \param packetPiece Packet to be read.
 * \param param Parameter to be verified
 * \return Boolean.
 */
uint8_t
packet_verify_parameter(iotus_packet_t *packetPiece, uint8_t param) {
  if(NULL == packetPiece) {
    SAFE_PRINTF_LOG_ERROR("Null pointer");
    return 0;
  }

  if(packetPiece->params & param)
    return TRUE;
  return FALSE;
}


/*---------------------------------------------------------------------*/
/*
 * \brief Allow other services to set a parameter into a msg
 * \param packetPiece Packet to be set.
 * \param param Parameter to be written
 * \return Boolean.
 */
void
packet_set_parameter(iotus_packet_t *packetPiece, uint8_t param) {
  if(NULL == packetPiece) {
    SAFE_PRINTF_LOG_ERROR("Null pointer");
    return;
  }

  /* Encode parameters */
  packetPiece->params |= param;
}

/*---------------------------------------------------------------------*/
/*
 * Return the pointer to the node of final destination
 * \param packetPiece       The pointer to the packet to be searched.
 * \return                The pointer to the node of the final destination.
 */
iotus_node_t *
packet_get_final_destination(iotus_packet_t *packetPiece)
{
  if(NULL == packetPiece) {
    SAFE_PRINTF_LOG_ERROR("Null pointer");
    return NULL;
  }
  return packetPiece->finalDestinationNode;
}


/*---------------------------------------------------------------------*/
/*
 * Return the pointer to the node of next destination
 * \param packetPiece       The pointer to the packet to be searched.
 * \return                The pointer to the node of the final destination.
 */
iotus_node_t *
packet_get_next_destination(iotus_packet_t *packetPiece)
{
  if(NULL == packetPiece) {
    SAFE_PRINTF_LOG_ERROR("Null pointer");
    return NULL;
  }
  return packetPiece->nextDestinationNode;
}

/*---------------------------------------------------------------------*/
/**
 * \brief               Set a specific tx power for a packet transmission
 * @param packetPiece   Packet to set.
 * @param power         The power in dBm, generally varing from 0 to -25
 * \return              Status of this set.
 */
Status
packet_set_tx_power(iotus_packet_t *packetPiece, int8_t power)
{
  iotus_additional_info_t *txBlockInfo = pieces_get_additional_info(packetPiece->additionalInfoList,
                    IOTUS_PACKET_INFO_TYPE_RADIO_TX_BLCK);

  packet_tx_block_input_t txBlock_var;
  if(NULL == txBlockInfo) {
    txBlock_var.txPower = power;
    //This packet does not have this block yet
    txBlockInfo = pieces_set_additional_info(packetPiece->additionalInfoList,
                                IOTUS_PACKET_INFO_TYPE_RADIO_TX_BLCK,
                                (uint8_t *)(&txBlock_var),
                                sizeof(packet_tx_block_input_t),
                                TRUE);
    if(txBlockInfo == NULL) {
      SAFE_PRINTF_LOG_ERROR("Add info no set");
      return FAILURE;
    }
  }
  //Just set the value into the buffer
  uint8_t *addInfoDataPointer = pieces_get_data_pointer(txBlockInfo);
  memcpy((uint8_t *)(&txBlock_var), addInfoDataPointer,sizeof(packet_tx_block_input_t));
  txBlock_var.txPower = power;
  memcpy(addInfoDataPointer, (uint8_t *)(&txBlock_var),sizeof(packet_tx_block_input_t));
  return SUCCESS;
}


/*---------------------------------------------------------------------*/
/**
 * \brief               Get a specific tx power for a packet transmission
 * @param packetPiece   Packet to check.
 * \return              The power selected, 0xFF for no selection
 */
int8_t
packet_get_tx_power(iotus_packet_t *packetPiece)
{
  iotus_additional_info_t *txBlockInfo = pieces_get_additional_info(packetPiece->additionalInfoList,
                    IOTUS_PACKET_INFO_TYPE_RADIO_TX_BLCK);

  if(NULL == txBlockInfo) {
    return 0xFF;
  }
  //Just set the value into the buffer
  packet_tx_block_input_t *txBlock_var = (packet_tx_block_input_t *)pieces_get_data_pointer(txBlockInfo);
  return txBlock_var->txPower;
}

/*---------------------------------------------------------------------*/
/*
 * 
 * \param 
 * \return Packet final size
 */
iotus_packet_t *
packet_create_msg(uint16_t payloadSize, iotus_layer_priority priority,
    uint16_t timeout, const uint8_t* payload,
    iotus_node_t *finalDestination, void *callbackFunction)
{

  iotus_packet_t *newMsg = (iotus_packet_t *)pieces_malloc(&iotus_packet_struct_mem, sizeof(iotus_packet_t), payload, payloadSize);
  if(NULL == newMsg) {
    SAFE_PRINTF_LOG_ERROR("Alloc failed.");
    return NULL;
  }

  LIST_STRUCT_INIT(newMsg, additionalInfoList);
  (newMsg)->firstHeaderBitSize = 0;
  (newMsg)->lastHeaderSize = 0;

  newMsg->nextDestinationNode = NULL;

  newMsg->params |= (PACKET_PARAMETERS_PRIORITY_FIELD & priority);
  //((struct packetPiece *)newMsg)->timeout = timeout;
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
 * \param packetPiece Packet to be read.
 * \return Total size.
 */
unsigned int
packet_get_size(iotus_packet_t *packetPiece) {
  return (packetPiece->data.size);
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
 * \param packetPiece Msg to apply this push
 * \return Packet final size
 */
uint16_t
packet_push_bit_header(uint16_t bitSequenceSize, const uint8_t *bitSeq,
  iotus_packet_t *packetPiece) {

  uint16_t i;
  uint16_t newSizeInBYTES = 0;
  uint16_t oldSizeInBYTES = ((packetPiece->firstHeaderBitSize)+7)/8;//round up
  uint16_t packetOldTotalSize = packet_get_size(packetPiece);
  
  //Verify if a new byte is required
  uint16_t freeSpaceInBITS = (oldSizeInBYTES*8)-(packetPiece->firstHeaderBitSize);
  if(freeSpaceInBITS < bitSequenceSize) {
    //Reallocate new buffer for this system
    newSizeInBYTES = (bitSequenceSize - freeSpaceInBITS + 7)/8;//round up


    uint8_t newBuff[newSizeInBYTES + packetOldTotalSize];
    for(i=0;i<newSizeInBYTES;i++) {
      newBuff[i]=0;
    }
    //transfer the old buffer to the new one, backwards!
    memcpy(newBuff+newSizeInBYTES,pieces_get_data_pointer(packetPiece),packetOldTotalSize);


    //Delete the old buffer
#if IOTUS_USING_MALLOC == 0
    //This sequence of free first then alloc makes more sense in constrained devices
    mmem_free(&(packetPiece->data));
    //recreate it...
    if(0 == mmem_alloc(&(packetPiece->data), newSizeInBYTES + packetOldTotalSize)) {
      /* Failed to alloc memory */
      SAFE_PRINTF_LOG_WARNING("Alloc failed");
      //retore old info...
      if(0 == mmem_alloc(&(packetPiece->data), packetOldTotalSize)) {
        SAFE_PRINTF_LOG_ERROR("Recovery Failed!");
        return 0;
      }
      memcpy(pieces_get_data_pointer(packetPiece),newBuff+newSizeInBYTES,packetOldTotalSize);
      return 0;
    }
#else
    //This sequence of free first then alloc makes more sense in constrained devices
    free(pieces_get_data_pointer(packetPiece));
    //recreate it...
    packetPiece->data.ptr = malloc(newSizeInBYTES + packetOldTotalSize);
    if(NULL == packetPiece->data.ptr) {
      /* Failed to alloc memory */
      SAFE_PRINTF_LOG_WARNING("Alloc failed");
      //retore old info...
      packetPiece->data.ptr = malloc(packetOldTotalSize);
      if(NULL == packetPiece->data.ptr) {
        SAFE_PRINTF_LOG_ERROR("Recovery Failed!");
        return 0;
      }
      memcpy(pieces_get_data_pointer(packetPiece),newBuff+newSizeInBYTES,packetOldTotalSize);
      return 0;
    }
    packetPiece->data.size += newSizeInBYTES;
#endif
    memcpy(pieces_get_data_pointer(packetPiece),newBuff,packetOldTotalSize+newSizeInBYTES);
  }

  //Insert the new bits information
  uint16_t byteToPush = (newSizeInBYTES + oldSizeInBYTES) - ((packetPiece->firstHeaderBitSize)/8);
  uint8_t bitToPush = ((packetPiece->firstHeaderBitSize)%8);
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
      (pieces_get_data_pointer(packetPiece))[byteToPush] &= ~(1<<bitToPush);
    } else {
      (pieces_get_data_pointer(packetPiece))[byteToPush] |= (1<<bitToPush);
    }
  }
  packetPiece->firstHeaderBitSize += bitSequenceSize;//counted in bits

  return packet_get_size(packetPiece);
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
 * \param packetPiece Msg to apply this append
 * \return Packet final size
 */
uint16_t
packet_append_last_header(uint16_t byteSequenceSize, const uint8_t *headerToAppend,
  iotus_packet_t *packetPiece) {
  int i;//Verify if the msg piece already has something
  uint16_t packetOldTotalSize = packet_get_size(packetPiece);
  uint16_t packetNewTotalSize = packetOldTotalSize + byteSequenceSize;
  //Reallocate new buffer for this system
  uint8_t newBuff[packetNewTotalSize];

/* this was malloc before...
  if(newBuff == NULL) {
    SAFE_PRINTF_LOG_ERROR("allocate memory");
    return 0;
  }
*/

  //transfer the old buffer to the new one! (left to the right)
  memcpy(newBuff, pieces_get_data_pointer(packetPiece), packetOldTotalSize);

  //Delete the old buffer
#if IOTUS_USING_MALLOC == 0
  //This sequence of free first then alloc makes more sense in constrained devices
  mmem_free(&(packetPiece->data));
  //recreate it...
  if(0 == mmem_alloc(&(packetPiece->data), packetNewTotalSize)) {
    /* Failed to alloc memory */
    SAFE_PRINTF_LOG_WARNING("Alloc failed");
    //retore old info...
    if(0 == mmem_alloc(&(packetPiece->data), packetOldTotalSize)) {
      SAFE_PRINTF_LOG_ERROR("Recovery Failed!");
      return 0;
    }
    memcpy(pieces_get_data_pointer(packetPiece),newBuff,packetOldTotalSize);
    return 0;
  }
#else
  //This sequence of free first then alloc makes more sense in constrained devices
  free(pieces_get_data_pointer(packetPiece));
  //recreate it...
  packetPiece->data.ptr = malloc(packetNewTotalSize);
  if(NULL == packetPiece->data.ptr) {
    /* Failed to alloc memory */
    SAFE_PRINTF_LOG_WARNING("Alloc failed");
    //retore old info...
    packetPiece->data.ptr = malloc(packetOldTotalSize);
    if(NULL == packetPiece->data.ptr) {
      SAFE_PRINTF_LOG_ERROR("Recovery Failed!");
      return 0;
    }
    memcpy(pieces_get_data_pointer(packetPiece),newBuff,packetOldTotalSize);
    return 0;
  }
  packetPiece->data.size += byteSequenceSize;
#endif
  memcpy(pieces_get_data_pointer(packetPiece),newBuff,packetOldTotalSize);

  //update size
  packetPiece->lastHeaderSize += byteSequenceSize;
  
  //Insert the new bytes, backwards...
  for(i=0; i < byteSequenceSize; i++) {
    pieces_get_data_pointer(packetPiece)[packetNewTotalSize - i - 1] = headerToAppend[i];
  }

  return packet_get_size(packetPiece);
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
 * \param packetPiece Packet to be read.
 * \return Byte read.
 */
uint8_t
packet_read_byte(uint16_t bytePos, iotus_packet_t *packetPiece) {
  if(packet_get_size(packetPiece) <= bytePos) {
    return 0;
  }
  return pieces_get_data_pointer(packetPiece)[bytePos];
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

    #if IOTUS_USING_MALLOC == 0
    SAFE_PRINT("\tService Packet:MMEM\n");
    #else
    SAFE_PRINT("\tService Packet:Malloc\n");
    #endif
    

    // Initiate the lists of module
    list_init(gPacketMsgList);
  } else if (IOTUS_RUN_SERVICE == signal){

  } else if (IOTUS_END_SERVICE == signal){

  }
}

/* The following stuff ends the \defgroup block at the beginning of
   the file: */

/** @} */
