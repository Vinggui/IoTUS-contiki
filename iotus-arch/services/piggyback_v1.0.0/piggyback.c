
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
#include "layer-packet-manager.h"
#include "lib/memb.h"
#include "list.h"
#include "iotus-api.h"
#include "iotus-netstack.h"
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

// #define PIGGYBACK_MAX_ATTACHED_PIECES     ((1<<(PIGGYBACK_MAX_ATTACHED_PIECES_POWER)) -1)
#define PIGGYBACK_MAX_ATTACHED_PIECES     3

MEMB(iotus_piggyback_mem, iotus_piggyback_t, IOTUS_PIGGYBACK_LIST_SIZE);
LIST(gPiggybackFramesList);
LIST(gPiggybackFramesInsertedList);

static struct ctimer piggyback_timeout_ctimer;
static piggy_cb_func gLayersCB[IOTUS_MAX_LAYER_NUM-1] = {NULL};

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
  list_remove(gPiggybackFramesInsertedList, piece);

  return pieces_destroy(&iotus_piggyback_mem, piece);
}

/*---------------------------------------------------------------------*/
/**
 * \brief         Destroy the list of additional information that a piece may contain.
 * \param packet  The pointer to the packet being confirmed.
 */
void
piggyback_confirm_sent(iotus_packet_t *packet, uint8_t status)
{
  if(packet == NULL) {
    return;
  }

  if(status != MAC_TX_OK) {
    return;
  }
  //TODO Send confirmation to layer owner... They have to first register a function for that

  // printf("piggy clean1 %u %u %u\n", list_length(gPiggybackFramesInsertedList), list_length(gPiggybackFramesList), memb_numfree(&iotus_piggyback_mem));
  iotus_piggyback_t *h;
  iotus_piggyback_t *hOld;
  // while(NULL != (h =list_pop(list))) {
  h = list_head(gPiggybackFramesInsertedList);
  hOld = h;
  for(; h!=NULL; h=list_item_next(h)) {
    // printf("confirm %u read %u\n", packet->pktID, h->pktID);
    if(h->pktID == packet->pktID) {
      piggyback_destroy(h);
      h = hOld;
    }
    hOld = h;
  }
  // printf("piggy clean2 %u %u %u\n", list_length(gPiggybackFramesInsertedList), list_length(gPiggybackFramesList), memb_numfree(&iotus_piggyback_mem));
  SAFE_PRINTF_LOG_INFO("Piggy list clean");
}

/*---------------------------------------------------------------------*/
/*
 */
static void
piggyback_timeout_handler(void *ptr) {
  //Get the first of the list
  iotus_piggyback_t *h;
  h = list_head(gPiggybackFramesList);

  if(h == NULL) {
    return;
  }

  iotus_packet_t *packet = iotus_initiate_packet(
                            pieces_get_data_size(h),
                            pieces_get_data_pointer(h),
                            PACKET_PARAMETERS_WAIT_FOR_ACK,
                            h->priority,
                            5000,
                            h->finalDestinationNode,
                            NULL);

  if(NULL == packet) {
    SAFE_PRINTF_LOG_INFO("Packet failed");
    return;
  }
  piggyback_destroy(h);

  SAFE_PRINTF_LOG_INFO("Piggy Timeout %u \n", packet->pktID);
  iotus_netstack_return status = active_transport_protocol->build_to_send(packet);
  // if (!(MAC_TX_OK == status ||
  //     MAC_TX_DEFERRED == status)) {
  if (MAC_TX_DEFERRED != status) {
    // printf("Packet piggy del %u\n", packet->pktID);
    packet_destroy(packet);
  }

  update_piggy_timeout_timer();
}

/*---------------------------------------------------------------------*/
/*
 */
