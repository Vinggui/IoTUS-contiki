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
  pieces_clean_additional_info_list(piece->additionalInfoList);
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
packet_set_tx_block(iotus_packet_t *packetPiece, int8_t power, uint8_t channel)
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
      SAFE_PRINTF_LOG_ERROR("Add info not set");
      return FAILURE;
    }
  }
  /*
    Not necessary anymore...
      //Just set the value into the buffer
      uint8_t *addInfoDataPointer = pieces_get_data_pointer(txBlockInfo);
      memcpy((uint8_t *)(&txBlock_var), addInfoDataPointer,sizeof(packet_tx_block_input_t));
      txBlock_var.txPower = power;
      memcpy(addInfoDataPointer, (uint8_t *)(&txBlock_var),sizeof(packet_tx_block_input_t));
   */
  return SUCCESS;
}

/*---------------------------------------------------------------------*/
/**
 * \brief               Set a specific rx block for a packet transmission
 * \param packetPiece   Packet to set.
 * \param netId         The network Id of this packet (PanID).
 * \param linkQuality   The link quality received with this packet.
 * \param rssi          This RSSI value received with this packet.
 * \return              Status of this set.
 */
Status
packet_set_rx_block(iotus_packet_t *packetPiece, uint16_t netId, uint8_t linkQuality, uint8_t rssi)
{
  iotus_additional_info_t *rxBlockInfo = pieces_get_additional_info(packetPiece->additionalInfoList,
                    IOTUS_PACKET_INFO_TYPE_RADIO_RCV_BLCK);

  packet_rcv_block_output_t rxBlock_var;
  if(NULL == rxBlockInfo) {
    rxBlock_var.networkID = netId;
    rxBlock_var.linkQuality = linkQuality;
    rxBlock_var.rssi = rssi;
    //This packet does not have this block yet
    rxBlockInfo = pieces_set_additional_info(packetPiece->additionalInfoList,
                                IOTUS_PACKET_INFO_TYPE_RADIO_TX_BLCK,
                                (uint8_t *)(&rxBlock_var),
                                sizeof(packet_rcv_block_output_t),
                                TRUE);
    if(rxBlockInfo == NULL) {
      SAFE_PRINTF_LOG_ERROR("Add info not set");
      return FAILURE;
    }
  }
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
    uint16_t timeout, const uint8_t* payload, Boolean insertIotusHeader,
    iotus_node_t *finalDestination, void *callbackFunction)
{

  iotus_packet_t *newMsg = (iotus_packet_t *)pieces_malloc(
                                    &iotus_packet_struct_mem,
                                    sizeof(iotus_packet_t),
                                    payload, payloadSize);
  if(NULL == newMsg) {
    SAFE_PRINTF_LOG_ERROR("Alloc failed.");
    return NULL;
  }

  timestamp_mark(&(newMsg->timeout), timeout);
  LIST_STRUCT_INIT(newMsg, additionalInfoList);
  
  //this packet will go down the stack, towards the physical layer
  newMsg->firstHeaderBitSize = 0;
  newMsg->lastHeaderSize = 0;
  newMsg->nextDestinationNode = NULL;
  newMsg->params = 0;
  newMsg->iotusHeader = PACKET_IOTUS_HDR_FIRST_BIT;
  newMsg->priority = priority;
  newMsg->callbackHandler = callbackFunction;
  
  newMsg->finalDestinationNode = finalDestination;

  //Set some params into this packet
  if(TRUE == insertIotusHeader) {
    packet_set_parameter(newMsg,PACKET_PARAMETERS_IS_NEW_PACKET_SYSTEM);
  }
  if(finalDestination == NODES_BROADCAST) {
    SAFE_PRINT("It`s broadcast\n");
    newMsg->iotusHeader |= PACKET_IOTUS_HDR_IS_BROADCAST;
  }

  //Link the message into the list, sorting...
  pieces_insert_timeout_priority(gPacketMsgList, newMsg);

  return newMsg;
}

/*---------------------------------------------------------------------*/
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
 * \brief Function to push bits into the header
 * \param bitSequenceSize The amount of bit that will be push into.
 * \param bitSeq An array of bytes containing the bits
 * \param packetPiece Msg to apply this push
 * \return Packet final size
 */
uint16_t
packet_push_bit_header(uint16_t bitSequenceSize, const uint8_t *bitSeq,
  iotus_packet_t *packetPiece) {

  /* This operation is only allowed to packets going down the stack (to be transmitted) */
  if(packetPiece->priority == IOTUS_PRIORITY_RADIO) {
    return 0;
  }
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
  uint16_t byteToPushToPkt = (newSizeInBYTES + oldSizeInBYTES) - 1 - ((packetPiece->firstHeaderBitSize)/8);
  uint8_t bitToPush = ((packetPiece->firstHeaderBitSize)%8);
  // this 1 + is due to the sequence in which this value is decreased.
  uint16_t byteToReadFromInput = 1 + (bitSequenceSize/8);
  
  for(i=0; i < bitSequenceSize; i++) {
    //Verify and change the byte to be push into...
    if(bitToPush > 8) {
      bitToPush = 0;
      byteToPushToPkt--;
    }
    //Read the bits from the source
    uint8_t bitShifted = (1<<(i%8));
    if((i%8) == 0) {
      byteToReadFromInput--;
    }

    uint8_t bitRead = (bitSeq[byteToReadFromInput] & bitShifted);
    if(bitRead == 0) {
      (pieces_get_data_pointer(packetPiece))[byteToPushToPkt] &= ~(1<<bitToPush);
    } else {
      (pieces_get_data_pointer(packetPiece))[byteToPushToPkt] |= (1<<bitToPush);
    }
    bitToPush++;
  }
  packetPiece->firstHeaderBitSize += bitSequenceSize;//counted in bits

  return packet_get_size(packetPiece);
}


