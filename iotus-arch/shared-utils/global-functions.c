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
#include "sys/rtimer.h"

#define DEBUG 0
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else /* DEBUG */
#define PRINTF(...)
#endif /* DEBUG */

////////////////////////////////////////////////////////////
//             TEMPORARY MEASURES                         //
////////////////////////////////////////////////////////////
rtimer_clock_t packetBuildingTime;

//////////////////////////////////////////////////////////////////
//                        Packet related                        //
//////////////////////////////////////////////////////////////////
/**
 * \brief       Calculates the amount of bytes safe to be used
 *              per layer, given that they will insert their
 *              headers.
 * @param layer IOTUS_PRIORITY_DATA_LINK
 *              IOTUS_PRIORITY_ROUTING
 *              IOTUS_PRIORITY_TRANSPORT
 *              any - Application
 * \return      The amount of bytes available to be used, per packet.
 */
uint16_t
get_safe_pdu_for_layer(iotus_layer_priority layer)
{
  uint16_t available_pdu;
  available_pdu = iotus_packet_dimensions.totalSize;
  if(layer == IOTUS_PRIORITY_DATA_LINK) {
    return available_pdu;
  }
  available_pdu -= iotus_packet_dimensions.datalinkHeaders;
  if(layer == IOTUS_PRIORITY_ROUTING) {
    return available_pdu;
  }
  available_pdu -= iotus_packet_dimensions.routingHeaders;
  if(layer == IOTUS_PRIORITY_TRANSPORT) {
    return available_pdu;
  }
  available_pdu -= iotus_packet_dimensions.transportHeaders;
  return available_pdu;
}



/***********************************************************************/


/* The following stuff ends the \defgroup block at the beginning of
   the file: */

/** @} */