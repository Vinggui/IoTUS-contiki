/**
 * \defgroup description...
 *
 * This...
 *
 * @{
 */

/*
 * phase_recorder
 * .h
 *
 *  Created on: Nov 15, 2017
 *      Author: vinicius
 */

#ifndef IOTUS_ARCH_SERVICES_PHASE_RECORDER_PHASE_RECORDER_H_
#define IOTUS_ARCH_SERVICES_PHASE_RECORDER_PHASE_RECORDER_H_

#include "iotus-core.h"
#include "sys/rtimer.h"


typedef enum {
  PHASE_UNKNOWN,
  PHASE_SEND_NOW,
  PHASE_DEFERRED,
} phase_recorder_status_t;


void
phase_recorder_update(iotus_node_t *neighbor, rtimer_clock_t time,
             int mac_status);

phase_recorder_status_t
phase_recorder_wait(const iotus_node_t *neighbor, rtimer_clock_t cycle_time,
           rtimer_clock_t guard_time, iotus_packet_t *packet);

void
iotus_signal_handler_phase_recorder(iotus_service_signal signal, void *data);

#endif /* IOTUS_ARCH_SERVICES_PHASE_RECORDER_PHASE_RECORDER_H_ */

/* The following stuff ends the \defgroup block at the beginning of
   the file: */

/** @} */
