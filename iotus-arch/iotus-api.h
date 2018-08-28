/**
 * \defgroup description...
 *
 * This...
 *
 * @{
 */

/*
 * iotus-api.h
 *
 *  Created on: Apr 25, 2018
 *      Author: vinicius
 */

#ifndef IOTUS_ARCH_IOTUS_API_H_
#define IOTUS_ARCH_IOTUS_API_H_

#include "iotus-core.h"
#include "layer-packet-manager.h"
#include "packet.h"
#include "packet-defs.h"
#include "packet-default-additional-info.h"
#include "pieces.h"
#include "nodes.h"
#include "platform-conf.h"

//////////////////////////////////////////////////////////////////////////
//                                Prototypes                            //
//////////////////////////////////////////////////////////////////////////


typedef void (* packet_sent_cb)(iotus_packet_t *packet, iotus_netstack_return returnAns);
typedef void (* packet_handler)(iotus_packet_t *packet);


/* Call this macro instead of the function iotus_core_start_system itself*/
#ifdef IOTUS_COMPILE_MODE_DYNAMIC
#define IOTUS_CORE_START(transport, packetForward, data_link, radio) \
              iotus_core_start_system(transport, packetForward, data_link, radio)
#else
#define IOTUS_CORE_START(transport, packetForward, data_link, radio) \
              iotus_core_start_system()
#endif


iotus_packet_t * iotus_initiate_msg(uint16_t payloadSize, const uint8_t* payload, uint8_t params,
    uint16_t timeout, iotus_node_t *finalDestination);

void
iotus_set_interface_functions(packet_sent_cb confirmationFunc, packet_handler appHandler);

void
iotus_set_demanding_task(uint16_t time_needed, application_demanding_task task);

#endif /* IOTUS_ARCH_IOTUS_API_H_ */
/* The following stuff ends the \defgroup block at the beginning of
   the file: */

/** @} */
