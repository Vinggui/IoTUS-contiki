
/**
 * \defgroup description...
 *
 * This...
 *
 * @{
 */

/*
 * piggyback.c
 *
 *  Created on: Feb 27, 2018
 *      Author: vinicius
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lib/memb.h"
#include "list.h"
#include "iotus-core.h"
#include "global-functions.h"
#include "global-parameters.h"
#include "packet.h"
#include "packet-defs.h"
#include "pieces.h"
#include "piggyback.h"
#include "platform-conf.h"
#include "sys/ctimer.h"
#include "timestamp.h"

#define DEBUG IOTUS_DONT_PRINT//IOTUS_PRINT_IMMEDIATELY
#define THIS_LOG_FILE_NAME_DESCRITOR "piggyback"
#include "safe-printer.h"

#define PIGGYBACK_MAX_FRAME_SIZE            0x0FFF
#define PIGGYBACK_SINGLE_HEADER_FRAME       0x000F

MEMB(iotus_piggyback_mem, iotus_piggyback_t, IOTUS_PIGGYBACK_LIST_SIZE);
LIST(gPiggybackFramesList);

static struct ctimer piggyback_timeout_ctimer;

static void
update_piggy_timeout_timer(void);

/*---------------------------------------------------------------------*/
/*
 * 
 * \param 
 * \return Packet final size
 */
Boolean
piggyback_destroy(iotus_piggyback_t *piece) {
  list_remove(gPiggybackFramesList, piece);

  return pieces_destroy(&iotus_piggyback_mem, piece);
}

/*---------------------------------------------------------------------*/
/**
 * \brief     Destroy the list of additional information that a piece may contain.
 * \param h   The pointer to this list.
 */
void
piggyback_clean_list(list_t list)
{
  iotus_piggyback_t *h;
  while(NULL != (h =list_pop(list))) {
    piggyback_destroy(h);
  }
  SAFE_PRINTF_LOG_INFO("Piggy list clean");
}

/*---------------------------------------------------------------------*/
/*
 */
static void
piggyback_timeout_handler(void *ptr) {
  //Get the first of the list
  iotus_piggyback_t *h;
  h = list_pop(gPiggybackFramesList);

  if(h == NULL) {
    return;
  }
  // printf("aeeeeeee %u %u %s\n", (uint8_t)h->priority, pieces_get_data_size(h), pieces_get_data_pointer(h));

  iotus_initiate_msg(
                pieces_get_data_size(h),
                pieces_get_data_pointer(h),
                PACKET_PARAMETERS_WAIT_FOR_ACK | PACKET_PARAMETERS_ALLOW_PIGGYBACK,
                h->priority,
                0,
                h->finalDestinationNode);

  piggyback_destroy(h);

  update_piggy_timeout_timer();
}

/*---------------------------------------------------------------------*/
static void
update_piggy_timeout_timer(void) {
  //Look for header pieces that match this packet conditions
  iotus_piggyback_t *h;
  h = list_head(gPiggybackFramesList);

  if(h != NULL) {
    ctimer_set(&piggyback_timeout_ctimer, h->timeout, piggyback_timeout_handler, NULL);
  } else {
    ctimer_stop(&piggyback_timeout_ctimer);
  }
}

/*---------------------------------------------------------------------*/
/*
 * Function to create      header pieces to be attached
 * \param piggyPayloadSize Size of the header in bytes
 * \param piggyPayload     The information to be attached
 * \param type             The type of header to be piggybacked.
 * \param destinationNode  The destination node of this data.
 * \param timeout          The time available for this header to be piggyback, before it turns into a packet.
 * \return Pointer to the struct with the final packet to be transmited (read by framer layer).
 */
