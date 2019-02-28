
/**
 * \defgroup description...
 *
 * This...
 *
 * @{
 */

/*
 * null.c
 *
 *  Created on: Feb 27, 2018
 *      Author: vinicius
 */
#include <string.h>
#include "addresses.h"
#include "contiki.h"
#include "iotus-api.h"
#include "iotus-core.h"
#include "iotus-netstack.h"
#include "layer-packet-manager.h"
#include "neighbor_discovery.h"



#define DEBUG IOTUS_DONT_PRINT//IOTUS_PRINT_IMMEDIATELY
#define THIS_LOG_FILE_NAME_DESCRITOR "NDisc"
#include "safe-printer.h"


/*
 * Each piece has the first byte as the layer.
 * Second byte as the size of the piece inserted..
*/


uint16_t nd_beacon_period;
uint16_t nd_association_scan_duration;

LIST(gNDPieces);

// static uint8_t gLayersDoingND = 0;
static uint8_t gNDOperations[ND_PKT_MAX_VALUE-1] = {0};
static nd_cb_func gLayersCB[IOTUS_MAX_LAYER_NUM-1] = {NULL};
static uint8_t gMsgPayload[30];
nd_pkt_types ndLastOperation = 0;
iotus_node_t *nd_node_nogotiating = NULL;


/*---------------------------------------------------------------------------*/
void
nd_remove_subscription(iotus_layer_priority layer)
{
  iotus_additional_info_t *h = list_head(gNDPieces);
  while(NULL != h) {
    iotus_additional_info_t *next = list_item_next(h);

    /*
     * The h->type is the operation
     * The first byte is the size of this piece [0]
     * The second byte is the layer of this piece [1]
     */
    uint8_t *payload = pieces_get_data_pointer(h);
    if(payload[1] == layer) {
      pieces_destroy_additional_info(gNDPieces,h);
    }
    h = next;
  }

  uint8_t i=0;
  for(; i<ND_PKT_MAX_VALUE-1; i++) {
    gNDOperations[i] &= ~(1<<layer);
  }
  gLayersCB[layer] = NULL;
}

// /*---------------------------------------------------------------------------*/
void
nd_set_operation_msg(iotus_layer_priority layer,
                     nd_pkt_types operation,
                     uint8_t size,
                     uint8_t* payload)
{
  if(size>0) {
    //First delete older msgs
    iotus_additional_info_t *h = list_head(gNDPieces);
    while(NULL != h) {
      iotus_additional_info_t *next = list_item_next(h);

      uint8_t *payload = pieces_get_data_pointer(h);
      if(payload[1] == layer &&
         h->type == operation) {
        pieces_destroy_additional_info(gNDPieces,h);
      }
      h = next;
    }
    
    //The type indicates which operation is saved there
    uint8_t *data = pieces_set_additional_info_var(gNDPieces, operation, size+2, TRUE);
    if(NULL == data) {
      SAFE_PRINTF_LOG_ERROR("Association piece not assigned");
      return;
    }

    data[0] = size+2;
    data[1] = layer;
    memcpy(data+2, payload, size);
  }
}
// ---------------------------------------------------------------------------
// uint8_t
// nd_get_assigned_layer(iotus_layer_priority layer)
// {
//   return (gLayersDoingND & (1<<layer));
// }
/*---------------------------------------------------------------------------*/
void
nd_set_layer_operations(iotus_layer_priority layer, nd_pkt_types op)
{
  gNDOperations[op] |= (1<<layer);
}
/*---------------------------------------------------------------------------*/
void
nd_set_layer_cb(iotus_layer_priority layer, nd_cb_func cb)
{
  gLayersCB[layer] = cb;
}
/*---------------------------------------------------------------------------*/
uint8_t
nd_get_layer_operations(nd_pkt_types op)
{
  return gNDOperations[op];
}

