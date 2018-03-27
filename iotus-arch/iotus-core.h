/**
 * \defgroup dedcription...
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



/* The following stuff ends the \defgroup block at the beginning of
   the file: */

/** @} */
#ifndef IOTUS_ARCH_IOTUS_CORE_H_
#define IOTUS_ARCH_IOTUS_CORE_H_

#include "platform-conf.h"

#ifdef IOTUS_COMPILE_MODE_DYNAMIC
  //Define the Enumaration types for dynamic mode
  #if IOTUS_CONF_USING_TRANSPORT == 1
    typedef enum iotus_transport_protocols IOTUS_PROTOCOL_TRANSPORT_ENUM_OPTIONS iotus_transport_protocols;
  #else
    typedef enum iotus_transport_protocols {IOTUS_NO_TRANSPORT_LAYER} iotus_transport_protocols;
  #endif
  #if IOTUS_CONF_USING_ROUTING == 1
    typedef enum iotus_routing_protocols IOTUS_PROTOCOL_ROUTING_ENUM_OPTIONS iotus_routing_protocols;
  #else
    typedef enum iotus_routing_protocols {IOTUS_NO_ROUTING_LAYER} iotus_routing_protocols;
  #endif
  #if IOTUS_CONF_USING_DATA_LINK == 1
    typedef enum iotus_data_link_protocols IOTUS_PROTOCOL_DATA_LINK_ENUM_OPTIONS iotus_data_link_protocols;
  #else
    typedef enum iotus_data_link_protocols {IOTUS_NO_DATA_LINK_LAYER} iotus_data_link_protocols;
  #endif

    typedef enum iotus_radio_drivers IOTUS_RADIO_DRIVERS_ENUM_OPTIONS iotus_radio_drivers;
#endif /* #ifdef IOTUS_COMPILE_MODE_DYNAMIC */

typedef enum iotus_service_signals {IOTUS_START_SERVICE, IOTUS_RUN_SERVICE, IOTUS_END_SERVICE} iotus_service_signal;


typedef uint8_t Boolean;
#define TRUE            1
#define FALSE           0

struct iotus_transport_protocol_struct {
  void (* start)(void);
  void (* run)(void);
  void (* close)(void);
};

struct iotus_routing_protocol_struct {
  void (* start)(void);
  void (* run)(void);
  void (* close)(void);
};

struct iotus_data_link_protocol_struct {
  void (* start)(void);
  void (* run)(void);
  void (* close)(void);
};

struct iotus_radio_driver_struct {
  void (* start)(void);
  void (* run)(void);
  void (* close)(void);
};

//************************************************************************
//                  Prototypes
//************************************************************************
/* Call this macro instead of the function iotus_core_start_system itself*/
#ifdef IOTUS_COMPILE_MODE_DYNAMIC
#define IOTUS_CORE_START(transport, routing, data_link, radio) iotus_core_start_system(transport, routing, data_link, radio)
#else
#define IOTUS_CORE_START(transport, routing, data_link, radio) iotus_core_start_system()
#endif
//Functions that must be available
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
#endif /* IOTUS_ARCH_IOTUS_CORE_H_ */
/* The following stuff ends the \defgroup block at the beginning of
   the file: */

/** @} */
