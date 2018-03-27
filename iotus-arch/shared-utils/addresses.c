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
#include "addresses.h"
#include "iotus-core.h"
#include "platform-conf.h"


uint8_t addresses_types_sizes[IOTUS_ADDRESSES_TYPE_ADDR_TOTAL];


/*---------------------------------------------------------------------*/
/*
 * \brief Compare two addresses, considering the type requested.
 * \param addr1    The pointer of the first address.
 * \param addr2    The pointer of the second address.
 * \return         True if they match.
 */
uint8_t
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
/* The following stuff ends the \defgroup block at the beginning of
   the file: */

/** @} */
