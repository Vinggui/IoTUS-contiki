/**
 * \defgroup decription...
 *
 * This...
 *
 * @{
 */

/*
 * packet-default-chores.h
 *
 *  Created on: Nov 15, 2017
 *      Author: vinicius
 */

#ifndef IOTUS_ARCH_SERVICES_PACKET_DEFAULT_CHORES_H_
#define IOTUS_ARCH_SERVICES_PACKET_DEFAULT_CHORES_H_

/*
 * List of default known headers functions present into header
 */
typedef enum IOTUS_DEFAULT_HEADER_CHORES {
  IOTUS_DEFAULT_HEADER_CHORE_SOURCE_ADDRESS = 0,
  IOTUS_DEFAULT_HEADER_CHORE_CHECKSUM,
  IOTUS_DEFAULT_HEADER_CHORE_FRAGMENT,
  IOTUS_DEFAULT_HEADER_CHORE_SEQUENCE_NUMBER,

  IOTUS_DEFAULT_HEADER_CHORE_ONEHOP_BROADCAST,


  IOTUS_FINAL_NUMBER_DEFAULT_HEADER_CHORE
} iotus_default_header_chores;

#endif /* IOTUS_ARCH_SERVICES_PACKET_DEFAULT_CHORES_H_*/

/* The following stuff ends the \defgroup block at the beginning of
   the file: */

/** @} */
