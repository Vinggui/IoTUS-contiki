
/**
 * \defgroup decription...
 *
 * This...
 *
 * @{
 */

/*
 * global_parameters.c
 *
 *  Created on: Feb 27, 2018
 *      Author: vinicius
 */

#include <stdio.h>
#include <stdlib.h>
#include "list.h"
#include "iotus-core.h"
#include "global_parameters.h"
#include "packet.h"
#include "pieces.h"
#include "platform-conf.h"
#include "packet-defs.h"

#define DEBUG 1
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else /* DEBUG */
#define PRINTF(...)
#endif /* DEBUG */


struct piggyback_piece {
  COMMON_STRUCT_PIECES(struct piggyback_piece);
  uint8_t type;
};


LIST(gPiggybackFramesList);

/***********************************************************************/

void
piggyback_delete_piece(void *piecePointer) {
  struct piggyback_piece *piece = (struct piggyback_piece *)piecePointer;
  if(piece->dataSize > 0) {
    free(piece->data);
  }
  free(piecePointer);
}

/*
 * Function to create header pieces to be attached
 * @Params headerSize Size of the header in bytes
 * @Params headerData The information to be attached
 * @Params isBroadcastable Indicate if this packet will be able lto be broadcast
 * @Result Pointer to the struct with the final packet to be transmited (read by framer layer).
 */
void *
piggyback_create_piece(uint16_t headerSize, const uint8_t* headerData,
    uint8_t type, const uint8_t *nextDestination)
{
  void *newHeader = pieces_malloc(headerSize, sizeof(struct piggyback_piece));
  if(headerSize) {
    if(!pieces_set_data(newHeader, headerData)) {
      //Alloc failed, cancel operation
      piggyback_delete_piece(newHeader);
      return NULL;
    }
  }

  uint8_t params;
  /* Encode parameters */

  //set_piece_parameters(newHeader, params);
  //set_piggyback_piece_type(newHeader, type);

  //Link the header into the list
  list_push(gPiggybackFramesList, newHeader);
  return newHeader;
}


/*
 * Function to append piggyback informations into the tail of the msg (inversed)
 * @Params bytesSize The amount of bytes that will be appended.
 * @Params byteSeq An array of bytes in its normal sequence
 * @Params msg_piece Msg to apply this append
 * @Result Packet final size
 */
uint16_t
piggyback_apply(void *msg_piece) {
  if(!packet_verify_parameter(msg_piece, PACKET_PARAMETERS_ALLOW_PIGGYBACK)) {
    //No piggyback allowed for this packet
    return 0;
  }
  //Look for header pieces that match this packet conditions
  struct piggyback_piece *h;
  struct piggyback_piece *nextH;
  uint16_t packetOldSize = packet_get_size(msg_piece);
  

  h = list_head(gPiggybackFramesList);
  while(h != NULL) {
    nextH = list_item_next(h);
    //if(COMPARE DESTINATION ADDRESS)
    if(!packet_verify_parameter(msg_piece, PACKET_PARAMETERS_ALLOW_FRAGMENTATION)) {
      /*
        We will only get in here if this packet do NOT accept fragmentation. 
        Because this list is already ordered into latency sequence,
        no need to sort or anything
      */
      if((iotus_radio_max_message - packetOldSize) >= h->dataSize) {
        //This will fit...
        uint16_t newSize = packet_append_byte_header(h->dataSize, h->data, msg_piece);
        if(newSize != packetOldSize) {
          //Operation success
          list_remove(gPiggybackFramesList, h);
          //TODO CALL the CB function of the header
          piggyback_delete_piece(h);
        }
      }
    } else {
    //TODO This packet allows fragmentation...
      PRINTF("Packet piggyback with fragmentation not done... TODO");
    }

    h = nextH;
  }

  return packet_get_size(msg_piece);
}

/*
 * Default function required from IoTUS, to initialize, run and finish this service
 */
void
iotus_signal_handler_global_parameters(iotus_service_signal signal, void *data)
{
  if(IOTUS_START_SERVICE == signal) {
    PRINTF("\tService Piggyback\n");
    list_init(gPiggybackFramesList);
  }
  /* else if (IOTUS_RUN_SERVICE == signal){

  } else if (IOTUS_END_SERVICE == signal){

  }*/
}