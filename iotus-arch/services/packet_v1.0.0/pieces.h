/**
 * \defgroup decription...
 *
 * This...
 *
 * @{
 */

/*
 * packet.h
 *
 *  Created on: Nov 15, 2017
 *      Author: vinicius
 */

#ifndef IOTUS_ARCH_SERVICES_PACKET_PIECES_H_
#define IOTUS_ARCH_SERVICES_PACKET_PIECES_H_  

#include "platform-conf.h"
#include "clock.h"



#define COMMON_STRUCT_PIECES(structName) \
  structName *next;\
  uint8_t params;\
  unsigned long timeout_seconds;\
  clock_time_t timeout;\
  uint8_t priority;\
  void *callbackHandler;\
  uint16_t dataSize;\
  uint8_t *data

void*
pieces_malloc(uint16_t dataSize, uint16_t pieceSize);


uint8_t
pieces_set_data(void *piecePointer, const uint8_t *data);



#endif /* IOTUS_ARCH_SERVICES_PACKET_PIECES_H_ */

/* The following stuff ends the \defgroup block at the beginning of
   the file: */

/** @} */
