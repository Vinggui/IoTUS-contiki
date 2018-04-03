/**
 * \defgroup description...
 *
 * This...
 *
 * @{
 */

/*
 * default-chores.h
 *
 *  Created on: Nov 15, 2017
 *      Author: vinicius
 */

#ifndef IOTUS_ARCH_SHARED_UTILS_DEFAULT_CHORES_H_
#define IOTUS_ARCH_SHARED_UTILS_DEFAULT_CHORES_H_

#include "iotus-core.h"
#include "platform-conf.h"

/* This PRIORITY can have only 4 value, since it uses only 2 bits in the system. */
typedef enum IOTUS_PACKET_PRIORITY {
  IOTUS_PRIORITY_RADIO = 0,
  IOTUS_PRIORITY_DATA_LINK,
  IOTUS_PRIORITY_ROUTING,
  IOTUS_PRIORITY_TRANSPORT
} iotus_layer_priority;


/*
 * List of default known headers functions present into header
 */
typedef enum iotus_chores {
  IOTUS_CHORE_RESERVED = 0,

  /* Packet related chores */
  IOTUS_CHORE_INSERT_PKT_PREV_SRC_ADDRESS,
  IOTUS_CHORE_INSERT_PKT_NEXT_DST_ADDRESS,
  IOTUS_CHORE_INSERT_PKT_P2P_SRC_ADDRESS,
  IOTUS_CHORE_INSERT_PKT_P2P_DST_ADDRESS,
  IOTUS_CHORE_PKT_CHECKSUM,
  IOTUS_CHORE_PKT_RELIABLE,
  IOTUS_CHORE_PKT_ID,
  IOTUS_CHORE_PKT_FRAGMENTATION,
  IOTUS_CHORE_PKT_SEQUENCE_NUMBER,
  IOTUS_CHORE_PKT_SET_TTL,
  IOTUS_CHORE_PKT_P2P_RELIABLE,
  IOTUS_CHORE_PKT_P2P_PKT_ID,

  /* Network functions related chores */
  IOTUS_CHORE_ONEHOP_BROADCAST,
  IOTUS_CHORE_FLOODING,
  IOTUS_CHORE_NEIGHBOR_DISCOVERY,
  IOTUS_CHORE_SET_TX_POWER,

  /* System functions related chores */
  IOTUS_CHORE_SET_ADDR_FOR_RADIO,
  IOTUS_CHORE_SET_ADDR_SHORT_LONG,
  IOTUS_CHORE_SET_ADDR_PANID,
  IOTUS_CHORE_SET_ADDR_IPV6,

  /* Do not change this last option */
  IOTUS_FINAL_NUMBER_CHORE
} iotus_chores;


Status
iotus_subscribe_for_chore(iotus_layer_priority priority, iotus_chores func);

int8_t
iotus_get_layer_assigned_for(iotus_chores func);

#endif /* IOTUS_ARCH_SHARED_UTILS_DEFAULT_CHORES_H_*/

/* The following stuff ends the \defgroup block at the beginning of
   the file: */

/** @} */
