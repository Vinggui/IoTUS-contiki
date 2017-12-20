/**
 * \defgroup decription...
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

typedef enum iotus_transport_protocols IOTUS_PROTOCOL_TRANSPORT_ENUM_OPTIONS iotus_transport_protocols;
typedef enum iotus_routing_protocols IOTUS_PROTOCOL_ROUTING_ENUM_OPTIONS iotus_routing_protocols;
typedef enum iotus_data_link_protocols IOTUS_PROTOCOL_DATA_LINK_ENUM_OPTIONS iotus_data_link_protocols;

typedef enum iotus_service_signals {IOTUS_START_SERVICE, IOTUS_RUN_SERVICE, IOTUS_END_SERVICE} iotus_service_signal;

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

//************************************************************************
//                  Prototypes
//************************************************************************
//Functions that must be available
void
iotus_core_start_system (
  iotus_transport_protocols transport,
  iotus_routing_protocols routing,
  iotus_data_link_protocols data_link);
#endif /* IOTUS_ARCH_IOTUS_CORE_H_ */
