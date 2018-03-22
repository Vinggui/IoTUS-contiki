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

#include "platform-conf.h"
#include "clock.h"
#include "timestamp.h"
#include "nodes.h"



#define COMMON_STRUCT_PIECES(structName) \
  structName *next;\
  timestamp_t timeout;\
  uint8_t params;\
  uint8_t priority;\
  void *callbackHandler;\
  iotus_node_t *finalDestinationNode;\
  uint16_t dataSize;\
  uint8_t *data

#define COMMON_ADDITIONAL_INFO_HEADER(structName) \
  structName *next;\
  uint8_t type;\
  uint16_t dataSize;\
  uint8_t *data


typedef struct generic_piece {
  COMMON_STRUCT_PIECES(struct generic_piece);
} iotus_generic_piece_t;

typedef struct generic_additional_info {
  COMMON_ADDITIONAL_INFO_HEADER(struct generic_additional_info);
} iotus_additional_info_t;

iotus_generic_piece_t *
pieces_malloc(uint16_t dataSize, uint16_t pieceSize);


Boolean
pieces_set_data(void *piecePointer, const uint8_t *data);

iotus_additional_info_t *
pieces_get_additional_info_by_type(list_t list, uint8_t type);

void
pieces_insert_timeout_priority(list_t list, void *item);

#endif /* IOTUS_ARCH_SHARED_UTILS_PIECES_H_ */

/* The following stuff ends the \defgroup block at the beginning of
   the file: */

/** @} */
