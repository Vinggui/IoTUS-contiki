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

enum iotus_transport_protocols IOTUS_PROTOCOL_TRANSPORT_ENUM_OPTIONS;
enum iotus_routing_protocols IOTUS_PROTOCOL_ROUTING_ENUM_OPTIONS;
enum iotus_data_link_protocols IOTUS_PROTOCOL_DATA_LINK_ENUM_OPTIONS;

struct iotus_service_struct {
  void (* start)(void);
  void (* run)(void);
  void (* close)(void);
};


//************************************************************************
//                  Prototypes
//************************************************************************

//Functions that must be available
void start_new_comm_stack (void);

#endif /* IOTUS_ARCH_IOTUS_CORE_H_ */
