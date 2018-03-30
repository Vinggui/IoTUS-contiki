/**
 * \defgroup decription...
 *
 * This...
 *
 * @{
 */

/*
 * addresses.c
 *
 *  Created on: Nov 18, 2017
 *      Author: vinicius
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "addresses.h"
#include "iotus-core.h"
#include "lib/mmem.h"
#include "platform-conf.h"


uint8_t addresses_types_sizes[IOTUS_ADDRESSES_TYPE_ADDR_TOTAL-1];
struct mmem addresses_mem[IOTUS_ADDRESSES_TYPE_ADDR_TOTAL-1]={{NULL,0,NULL}};
uint8_t iotus_node_id_hardcoded[4]={0};
uint8_t iotus_node_long_id_hardcoded[8]={0};
uint8_t iotus_pan_id_hardcoded[8]={0};

/*---------------------------------------------------------------------*/
/*
 * \brief Compare two addresses, considering the type requested.
 * \param addr1    The pointer of the first address.
 * \param addr2    The pointer of the second address.
 * \return         True if they match.
 */
Boolean
addresses_compare(uint8_t *addr1, uint8_t *addr2, uint8_t addressesSize)
{
  uint8_t i;
  for(i=0; i<addressesSize; i++) {
    if(addr1 != addr2) {
      return TRUE;
    }
  }
  return FALSE;
}
/*---------------------------------------------------------------------*/
/**
 * \brief       Set the address into the system, given it's type.
 * @param type  The type of the address to be set.
 * @param value The array of byte for this address.
 */
Status
addresses_set_value(iotus_address_type type, const uint8_t *value)
{
  //The type size must be already set by now.
  if(ADDRESSES_GET_TYPE_SIZE(type) == 0) {
    return FAILURE;
  }


  if(IOTUS_ADDRESSES_TYPE_ADDR_SHORT == type) {
    memcpy(iotus_node_id_hardcoded,value,ADDRESSES_GET_TYPE_SIZE(type));
  } else if(IOTUS_ADDRESSES_TYPE_ADDR_LONG == type) {
    memcpy(iotus_node_long_id_hardcoded,value,ADDRESSES_GET_TYPE_SIZE(type));
  } else if(IOTUS_ADDRESSES_TYPE_ADDR_PANID == type) {
    memcpy(iotus_pan_id_hardcoded,value,ADDRESSES_GET_TYPE_SIZE(type));
  } else {
    //Verify if this address already have something...
    if(addresses_mem[type].size != 0) {
      mmem_free(&(addresses_mem[type]));
    }
    if(0 == mmem_alloc(&(addresses_mem[type]),ADDRESSES_GET_TYPE_SIZE(type))) {
      return FAILURE;
    }
    
    memcpy((uint8_t *)MMEM_PTR(&(addresses_mem[type])),value,ADDRESSES_GET_TYPE_SIZE(type));
  }
  return SUCCESS;
}
/*---------------------------------------------------------------------*/
/**
 * 
 */
uint8_t *
addresses_get_pointer(iotus_address_type type)
{
  if(IOTUS_ADDRESSES_TYPE_ADDR_SHORT == type) {
    return iotus_node_id_hardcoded;
  } else if(IOTUS_ADDRESSES_TYPE_ADDR_LONG == type) {
    return iotus_node_long_id_hardcoded;
  } else if(IOTUS_ADDRESSES_TYPE_ADDR_PANID == type) {
    return iotus_pan_id_hardcoded;
  } else {
    return ((uint8_t *)MMEM_PTR(&(addresses_mem[type])));
  }
}
/*---------------------------------------------------------------------*/


/* The following stuff ends the \defgroup block at the beginning of
   the file: */

/** @} */
