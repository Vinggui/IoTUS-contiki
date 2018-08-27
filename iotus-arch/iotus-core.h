/**
 * \defgroup description...
 *
 * This...
 *
 * @{
 */

/*
 * iotus-core.h
 *
 *  Created on: Oct 23, 2017
 *      Author: vinicius
 */
#ifndef IOTUS_ARCH_IOTUS_CORE_H_
#define IOTUS_ARCH_IOTUS_CORE_H_

#include "platform-conf.h"

typedef int8_t Boolean;
#define TRUE            1
#define FALSE           0

typedef int8_t Status;
#define SUCCESS         1
#define FAILURE         0


/* This PRIORITY can have only 4 value, since it uses only 2 bits in the system. */
typedef enum IOTUS_LAYER_PRIORITY {
  IOTUS_PRIORITY_RADIO = 0,
  IOTUS_PRIORITY_DATA_LINK,
  IOTUS_PRIORITY_ROUTING,
  IOTUS_PRIORITY_TRANSPORT,
  IOTUS_PRIORITY_APPLICATION,
  IOTUS_PRIORITY_APPLICATION_SUB_1,
  IOTUS_PRIORITY_APPLICATION_SUB_2,
  IOTUS_PRIORITY_APPLICATION_SUB_3,
  IOTUS_PRIORITY_APPLICATION_SUB_4
} iotus_layer_priority;


/* Generic LAYER return values. */
typedef enum IOTUS_NETSTACK_RETURN {
  /**< The MAC layer transmission was OK. */
  MAC_TX_OK,

  /**< The MAC layer transmission could not be performed due to a
     collision. */
  MAC_TX_COLLISION,

  /**< The MAC layer did not get an acknowledgement for the packet. */
  MAC_TX_NOACK,

  /**< The MAC layer deferred the transmission for a later time. */
  MAC_TX_DEFERRED,

  /**< The MAC layer transmission could not be performed because of an
     error. The upper layer may try again later. */
  MAC_TX_ERR,

  /**< The MAC layer transmission could not be performed because of a
     fatal error. The upper layer does not need to try again, as the
     error will be fatal then as well. */
  MAC_TX_ERR_FATAL,

  //#######################################

  /**< The Routing layer transmission was OK. */
  ROUTING_TX_OK,

  /**< The Routing layer transmission could not be performed because of an
     error. The upper layer may try again later. */
  ROUTING_TX_ERR,

  /**< The Routing layer transmission could not be performed because of a
     fatal error. The upper layer does not need to try again, as the
     error will be fatal then as well. */
  ROUTING_TX_ERR_FATAL,

  //#######################################

  /**< The transport layer transmission was OK. */
  TRANSPORT_TX_OK,

  /**< The transport layer transmission could not be performed because of an
     error. The upper layer may try again later. */
  TRANSPORT_TX_ERR,

  /**< The transport layer transmission could not be performed because of a
     fatal error. The upper layer does not need to try again, as the
     error will be fatal then as well. */
  TRANSPORT_TX_ERR_FATAL,

  //#######################################

  RX_ERR_DROPPED,
  RX_PROCESSED
} iotus_netstack_return;

#ifdef IOTUS_COMPILE_MODE_DYNAMIC
  //Define the Enumaration types for dynamic mode
  //#if IOTUS_CONF_USING_TRANSPORT == 1
    typedef enum iotus_transport_protocols IOTUS_PROTOCOL_TRANSPORT_ENUM_OPTIONS iotus_transport_protocols;
  //#else
    //typedef enum iotus_transport_protocols {IOTUS_NO_TRANSPORT_LAYER} iotus_transport_protocols;
  //#endif
  //#if IOTUS_CONF_USING_ROUTING == 1
    typedef enum iotus_network_protocols IOTUS_PROTOCOL_NETWORK_ENUM_OPTIONS iotus_network_protocols;
  //#else
    //typedef enum iotus_routing_protocols {IOTUS_NO_ROUTING_LAYER} iotus_routing_protocols;
  //#endif
  //#if IOTUS_CONF_USING_DATA_LINK == 1
    typedef enum iotus_data_link_protocols IOTUS_PROTOCOL_DATA_LINK_ENUM_OPTIONS iotus_data_link_protocols;
  //#else
    //typedef enum iotus_data_link_protocols {IOTUS_NO_DATA_LINK_LAYER} iotus_data_link_protocols;
  //#endif

    typedef enum iotus_radio_drivers IOTUS_RADIO_DRIVERS_ENUM_OPTIONS iotus_radio_drivers;
#endif /* #ifdef IOTUS_COMPILE_MODE_DYNAMIC */

typedef enum iotus_service_signals {IOTUS_START_SERVICE, IOTUS_RUN_SERVICE, IOTUS_END_SERVICE} iotus_service_signal;

typedef void (* application_demanding_task)(void);

///////////////////////////////////////////////////////////////////////
//                                Externs                            //
///////////////////////////////////////////////////////////////////////
//#if IOTUS_CONF_USING_TRANSPORT == 1
  extern struct iotus_transport_protocol_struct const *active_transport_protocol;
//#endif
//#if IOTUS_CONF_USING_ROUTING == 1
  extern struct iotus_network_protocol_struct const *active_network_protocol;
//#endif
//#if IOTUS_CONF_USING_DATA_LINK == 1
  extern struct iotus_data_link_protocol_struct const *active_data_link_protocol;
//#endif
extern struct iotus_radio_driver_struct const *active_radio_driver;

//////////////////////////////////////////////////////////////////////////
//                                Prototypes                            //
//////////////////////////////////////////////////////////////////////////
void
iotus_core_netstack_idle_for(iotus_layer_priority layer, uint16_t maxDuration);

#endif /* IOTUS_ARCH_IOTUS_CORE_H_ */
/* The following stuff ends the \defgroup block at the beginning of
   the file: */

/** @} */