iotus_piggyback_t *
piggyback_create_piece(uint16_t piggyPayloadSize, const uint8_t* piggyPayload,
    uint8_t targetLayer, iotus_node_t *destinationNode, int16_t timeout)
{
  if(piggyPayloadSize > PIGGYBACK_MAX_FRAME_SIZE) {
    SAFE_PRINTF_LOG_ERROR("Piggy hdr too large");
    return NULL;
  }
  iotus_piggyback_t *newPiece = (iotus_piggyback_t *)pieces_malloc(
                                                        &iotus_piggyback_mem,
                                                        sizeof(iotus_piggyback_t),
                                                        NULL,
                                                        piggyPayloadSize);
  
  if(NULL == newPiece) {
    SAFE_PRINTF_LOG_ERROR("Alloc failed.");
    return NULL;
  }
  memcpy(pieces_get_data_pointer(newPiece),piggyPayload,piggyPayloadSize);
  
  timestamp_delay(&(newPiece->timeout), timeout);

  newPiece->priority = targetLayer;
  uint8_t params = IOTUS_PIGGYBACK_LAYER & targetLayer;
  /* Encode parameters 
   * 0b11000000 - 2 bits indicates to which layer this frame is supposed to be sent
   * 0b00100000 - 1 bit indicate if this frame is to the next node or stick to final destination
   * 0b00010000 - 1 bit indicate if this frame has a second byte to describe header size
   * 0b00001111 - 4 bits indicate the size of the header (if smaller than 16bytes) or the
   *                most significant byte of the size (if bigger).
   */
  if(piggyPayloadSize > PIGGYBACK_SINGLE_HEADER_FRAME) {
    params |= (uint8_t)(piggyPayloadSize&0x000F) | IOTUS_PIGGYBACK_ATTACHMENT_WITH_EXTENDED_SIZE;
    newPiece->extendedSize = (uint8_t)(piggyPayloadSize & 0x00FF);
  } else {
    params |= (uint8_t)(piggyPayloadSize & 0x000F);
  }
  newPiece->params = params;

  //Set the destination node
  newPiece->finalDestinationNode = destinationNode;

  //Link the header into the list, sorting insertion
  pieces_insert_timeout_priority(gPiggybackFramesList,newPiece);
  update_piggy_timeout_timer();

  SAFE_PRINTF_LOG_INFO("Frame created\n");
  return newPiece;
}


/*---------------------------------------------------------------------*/
static Boolean
insert_piggyback_to_packet(iotus_packet_t *packet_piece,
                    iotus_piggyback_t *piggyback_piece, Boolean stick,
                    Boolean isLastPiece, uint16_t availableSpace)
{
  /*
  if(packet_get_parameter(packet_piece, PACKET_PARAMETERS_ALLOW_FRAGMENTATION)) {
    //TODO This packet allows fragmentation...
    SAFE_PRINT("Packet w/ fragTODO\n");
  } else {
    */
   
  /*
    We will only get in here if this packet do NOT accept fragmentation. 
    Because this list is already ordered into latency sequence,
    no need to sort or anything
  */
  SAFE_PRINT("No frag\n");

  uint16_t packetOldSize = packet_get_size(packet_piece);
  uint16_t piggyback_with_ext_size;
  
  piggyback_with_ext_size = (piggyback_piece->params & IOTUS_PIGGYBACK_ATTACHMENT_WITH_EXTENDED_SIZE);

  if((availableSpace - packetOldSize) >= (piggyback_piece->data.size +
                                          member_size(iotus_piggyback_t,params) +
                                          member_size(iotus_piggyback_t,extendedSize))) {
    //This piggyback will fit...
    
    if(stick) {
      SAFE_PRINTF_LOG_INFO("Sticking piggy");
      piggyback_piece->params |= IOTUS_PIGGYBACK_ATTACHMENT_TYPE_FINAL_DEST;
    }

    // This changed, we'll use a new byte at the beggining *************
     // if(isLastPiece) {
    //   piggyback_piece->params |= IOTUS_PIGGYBACK_IS_FINAL_ATTACHMENT;
    //   SAFE_PRINTF_LOG_INFO("Last Piece att");
    // }

    //Insert the header at the end of the data buffer of this piggyback
    uint8_t tempBuffer[piggyback_piece->data.size +
                       member_size(iotus_piggyback_t,params) +
                       member_size(iotus_piggyback_t,extendedSize)];
    uint8_t *tempBuffPointer = tempBuffer;

    *tempBuffPointer = piggyback_piece->params;
    tempBuffPointer++;
    if(piggyback_with_ext_size) {
      *tempBuffPointer = piggyback_piece->extendedSize;
      tempBuffPointer++;
    }
    memcpy(tempBuffPointer, pieces_get_data_pointer(piggyback_piece), piggyback_piece->data.size);

    if(0 == packet_append_last_header((uint16_t)(tempBuffPointer-tempBuffer+piggyback_piece->data.size),
                                      tempBuffer,
                                      packet_piece)) {
      SAFE_PRINTF_LOG_ERROR("Append");
      return FALSE;
    }

    //Operation success
    SAFE_PRINTF_LOG_INFO("Appended");
    list_remove(gPiggybackFramesList, piggyback_piece);
    list_push(packet_piece->attachedPiggyback, piggyback_piece);
    //The callback will be done by the packet service,before destroying it.
    //
    //
    update_piggy_timeout_timer();
    return TRUE;
  }
  //}
  return FALSE;
}