/*---------------------------------------------------------------------*/
/*
 * \brief Function to append full bytes headers into the tail (inversed)
 * \param bytesSize The amount of bytes that will be appended.
 * \param byteSeq An array of bytes in its normal sequence
 * \param packetPiece Msg to apply this append
 * \return Packet final size, 0 if failed.
 */
uint16_t
packet_append_last_header(uint16_t byteSequenceSize, const uint8_t *headerToAppend,
  iotus_packet_t *packetPiece) {
  int i;//Verify if the msg piece already has something

  /* This operation is only allowed to packets going down the stack (to be transmitted) */
  if(packetPiece->priority == IOTUS_PRIORITY_RADIO) {
    return 0;
  }

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
 * \brief  Function to read any byte of a message, given its position
 * \param bytePos The position of the byte
 * \param packetPiece Packet to be read.
 * \return Byte read.
 */
uint8_t
packet_read_byte(uint16_t bytePos, iotus_packet_t *packetPiece)
{
  if(packet_get_size(packetPiece) <= bytePos) {
    return 0;
  }
  return pieces_get_data_pointer(packetPiece)[bytePos];
}

/*---------------------------------------------------------------------*/
/*
 * \brief  Read bytes appended in an incoming packet.
 * \param packetPiece Packet to be read.
 * \param buf         The buf to save the read data into.
 * \param num         The number of byes to be read.
 * \return Byte read.
 */
Status
packet_unwrap_appended_byte(iotus_packet_t *packetPiece, uint8_t *buf, uint16_t num)
{
  if(buf == NULL || packetPiece == NULL) {
    return FAILURE;
  }
  uint16_t i;
  int32_t pos;
  for(i=0;i<num;i++) {
    pos = packetPiece->data.size -1 - packetPiece->lastHeaderSize;
    if(pos < 0) {
      return FAILURE;
    }

    *buf = pieces_get_data_pointer(packetPiece)[pos];
    buf++;
    packetPiece->lastHeaderSize++;
  }
  return SUCCESS;
}

/*---------------------------------------------------------------------*/
/*
 * \brief  Read bits pushed in an incoming packet. Max of 32 bit a at time.
 * \param 
 * \param packetPiece Packet to be read.
 * \return The sequence of Bit read.
 */
uint32_t
packet_unwrap_pushed_bit(iotus_packet_t *packetPiece, uint8_t num)
{
  if(packetPiece == NULL || num > 32) {
    return 0;
  }
  uint32_t result;
  uint16_t byteToStart;
  /**
   * Adding this remainder is necessary for correct read of the whole requested
   * num size.
   */
  num += packetPiece->firstHeaderBitSize%8;
  byteToStart = packetPiece->firstHeaderBitSize/8;

  result = 0;
  while(num/8 > 0) {
    result = (result<<8);
    result |= (uint32_t)pieces_get_data_pointer(packetPiece)[byteToStart];
    num -= 8;
    byteToStart++;
    packetPiece->firstHeaderBitSize += 8;
  }
  //Complete getting the rest of the bits
  for(;num>0;num--) {
    byteToStart = packetPiece->firstHeaderBitSize/8;
    result = (result<<1);
    result |= (1 & (uint32_t)pieces_get_data_pointer(packetPiece)[byteToStart]);
    packetPiece->firstHeaderBitSize++;
  }
  return result;
}

/*---------------------------------------------------------------------*/
void
packet_parse(iotus_packet_t *packetPiece) {
  if(packetPiece == NULL) {
    return;
  }

  if(IOTUS_PRIORITY_RADIO == packetPiece->priority) {
    //Radio priority means that this pkt is going up the stack, towards app layer
    packetPiece->firstHeaderBitSize = 0;

    /**
     * Find the first set bit in this packet and jump to the next bit
     * of the iotus dynamic header.
     */
    uint8_t i;
    i=packet_read_byte(0,packetPiece);
    while(i>0) {
      i/=2;
      packetPiece->firstHeaderBitSize++;
    }
    packetPiece->firstHeaderBitSize = 9-packetPiece->firstHeaderBitSize;
    //Now that we found the first bit of the packet, we can ignore it.
    uint8_t byteMapToReset = 1<< (8-packetPiece->firstHeaderBitSize);
    *pieces_get_data_pointer(packetPiece) &= ~(byteMapToReset);

    //Get the dynamic header now
    packetPiece->iotusHeader = packet_unwrap_pushed_bit(packetPiece,sizeof(packetPiece->iotusHeader));
    
    if(packetPiece->iotusHeader & PACKET_IOTUS_HDR_IS_BROADCAST) {
      packetPiece->nextDestinationNode = NODES_BROADCAST;
    }
  }
}

/*---------------------------------------------------------------------*/
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
