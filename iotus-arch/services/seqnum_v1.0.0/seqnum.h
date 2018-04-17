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

uint8_t
seqnum_acquire(iotus_nodes_t *node);

void
iotus_signal_handler_seqnum(iotus_service_signal signal, void *data);

#endif /* IOTUS_ARCH_SERVICES_SEQNUM_SEQNUM_H_ */

/* The following stuff ends the \defgroup block at the beginning of
   the file: */

/** @} */
