/**
 * \defgroup decription...
 *
 * This...
 *
 * @{
 */

/*
 * global-functions.c
 *
 *  Created on: Feb 27, 2018
 *      Author: vinicius
 */

#include <stdio.h>
#include <stdlib.h>
#include "iotus-core.h"
#include "platform-conf.h"

#define DEBUG 1
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else /* DEBUG */
#define PRINTF(...)
#endif /* DEBUG */

/***********************************************************************
                              Functions
 ***********************************************************************/
/**
 * \brief      Proceed the checksum for some given buffer
 * @param buf  The buffer pointer to operate.
 * @param size The size of this buffer.
 * \return  the checksum value.
 */
uint16_t
checksum_buf(uint8_t *buf, uint16_t size)
{
  //TODO create checksum
  return 0xFEED;
}





/***********************************************************************/


/* The following stuff ends the \defgroup block at the beginning of
   the file: */

/** @} */