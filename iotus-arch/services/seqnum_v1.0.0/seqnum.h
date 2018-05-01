/**
 * \defgroup description...
 *
 * This...
 *
 * @{
 */

/*
 * seqnum
 * .h
 *
 *  Created on: Nov 15, 2017
 *      Author: vinicius
 */

#ifndef IOTUS_ARCH_SERVICES_SEQNUM_SEQNUM_H_
#define IOTUS_ARCH_SERVICES_SEQNUM_SEQNUM_H_

#include "iotus-core.h"

#define SEQNUM_MAX_AGE            20000//msec

Status
seqnum_register(iotus_node_t *node, uint16_t sequence_number);

uint16_t
seqnum_get_last(iotus_node_t *node);

uint8_t
seqnum_acquire(iotus_node_t *node);

void
iotus_signal_handler_seqnum(iotus_service_signal signal, void *data);

#endif /* IOTUS_ARCH_SERVICES_SEQNUM_SEQNUM_H_ */

/* The following stuff ends the \defgroup block at the beginning of
   the file: */

/** @} */
