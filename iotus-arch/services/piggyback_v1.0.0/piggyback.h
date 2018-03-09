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

#ifndef IOTUS_ARCH_SERVICES_PIGGYBACK_PIGGYBACK_H_
#define IOTUS_ARCH_SERVICES_PIGGYBACK_PIGGYBACK_H_

#include "iotus-core.h"
#include "platform-conf.h"
#include "timestamp.h"


void
piggyback_delete_piece(void *piecePointer);

void *
piggyback_create_piece(uint16_t headerSize, const uint8_t* headerData,
    uint8_t type, const uint8_t *nextDestination);

uint16_t
piggyback_apply(void *msg_piece);

void
iotus_signal_handler_piggyback(iotus_service_signal signal, void *data);

#endif /* IOTUS_ARCH_SERVICES_PIGGYBACK_PIGGYBACK_H_ */

/* The following stuff ends the \defgroup block at the beginning of
   the file: */

/** @} */