void
piggyback_unwrap_payload(iotus_packet_t *packet) {
    uint8_t piggyHeader, numberOfPieces, pieceHdr;
    uint8_t piggyLayer, extPieceHdr, removedPieces;
    uint8_t oldPosReader;
    uint16_t piggyDataSize;

  if(packet_get_parameter(packet, PACKET_PARAMETERS_IS_NEW_PACKET_SYSTEM)) {
    uint16_t posReader = 0;

    //Get the first byte of the pigyback payload
    piggyHeader = packet_read_byte_backward(posReader++, packet);
    // packet_unwrap_appended_byte(packet, &piggyHeader, 1);

    numberOfPieces = piggyHeader & PIGGYBACK_MAX_ATTACHED_PIECES;

    uint8_t i = 0;
    removedPieces = 0;
    for(; i < numberOfPieces; i++) {
      piggyDataSize = 0;
      extPieceHdr = 0;
      oldPosReader = posReader;

      pieceHdr = packet_read_byte_backward(posReader++, packet);

      piggyDataSize = IOTUS_PIGGYBACK_ATTACHMENT_SIZE_MASK & pieceHdr;

      if(IOTUS_PIGGYBACK_ATTACHMENT_WITH_EXTENDED_SIZE & pieceHdr) {
        extPieceHdr = 1;
        piggyDataSize = piggyDataSize<<8;
        piggyDataSize |= packet_read_byte_backward(posReader++, packet);
      }
 
      //Radio layer never uses piggyback, so we ignore it by saying DATA LINK is 0
      piggyLayer = ((pieceHdr & IOTUS_PIGGYBACK_LAYER)>>6) + 1;

      //Now we need to verify if this node should unwraped
      if(!(piggyHeader & IOTUS_PIGGYBACK_GENERAL_HDR_IS_FINAL_DEST) &&
         pieceHdr & IOTUS_PIGGYBACK_ATTACHMENT_TYPE_FINAL_DEST) {
        /*
         * This is a final destination piggyback piece, leave it here..
         * since this is not the destination node.
         */
        posReader += piggyDataSize;
        continue;
      }

      //Otherwise, proceed with this piggyback piece and deliver...
      uint8_t tempBuffForPiece[piggyDataSize+2];

      uint16_t correctedPos = packet_get_payload_size(packet)-posReader-piggyDataSize;

      //+1 to leave space to correct direction
      packet_extract_data_bytes(tempBuffForPiece, correctedPos, piggyDataSize+extPieceHdr+1, packet);

      
      //Fix direction of bytes
      uint8_t i, olderByte;
      for(i=0; i<piggyDataSize/2; i++) {
        olderByte = tempBuffForPiece[i];
        tempBuffForPiece[i] = tempBuffForPiece[piggyDataSize-i-1];
        tempBuffForPiece[piggyDataSize-i-1] = olderByte;
      }
      tempBuffForPiece[piggyDataSize] = '\0';
      tempBuffForPiece[piggyDataSize+1] = '\0';

      if(gLayersCB[piggyLayer] != NULL) {
        //There is a cb and we will call it
        gLayersCB[piggyLayer](packet, piggyDataSize, tempBuffForPiece);
      }

      //As we extract this from the packet, then we has to return to original value
      posReader = oldPosReader;
      removedPieces++;
    }

    numberOfPieces -= removedPieces;
    if(numberOfPieces > 0) {
      packet_set_parameter(packet, PACKET_PARAMETERS_ALREADY_WITH_PIGGYBACK);
    } else {
      packet_clear_parameter(packet, PACKET_PARAMETERS_ALREADY_WITH_PIGGYBACK);
    }


    numberOfPieces &= IOTUS_PIGGYBACK_GENERAL_HDR_NUMBER_PIECES;
    //Indicates that this packet has some piggyback already
    packet_set_byte_backward(numberOfPieces, 0, packet);
  }
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
    iotus_layer_priority targetLayer, iotus_node_t *destinationNode, int16_t timeout)
{
  if(piggyPayloadSize > PIGGYBACK_MAX_FRAME_SIZE) {
    SAFE_PRINTF_LOG_ERROR("Piggy hdr too large");
    return NULL;
  }
  // printf("piggy create1 %u %u\n", list_length(gPiggybackFramesList), memb_numfree(&iotus_piggyback_mem));
  iotus_piggyback_t *newPiece = (iotus_piggyback_t *)pieces_malloc(
                                                        &iotus_piggyback_mem,
                                                        sizeof(iotus_piggyback_t),
                                                        NULL,
                                                        piggyPayloadSize);
  
  if(NULL == newPiece) {
    printf("Alloc failed.\n");
    // SAFE_PRINTF_LOG_ERROR("Alloc failed.");
    return NULL;
  }
  memcpy(pieces_get_data_pointer(newPiece),piggyPayload,piggyPayloadSize);
  
  timestamp_delay(&(newPiece->timeout), timeout);

  //Radio layer never uses piggyback, so we ignore it by saying DATA LINK is 0
  newPiece->priority = (targetLayer-1);

  uint8_t params = IOTUS_PIGGYBACK_LAYER & (newPiece->priority<<6);
  /* Encode parameters 
   * 0b11000000 - 2 bits indicates to which layer this frame is supposed to be sent
   * 0b00100000 - 1 bit indicate if this frame is to the next node or stick to final destination
   * 0b00010000 - 1 bit indicate if this frame has a second byte to describe header size
   * 0b00001111 - 4 bits indicate the size of the header (if smaller than 16bytes) or the
   *                most significant byte of the size (if bigger).
   */
  if(piggyPayloadSize > PIGGYBACK_SINGLE_HEADER_FRAME) {
    params |= (uint8_t)(piggyPayloadSize>>8) & IOTUS_PIGGYBACK_ATTACHMENT_SIZE_MASK;
    params |= IOTUS_PIGGYBACK_ATTACHMENT_WITH_EXTENDED_SIZE;
    newPiece->extendedSize = (uint8_t)(piggyPayloadSize & 0x00FF);
  } else {
    params |= (uint8_t)(piggyPayloadSize & 0x000F);
  }
  newPiece->params = params;

  //Set the destination node
  newPiece->finalDestinationNode = destinationNode;

  //Link the header into the list, sorting insertion
  pieces_insert_timeout_priority(gPiggybackFramesList,newPiece);
  // printf("piggy create2 %u %u\n", list_length(gPiggybackFramesList), memb_numfree(&iotus_piggyback_mem));
  update_piggy_timeout_timer();

  SAFE_PRINTF_LOG_INFO("Frame created\n");
  return newPiece;
}


