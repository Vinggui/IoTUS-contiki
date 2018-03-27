/**
 * \defgroup decription...
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

#include "lib/memb.h"
#include "lib/mmem.h"
#include "platform-conf.h"
#include "clock.h"
#include "timestamp.h"
#include "nodes.h"


#ifndef IOTUS_RADIO_FULL_ADDRESS
  #error Please define IOTUS_RADIO_FULL_ADDRESS into platform-conf.h
#endif


#ifndef IOTUS_USING_MALLOC
  #error Defining "IOTUS_USING_MALLOC=0|1" into platform-conf.h is necessary. 0 indicates that MMEM allocation will be used instead.
#endif


#define COMMON_STRUCT_PIECES(structName) \
  structName *next;\
  struct mmem data;\
  uint8_t params;\
  uint8_t priority;\
  timestamp_t timeout;\
  void *callbackHandler;\
  iotus_node_t *finalDestinationNode;

#define COMMON_ADDITIONAL_INFO_HEADER(structName) \
  structName *next;\
  struct mmem data;\
  uint8_t type;\
  uint8_t isCopied;


typedef struct generic_piece {
  COMMON_STRUCT_PIECES(struct generic_piece);
} iotus_generic_piece_t;

typedef struct generic_additional_info {
  COMMON_ADDITIONAL_INFO_HEADER(struct generic_additional_info);
} iotus_additional_info_t;


//////////////////////////////////////////
//             Functions                //
//////////////////////////////////////////
uint8_t *
pieces_get_data_pointer(void *piecePointer);

iotus_generic_piece_t *
pieces_malloc(struct memb *m, uint16_t allocSize, const uint8_t *data, uint16_t dataSize);

void
pieces_destroy_additional_info(list_t list, iotus_additional_info_t *item);

void
pieces_clean_additional_info_list(list_t list);

iotus_additional_info_t *
pieces_get_additional_info(list_t list, uint8_t type);

iotus_additional_info_t *
pieces_set_additional_info(list_t list, uint8_t type, uint8_t *data, uint16_t dataSize, Boolean copyIntoBuffer);

Boolean
pieces_destroy(struct memb *m, void *h);


void
pieces_insert_timeout_priority(list_t list, void *item);

#endif /* IOTUS_ARCH_SHARED_UTILS_PIECES_H_ */

/* The following stuff ends the \defgroup block at the beginning of
   the file: */

/** @} */
