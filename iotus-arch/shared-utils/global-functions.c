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
#include "global-parameters.h"

#define DEBUG 0
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else /* DEBUG */
#define PRINTF(...)
#endif /* DEBUG */


//////////////////////////////////////////////////////////////////
//                        Packet related                        //
//////////////////////////////////////////////////////////////////
/**
 * \brief       Calculates the amount of bytes safe to be used
 *              per layer, given that they will insert their
 *              headers.
 * @param layer 2 - Data link
 *              3 - Routing
 *              4 - Transport
 *              5 - Application
 * \return      The amount of bytes available to be used, per packet.
 */
uint16_t
get_safe_pdu_for_layer(uint8_t layer)
{
  uint16_t available_pdu;
  available_pdu = iotus_packet_dimensions.totalSize
                  - iotus_packet_dimensions.radioHeaders;
  if(layer == 2) {
    return available_pdu;
  }
  available_pdu -= iotus_packet_dimensions.datalinkHeaders;
  if(layer == 3) {
    return available_pdu;
  }
  available_pdu -= iotus_packet_dimensions.routingHeaders;
  if(layer == 4) {
    return available_pdu;
  }
  available_pdu -= iotus_packet_dimensions.transportHeaders;
  return available_pdu;
}



/***********************************************************************/


/* The following stuff ends the \defgroup block at the beginning of
   the file: */

/** @} */