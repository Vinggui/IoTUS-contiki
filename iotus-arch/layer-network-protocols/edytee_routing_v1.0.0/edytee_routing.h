/**
 * \defgroup decription...
 *
 * This...
 *
 * @{
 */

/*
 * edytee-routing.h
 *
 *  Created on: Nov 18, 2017
 *      Author: vinicius
 */

#ifndef IOTUS_ARCH_ROUTING_LAYER_PROTOCOLS_EDYTEE_ROUTING_V1_0_0_EDYTEE_ROUTING_H_
#define IOTUS_ARCH_ROUTING_LAYER_PROTOCOLS_EDYTEE_ROUTING_V1_0_0_EDYTEE_ROUTING_H_

#include "contiki.h"

typedef enum {
  EDYTEE_COMMAND_TYPE_DATA,
  EDYTEE_COMMAND_TYPE_COMMAND_DAO,
  EDYTEE_COMMAND_TYPE_COMMAND_DAO_ACK,
  EDYTEE_COMMAND_TYPE_COMMAND_DAO_TO_SINK,
  EDYTEE_COMMAND_TYPE_BROADCAST,

  EDYTEE_COMMAND_TYPE_MAX
} edytee_net_commands;

#define EDYTEE_RPL_DAOACK_DELAY         1//sec

PROCESS_NAME(edytee_routing_process);
extern const struct iotus_network_protocol_struct edytee_routing_protocol;

#endif /* IOTUS_ARCH_ROUTING_LAYER_PROTOCOLS_EDYTEE_ROUTING_V1_0_0_EDYTEE_ROUTING_H_ */

/* The following stuff ends the \defgroup block at the beginning of
   the file: */

/** @} */