/*---------------------------------------------------------------------------*/
static void
clean_pieces(nd_pkt_types type)
{
  iotus_additional_info_t *h = list_head(gNDPieces);
  while(NULL != h) {
    iotus_additional_info_t *next = list_item_next(h);

    /*
     * The h->type is the operation
     * The first byte is the size of this piece [0]
     * The second byte is the layer of this piece [1]
     */
    uint8_t *payload = pieces_get_data_pointer(h);
    if(h->type == type) {
      pieces_destroy_additional_info(gNDPieces,h);
    }
    h = next;
  }
}
/*---------------------------------------------------------------------*/
uint8_t *
nd_build_packet_type(nd_pkt_types operation) {
  iotus_additional_info_t *h;
  uint8_t totalSize = 1;

  //First byte is the total size of this msg
  uint8_t *bufferPointer = gMsgPayload + 1;

  h = list_head(gNDPieces);
  while(NULL != h) {
    if(h->type == operation) {
      uint8_t *ptr = pieces_get_data_pointer(h);
      memcpy(bufferPointer, ptr, ptr[0]);
      bufferPointer += ptr[0];
      totalSize += ptr[0];
    }
    h = list_item_next(h);
  }

  //Fix the total size now...
  gMsgPayload[0] = totalSize;
#if ND_STATIC_REQUESTS == 0
  if(operation != ND_PKT_BEACONS) {
    clean_pieces(operation);
  }
#endif
  ndLastOperation = operation;
  return gMsgPayload;
}

/*---------------------------------------------------------------------*/
/*
 * Expects that the layer using this system can recognize what type of packet it tried to send and receive.
*/
void
nd_unwrap_msg(nd_pkt_types type, iotus_packet_t *packet)
{
  uint8_t pieceSize = 0;
  uint8_t layer = 0;
  // uint8_t size = packet_get_payload_size(packet);
  uint8_t *ptr = packet_get_payload_data(packet);
  uint8_t totalSize = ptr[0]-1;
  ptr++;//first byte is the total length

  uint8_t lastMsgBuff[30] = {0};
  uint8_t lastSize = 0;
  uint8_t lastLayer = 0;


  /*
   * They layer calling this function HAS to be the last to have it`s CB called,
   * otherwise synch can be broken.
   */

  while(totalSize > 0) {
    pieceSize = ptr[0];
    layer = ptr[1];
    // printf("era %p layer %u\n", ptr, layer);
    if(iotus_get_layer_assigned_for(IOTUS_CHORE_NEIGHBOR_DISCOVERY) == layer) {
      //skip for later
      memcpy(lastMsgBuff, ptr+2, pieceSize);
      lastSize = pieceSize;
      lastLayer = layer;
      // printf("tava %u %p\n", skippedLayerPtr[1], skippedLayerPtr);

    } else if(gNDOperations[type] & (1<<layer) ||
              gLayersCB[layer] != NULL) {
      /*
       * This layer is really operating here
       * Call this cb function
       */
      gLayersCB[layer](packet, type, pieceSize-2, ptr+2);
    } else {
      SAFE_PRINTF_LOG_INFO("Layer not assigned");
    }
    ptr += pieceSize;
    totalSize -= pieceSize;
  }
// printf("cheguei %u %p %p\n", skippedLayerPtr[1], skippedLayerPtr, gLayersCB[skippedLayerPtr[1]]);//Ta dando 0!
  if(lastSize > 0) {
    gLayersCB[lastLayer](packet, type, lastSize, lastMsgBuff);
  }
}

/*---------------------------------------------------------------------*/
/*
 * Default function required from IoTUS, to initialize, run and finish this service
 */
void iotus_signal_handler_neighbor_discovery(iotus_service_signal signal, void *data)
{
  if(IOTUS_START_SERVICE == signal) {
    SAFE_PRINT("\tService Neighbor D.\n");
    // sprintf((char *)private_nd_control, "### tira ###");
    list_init(gNDPieces);
  }
  // else if (IOTUS_RUN_SERVICE == signal) {
    
  // } else if (IOTUS_END_SERVICE == signal) {
  //    pieces_clean_additional_info_list(gNDPieces);
  // }
}