/*---------------------------------------------------------------------*/
static Boolean
insert_piggyback_to_packet(iotus_packet_t *packet_piece,
                    iotus_piggyback_t *piggyback_piece, Boolean stick,
                    uint16_t availableSpace,
                    Boolean hasOneFreeByte)
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

  if(hasOneFreeByte) {
    hasOneFreeByte = 1;
  }

  uint16_t packetOldSize = packet_get_size(packet_piece);
  uint16_t piggyback_with_ext_size;
  
  piggyback_with_ext_size = (piggyback_piece->params & IOTUS_PIGGYBACK_ATTACHMENT_WITH_EXTENDED_SIZE);

  if((availableSpace - packetOldSize) >= (pieces_get_data_size(piggyback_piece) -
                                          hasOneFreeByte +
                                          member_size(iotus_piggyback_t,params) +
                                          member_size(iotus_piggyback_t,extendedSize))) {
    //This piggyback will fit...
    
    if(stick) {
      SAFE_PRINTF_LOG_INFO("Sticking piggy");
      piggyback_piece->params |= IOTUS_PIGGYBACK_ATTACHMENT_TYPE_FINAL_DEST;
    }

    //Insert the header at the end of the data buffer of this piggyback
    uint8_t tempBuffer[pieces_get_data_size(piggyback_piece) +
                       member_size(iotus_piggyback_t,params) +
                       member_size(iotus_piggyback_t,extendedSize) - 
                       hasOneFreeByte];
    uint8_t *tempBuffPointer = tempBuffer;

    *tempBuffPointer = piggyback_piece->params;
    tempBuffPointer++;
    if(piggyback_with_ext_size) {
      *tempBuffPointer = piggyback_piece->extendedSize;
      tempBuffPointer++;
    }
    memcpy(tempBuffPointer, pieces_get_data_pointer(piggyback_piece), pieces_get_data_size(piggyback_piece));

    if(hasOneFreeByte) {
      //Use this free space and reduce buffer creation for piggybacks
      uint8_t lastByte = pieces_get_data_pointer(piggyback_piece)[pieces_get_data_size(piggyback_piece)-1];
      packet_set_byte_backward(lastByte, 0, packet_piece);
    }

    if(0 == packet_append_last_header((uint16_t)(tempBuffPointer-tempBuffer +
                                                 pieces_get_data_size(piggyback_piece) -
                                                 hasOneFreeByte),
                                      tempBuffer,
                                      packet_piece)) {
      SAFE_PRINTF_LOG_ERROR("Append");
      return FALSE;
    }

    //Operation success
    SAFE_PRINTF_LOG_INFO("Appended");
    piggyback_piece->pktID = packet_piece->pktID;
    list_remove(gPiggybackFramesList, piggyback_piece);
    list_push(gPiggybackFramesInsertedList, piggyback_piece);

    // printf("piggy insert1 %u %u %u\n", list_length(gPiggybackFramesInsertedList), list_length(gPiggybackFramesList), memb_numfree(&iotus_piggyback_mem));
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
  uint8_t piggyFinalGeneralHdr = 0;
  uint16_t packetOldSize;
  Boolean hasOneFreeByte = FALSE;
  //Save the original size of this pkt
  packetOldSize = packet_get_size(packet_piece);

  //Check if this packet already had another pigyback inserted, like rexmit
  if(packet_get_parameter(packet_piece, PACKET_PARAMETERS_ALREADY_WITH_PIGGYBACK)) {
    //Get the first byte of the pigyback payload
    piggyFinalGeneralHdr = packet_read_byte_backward(0, packet_piece);

    hasOneFreeByte = TRUE;
    //update oldsize
    packetOldSize--;
  }


  h = list_head(gPiggybackFramesList);
  if(h == NULL) {
    SAFE_PRINTF_LOG_ERROR("Piggy search NULL");
  }
  while(h != NULL) {
    if(piggyFinalGeneralHdr >= PIGGYBACK_MAX_ATTACHED_PIECES) {
      break;
    }
    nextH = list_item_next(h);
    SAFE_PRINTF_LOG_INFO("Piggy search ok");
    Boolean toFinalDestination = (h->finalDestinationNode == packet_get_final_destination(packet_piece));
    if(toFinalDestination ||
       h->finalDestinationNode == packet_get_next_destination(packet_piece)) {

      if(TRUE == insert_piggyback_to_packet(packet_piece,
                                             h,
                                             toFinalDestination,
                                             availableSpace,
                                             hasOneFreeByte)) {
        piggyFinalGeneralHdr++;
        hasOneFreeByte = FALSE;
        //TODO remove this break to add more pieces...
        // break;
      }
    }
    h = nextH;
  }
// leds_toggle(ALL_LEDS);

  if(piggyFinalGeneralHdr > 0) {
    packet_set_parameter(packet_piece, PACKET_PARAMETERS_ALREADY_WITH_PIGGYBACK);
  }

  //So far, piggyFinalGeneralHdr has the number of pieces attached
  piggyFinalGeneralHdr &= IOTUS_PIGGYBACK_GENERAL_HDR_NUMBER_PIECES;

  //Verify if this is our last transmission
  if(packet_get_next_destination(packet_piece) == packet_get_final_destination(packet_piece)){
    piggyFinalGeneralHdr |= IOTUS_PIGGYBACK_GENERAL_HDR_IS_FINAL_DEST;
  }

  if(hasOneFreeByte == TRUE) {
    packet_set_byte_backward(piggyFinalGeneralHdr, 0, packet_piece);
  } else {
// leds_on(LEDS_RED);
    if(0 == packet_append_last_header(1,
                                      &piggyFinalGeneralHdr,
                                      packet_piece)) {
      SAFE_PRINTF_LOG_ERROR("Append");
    }
// leds_off(LEDS_RED);
  }

  return packet_get_size(packet_piece)-packetOldSize;
}

/*---------------------------------------------------------------------------*/
void
piggyback_subscribe(iotus_layer_priority layer, piggy_cb_func *cbFunc)
{
  gLayersCB[layer] = cbFunc;
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
    list_init(gPiggybackFramesInsertedList);
  }
  //Verify if any piggyback frame is expiring
  // piggyback_check_timeout();

  /*  else if (IOTUS_END_SERVICE == signal){

  }*/
}
/*---------------------------------------------------------------------*/