
/**
 * \defgroup decription...
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
#include "list.h"
#include "iotus-core.h"
#include "global-parameters.h"
#include "packet.h"
#include "pieces.h"
#include "piggyback.h"
#include "platform-conf.h"
#include "packet-defs.h"
#include "timestamp.h"

#define DEBUG 1
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else /* DEBUG */
#define PRINTF(...)
#endif /* DEBUG */


LIST(gPiggybackFramesList);


/*---------------------------------------------------------------------*/
/*
 * 
 * \param 
 * \return Packet final size
 */
void
piggyback_delete_piece(Iotus_piggyback_t *piece) {
  if(piece->dataSize > 0) {
    free(piece->data);
  }
  free(piece);
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
Iotus_piggyback_t *
piggyback_create_piece(uint16_t headerSize, const uint8_t* headerData,
    uint8_t type, Iotus_node *destinationNode, int32_t timeout)
{
  Iotus_piggyback_t *newPiece = (Iotus_piggyback_t *)pieces_malloc(headerSize, sizeof(Iotus_piggyback_t));
  if(headerSize) {
    if(!pieces_set_data(newPiece, headerData)) {
      //Alloc failed, cancel operation
      piggyback_delete_piece(newPiece);
      return NULL;
    }
  }

  timestamp_mark(&(newPiece->timeout), timeout);

  uint8_t params;
  /* Encode parameters */

  //set_piece_parameters(newPiece, params);
  //set_piggyback_piece_type(newPiece, type);

  //Set the destination node
  newPiece->finalDestinationNode = destinationNode;

  //Link the header into the list, sorting insertion
  pieces_insert_timeout_priority(gPiggybackFramesList,newPiece);
  PRINTF("Piggyback frame created.\n");
  return newPiece;
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
  if(!packet_verify_parameter(packet_piece, PACKET_PARAMETERS_ALLOW_PIGGYBACK)) {
    PRINTF("Packet doens't allow piggyback.\n");
    //No piggyback allowed for this packet
    return 0;
  }

  //Look for header pieces that match this packet conditions
  Iotus_piggyback_t *h;
  Iotus_piggyback_t *nextH;
  uint16_t packetOldSize = packet_get_size(packet_piece);
  
  h = list_head(gPiggybackFramesList);
  if(h == NULL) {
    PRINTF("Piggy search NULL.\n");
  }
  while(h != NULL) {
    nextH = list_item_next(h);
    PRINTF("Piggy search ok...\n");
    //FIXXXX HEREREEEE
    // If next destination matches, inform that this piggyback is only for next
    // If not, inform that this piggyback should stick until final destination
    if(h->finalDestinationNode == packet_get_final_destination(packet_piece)) {
      PRINTF("Piggy same final destination...\n");
      if(!packet_verify_parameter(packet_piece, PACKET_PARAMETERS_ALLOW_FRAGMENTATION)) {
        /*
          We will only get in here if this packet do NOT accept fragmentation. 
          Because this list is already ordered into latency sequence,
          no need to sort or anything
        */
       
        PRINTF("Piggy no fragmentation...\n");
        if((iotus_radio_max_message - packetOldSize) >= h->dataSize) {
          //This will fit...
          uint16_t newSize = packet_append_byte_header(h->dataSize, h->data, packet_piece);

          PRINTF("Piggy appended. Deleting frame..\n");
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
    PRINTF("\tService Piggyback\n");
    list_init(gPiggybackFramesList);
  }
  /* else if (IOTUS_RUN_SERVICE == signal){

  } else if (IOTUS_END_SERVICE == signal){

  }*/
}
/*---------------------------------------------------------------------*/