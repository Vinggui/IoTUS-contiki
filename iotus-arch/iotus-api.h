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
#include "packet.h"
#include "packet-defs.h"
#include "packet-default-additional-info.h"
#include "pieces.h"
#include "nodes.h"
#include "platform-conf.h"

//////////////////////////////////////////////////////////////////////////
//                                Prototypes                            //
//////////////////////////////////////////////////////////////////////////
/* Call this macro instead of the function iotus_core_start_system itself*/
#ifdef IOTUS_COMPILE_MODE_DYNAMIC
#define IOTUS_CORE_START(transport, routing, data_link, radio) \
              iotus_core_start_system(transport, routing, data_link, radio)
#else
#define IOTUS_CORE_START(transport, routing, data_link, radio) \
              iotus_core_start_system()
#endif

//////////////////////////////////////////////////////////////////////////
//                                Functions                             //
//////////////////////////////////////////////////////////////////////////
iotus_packet_t *
iotus_initiate_msg(uint16_t payloadSize, const uint8_t* payload, uint8_t params,
    iotus_layer_priority priority, uint16_t timeout, iotus_node_t *finalDestination);

void
iotus_allow_sleep(Boolean isAllowed);

void
iotus_set_demanding_task(uint16_t time_needed, application_demanding_task task);

/* Do not call this function below directly. Use the macro IOTUS_CORE_START instead. */
void
iotus_core_start_system (
  #ifdef IOTUS_COMPILE_MODE_DYNAMIC
  iotus_transport_protocols transport,
  iotus_routing_protocols routing,
  iotus_data_link_protocols data_link,
  iotus_radio_drivers radio_driver
  #else
  void
  #endif
);

#endif /* IOTUS_ARCH_IOTUS_API_H_ */
/* The following stuff ends the \defgroup block at the beginning of
   the file: */

/** @} */
