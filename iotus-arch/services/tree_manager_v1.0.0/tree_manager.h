/**
 * \defgroup description...
 *
 * This...
 *
 * @{
 */

/*
 * tree_manager.h
 *
 *  Created on: Fev 13, 2019
 *      Author: vinicius
 */

#ifndef IOTUS_ARCH_SERVICES_NULL_NULL_H_
#define IOTUS_ARCH_SERVICES_NULL_NULL_H_

#include "iotus-core.h"


#define STATIC_TREE                   1//1 for true. 0 for false
#define STATIC_COORDINATORS_NUM       2
#define STATIC_COORDINATORS           PREDEFINED_COORDINATORS

extern uint8_t amIRouter;
extern uint8_t routerNodes[];

void
iotus_signal_handler_tree_manager(iotus_service_signal signal, void *data);

#endif /* IOTUS_ARCH_SERVICES_NULL_NULL_H_ */

/* The following stuff ends the \defgroup block at the beginning of
   the file: */

/** @} */
