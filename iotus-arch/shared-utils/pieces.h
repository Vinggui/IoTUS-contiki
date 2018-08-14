/**
 * \defgroup description...
 *
 * This...
 *
 * @{
 */

/*
 * pieces.h
 *
 *  Created on: Nov 15, 2017
 *      Author: vinicius
 */

#ifndef IOTUS_ARCH_SHARED_UTILS_PIECES_H_
#define IOTUS_ARCH_SHARED_UTILS_PIECES_H_  

#include "clock.h"
#include "lib/memb.h"
#include "lib/mmem.h"
#include "chores.h"
#include "platform-conf.h"
#include "nodes.h"
#include "timestamp.h"


#ifndef IOTUS_RADIO_FULL_ADDRESS
  #error Please define IOTUS_RADIO_FULL_ADDRESS into platform-conf.h
#endif


#ifndef IOTUS_USING_MALLOC
  #error Defining "IOTUS_USING_MALLOC=0|1" into platform-conf.h is necessary. 0 indicates that MMEM allocation will be used instead.
#endif

typedef struct generic_piece {
  struct generic_piece *next;
  struct mmem data;
  iotus_layer_priority priority;
  iotus_node_t *finalDestinationNode;
  timestamp_t timeout;
  uint8_t params;
} iotus_generic_piece_t;

typedef struct generic_additional_info {
  struct generic_additional_info *next;
  struct mmem data;
  uint8_t type;
  uint8_t isBuffered;
} iotus_additional_info_t;

///////////////////////////////////////////
//             MACROS                    //
///////////////////////////////////////////
#define pieces_get_data_size(piecePointer)    (((iotus_additional_info_t *)piecePointer)->data.size)
#define pieces_get_data_pointer(piecePointer) ((uint8_t *)(((iotus_additional_info_t *)piecePointer)->data.ptr))

//////////////////////////////////////////
//             Functions                //
//////////////////////////////////////////

iotus_generic_piece_t *
pieces_malloc(struct memb *m, uint16_t allocSize, const uint8_t *data, uint16_t dataSize);

void
pieces_destroy_additional_info(list_t list, iotus_additional_info_t *item);

void
pieces_clean_additional_info_list(list_t list);

iotus_additional_info_t *
pieces_get_additional_info(list_t list, uint8_t type);

void *
pieces_modify_additional_info_var(list_t list, uint8_t type,
                                  uint16_t varSize,
                                  Boolean createBuffer);

void *
pieces_get_additional_info_var(list_t list, uint8_t type);

Boolean
pieces_destroy(struct memb *m, void *h);

void
pieces_insert_timeout_priority(list_t list, void *item);

#endif /* IOTUS_ARCH_SHARED_UTILS_PIECES_H_ */

/* The following stuff ends the \defgroup block at the beginning of
   the file: */

/** @} */
