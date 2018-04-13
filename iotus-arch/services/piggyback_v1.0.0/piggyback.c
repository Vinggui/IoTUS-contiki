
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
#include "timestamp.h"

#define DEBUG IOTUS_PRINT_IMMEDIATELY
#define THIS_LOG_FILE_NAME_DESCRITOR "piggyback"
#include "safe-printer.h"

#define PIGGYBACK_MAX_FRAME_SIZE            0x0FFF
#define PIGGYBACK_SINGLE_HEADER_FRAME       0x000F

MEMB(iotus_piggyback_mem, iotus_piggyback_t, IOTUS_PIGGYBACK_LIST_SIZE);
LIST(gPiggybackFramesList);


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
/*
 * Function to create     header pieces to be attached
 * \param headerSize      Size of the header in bytes
 * \param headerData      The information to be attached
 * \param type            The type of header to be piggybacked.
 * \param destinationNode The destination node of this data.
 * \param timeout         The time available for this header to be piggyback, before it turns into a packet.
 * \return Pointer to the struct with the final packet to be transmited (read by framer layer).
 */
iotus_piggyback_t *
piggyback_create_piece(uint16_t headerSize, const uint8_t* headerData,
    uint8_t targetLayer, iotus_node_t *destinationNode, int32_t timeout)
{
  if(headerSize > PIGGYBACK_MAX_FRAME_SIZE) {
    SAFE_PRINTF_LOG_ERROR("Piggy hdr too large");
    return NULL;
  }
  iotus_piggyback_t *newPiece = (iotus_piggyback_t *)pieces_malloc(&iotus_piggyback_mem, sizeof(iotus_piggyback_t), headerData, headerSize);
  
  if(NULL == newPiece) {
    SAFE_PRINTF_LOG_ERROR("Alloc failed.");
    return NULL;
  }
  

  timestamp_mark(&(newPiece->timeout), timeout);

  uint8_t params = IOTUS_PIGGYBACK_LAYER & targetLayer;
  /* Encode parameters 
   * 0b00000011 - 2 bits indicates to which layer this frame is supposed to be sent
   * 0b00000100 - 1 bit indicate if this frame is to the next node or stick to final destination
   * 0b00001000 - 1 bit indicate if this frame has a second byte to describe header size
   * 0b11110000 - 4 bits indicate the size of the header (if smaller than 16bytes) or the
   *                most significant byte of the size (if bigger).
   */
  if(headerSize > PIGGYBACK_SINGLE_HEADER_FRAME) {
    params |= ((uint8_t)((headerSize>>8)&0x000F) << 4) | 0x08;
    newPiece->extendedSize = (uint8_t)(headerSize & 0x00FF);
  } else {
    params |= (uint8_t)((headerSize & 0x000F)<< 4);
  }
  newPiece->params = params;

  //Set the destination node
  newPiece->finalDestinationNode = destinationNode;

  //Link the header into the list, sorting insertion
  pieces_insert_timeout_priority(gPiggybackFramesList,newPiece);
  SAFE_PRINTF_LOG_INFO("Frame created");
  return newPiece;
}


/*---------------------------------------------------------------------*/
static Boolean
insert_piggyback_to_packet(iotus_packet_t *packet_piece,
                    iotus_piggyback_t *piggyback_piece, Boolean stick)
{
  if(packet_get_parameter(packet_piece, PACKET_PARAMETERS_ALLOW_FRAGMENTATION)) {
    //TODO This packet allows fragmentation...
    SAFE_PRINT("Packet w/ fragTODO\n");
  } else {
    /*
      We will only get in here if this packet do NOT accept fragmentation. 
      Because this list is already ordered into latency sequence,
      no need to sort or anything
    */
    SAFE_PRINT("No frag\n");

    uint16_t packetOldSize = packet_get_size(packet_piece);
    uint16_t piggyback_with_ext_size = (piggyback_piece->params & IOTUS_PIGGYBACK_ATTACHMENT_WITH_EXTENDED_SIZE)? 1:0;
    if((get_safe_pdu_for_layer(2) - packetOldSize) >= 
          piggyback_piece->data.size + member_size(iotus_piggyback_t,params) + piggyback_with_ext_size) {
      //This piggyback will fit...
      
      if(stick) {
        SAFE_PRINTF_LOG_INFO("Sticking piggy");
        piggyback_piece->params |= IOTUS_PIGGYBACK_ATTACHMENT_TYPE_FINAL_DEST;
      }
      
      packet_append_last_header(piggyback_piece->data.size, pieces_get_data_pointer(piggyback_piece), packet_piece);
      if(piggyback_with_ext_size) {
        SAFE_PRINTF_LOG_INFO("Extended size");
        packet_append_last_header(member_size(iotus_piggyback_t,extendedSize), &(piggyback_piece->extendedSize), packet_piece);
      }
      packet_append_last_header(member_size(iotus_piggyback_t,params), &(piggyback_piece->params), packet_piece);

      //Operation success
      SAFE_PRINTF_LOG_INFO("Appended. Deleting");
      list_remove(gPiggybackFramesList, piggyback_piece);
      //TODO CALL the CB function of the header
      piggyback_destroy(piggyback_piece);

      //Set this variable to this packet transmission
      packet_set_parameter(packet_piece,PACKET_IOTUS_HDR_HAS_PIGGYBACK);
      return TRUE;
    }
  }
  return FALSE;
}

/*---------------------------------------------------------------------*/
/*
 * Function to append piggyback informations into the tail of the msg (inversed)
 * \param bytesSize The amount of bytes that will be appended.
 * \param byteSeq An array of bytes in its normal sequence
 * \param packet_piece Msg to apply this append
 * \return Packet final size
 */
uint16_t
piggyback_apply(iotus_packet_t *packet_piece) {
  if(!packet_get_parameter(packet_piece, PACKET_PARAMETERS_ALLOW_PIGGYBACK) ||
     !(PACKET_PARAMETERS_IS_NEW_PACKET_SYSTEM & packet_piece->params)) {
    SAFE_PRINTF_LOG_ERROR("Pkt cant piggyback");
    //No piggyback allowed for this packet
    return 0;
  }

  //Look for header pieces that match this packet conditions
  iotus_piggyback_t *h;
  iotus_piggyback_t *nextH;
  

  h = list_head(gPiggybackFramesList);
  if(h == NULL) {
    SAFE_PRINTF_LOG_ERROR("Piggy search NULL");
  }
  while(h != NULL) {
    nextH = list_item_next(h);
    SAFE_PRINTF_LOG_INFO("Piggy search ok");
    //FIXXXX HEREREEEE
    // If next destination matches, inform that this piggyback is only for next
    // If not, inform that this piggyback should stick until final destination
    if(h->finalDestinationNode == packet_get_final_destination(packet_piece)) {
      SAFE_PRINTF_LOG_INFO("Same Final dest");
      insert_piggyback_to_packet(packet_piece, h, TRUE);
    } else if (h->finalDestinationNode == packet_get_final_destination(packet_piece)) {
      /* Different final destination */
      insert_piggyback_to_packet(packet_piece, h, FALSE);
    }
    h = nextH;
  }


  return packet_get_size(packet_piece);
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
  /* else if (IOTUS_RUN_SERVICE == signal){

  } else if (IOTUS_END_SERVICE == signal){

  }*/
}
/*---------------------------------------------------------------------*/