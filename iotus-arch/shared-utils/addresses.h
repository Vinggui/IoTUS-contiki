/**
 * \defgroup description...
 *
 * This...
 *
 * @{
 */

/*
 * addresses.h
 *
 *  Created on: Nov 15, 2017
 *      Author: vinicius
 */

#ifndef IOTUS_ARCH_SHARED_UTILS_ADDRESSES_H_
#define IOTUS_ARCH_SHARED_UTILS_ADDRESSES_H_

#include <stdio.h>
#include <stdlib.h>
#include "addresses.h"
#include "iotus-core.h"
#include "lib/mmem.h"
#include "platform-conf.h"

//////////////////////////////////////////
//              Defines                 //
//////////////////////////////////////////
#define IOTUS_ADDRESSES_TYPE_INCLUDE(module_name)   \
      IOTUS_##module_name##_TYPE_ADDR_LONG=1,\
      IOTUS_##module_name##_TYPE_ADDR_SHORT,\
      IOTUS_##module_name##_TYPE_ADDR_PANID,\
      IOTUS_##module_name##_TYPE_ADDR_MINI,\
      IOTUS_##module_name##_TYPE_ADDR_EXTENDED,\
      IOTUS_##module_name##_TYPE_ADDR_IPV6\


typedef enum iotus_addresses_types {
  IOTUS_ADDRESSES_TYPE_INCLUDE(ADDRESSES),
  /* The hardcoded is given by the platform|cooja */
  IOTUS_ADDRESSES_TYPE_ADDR_BROADCAST,
  IOTUS_ADDRESSES_TYPE_ADDR_TOTAL
} iotus_address_type;

/////////////////////////////////////////
//                MACRO                //
/////////////////////////////////////////
#define ADDRESSES_GET_TYPE_SIZE(type)         addresses_types_sizes[type]
#define ADDRESSES_SET_TYPE_SIZE(type,size)    addresses_types_sizes[type]=size
#define addresses_get_pointer(type)           ((uint8_t *)MMEM_PTR(&(addresses_mem[type])))


///////////////////////////////////////////
//            Externs                    //
///////////////////////////////////////////
extern uint8_t addresses_types_sizes[];
extern struct mmem addresses_mem[IOTUS_ADDRESSES_TYPE_ADDR_TOTAL-1];
extern uint8_t iotus_node_id_hardcoded[3];
extern uint8_t iotus_panid_hardcoded[3];

//////////////////////////////////////////
//             Functions                //
//////////////////////////////////////////
Boolean
addresses_compare(uint8_t *addr1, uint8_t *addr2, uint8_t addressesSize);

Status
addresses_set_value(iotus_address_type type, const uint8_t *value);

#endif /* IOTUS_ARCH_SHARED_UTILS_ADDRESSES_H_ */

/* The following stuff ends the \defgroup block at the beginning of
   the file: */

/** @} */