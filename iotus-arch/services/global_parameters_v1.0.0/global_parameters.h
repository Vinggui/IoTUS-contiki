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

#ifndef IOTUS_ARCH_SERVICES_GLOBAL_PARAMETERS_GLOBAL_PARAMETERS_H_
#define IOTUS_ARCH_SERVICES_GLOBAL_PARAMETERS_GLOBAL_PARAMETERS_H_

#include "iotus-core.h"
#include "platform-conf.h"

typedef enum {
  IOTUS_PACKET_PRIORITIZE_ENERGY_EFFICIENCY = 0,
  IOTUS_PACKET_PRIORITIZE_LOW_LATENCY
} packet_prioritization;

extern packet_prioritization iotus_packet_prioritization;
extern uint16_t iotus_radio_max_message;
extern uint8_t iotus_radio_address_size;


void
iotus_signal_handler_global_parameters(iotus_service_signal signal, void *data);

#endif /* IOTUS_ARCH_SERVICES_GLOBAL_PARAMETERS_GLOBAL_PARAMETERS_H_ */

/* The following stuff ends the \defgroup block at the beginning of
   the file: */

/** @} */