/*---------------------------------------------------------------------*/
/*
 * Function to append piggyback informations into the tail of the msg (inversed)
 * \param bytesSize The amount of bytes that will be appended.
 * \param byteSeq An array of bytes in its normal sequence
 * \param packet_piece Msg to apply this append
 * \return Number of bytes inserted.
 */
uint16_t
piggyback_apply(iotus_packet_t *packet_piece, uint16_t availableSpace) {
  if(!packet_get_parameter(packet_piece, PACKET_PARAMETERS_ALLOW_PIGGYBACK) ||
    !(packet_get_parameter(packet_piece, PACKET_PARAMETERS_IS_NEW_PACKET_SYSTEM))) {
    //SAFE_PRINTF_LOG_WARNING("Pkt %p cant piggyback", packet_piece);
    //No piggyback allowed for this packet
    return 0;
  }

  //Look for header pieces that match this packet conditions
  iotus_piggyback_t *h;
  iotus_piggyback_t *nextH;
  Boolean isTheFirstPiggyback = TRUE;
  uint16_t packetOldSize;
  packetOldSize = packet_get_size(packet_piece);

  h = list_head(gPiggybackFramesList);
  if(h == NULL) {
    SAFE_PRINTF_LOG_ERROR("Piggy search NULL");
  }
  while(h != NULL) {
    nextH = list_item_next(h);
    SAFE_PRINTF_LOG_INFO("Piggy search ok");
    Boolean toFinalDestination = (h->finalDestinationNode == packet_get_final_destination(packet_piece));
    if(toFinalDestination ||
       h->finalDestinationNode == packet_get_next_destination(packet_piece)) {

      if(TRUE == insert_piggyback_to_packet(packet_piece,
                                             h,
                                             toFinalDestination,
                                             isTheFirstPiggyback,
                                             availableSpace)) {
        isTheFirstPiggyback = FALSE;
      }
    }
    h = nextH;
  }

  return packet_get_size(packet_piece)-packetOldSize;
}

/*---------------------------------------------------------------------*/
/*
 * Default function required from IoTUS, to initialize, run and finish this service
 */
void
iotus_signal_handler_piggyback(iotus_service_signal signal, void *data)
{
  if(IOTUS_START_SERVICE == signal) {
    SAFE_PRINT("\tService Piggyback\n");
    list_init(gPiggybackFramesList);
  }
  //Verify if any piggyback frame is expiring
  // piggyback_check_timeout();

  /*  else if (IOTUS_END_SERVICE == signal){

  }*/
}
/*---------------------------------------------------------------------*/