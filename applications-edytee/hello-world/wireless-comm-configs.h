/*
 * wireless-comm-configs.h
 *
 *  Created on: 15/12/2016
 *      Author: vinicius
 */

#ifndef WIRELESS_COMM_CONFIGS_H_
#define WIRELESS_COMM_CONFIGS_H_
#include "edytee-comm.h"

//Main parameters

//Node Role in this network
#define WIRELESS_COMM_ROLE                  NODE_ROLE_GENERIC
//Node mini address type
#define WIRELESS_COMM_MINI_ADDRESS_TYPE     NODE_MINI_ADDRESS_GENERIC

//Node channel to connect
#define WIRELESS_COMM_MAIN_CHANNEL_TYPE     NODE_CHANNEL_GENERIC
//Node channel to create, if a LEADER node
#define WIRELESS_COMM_SUB_CHANNEL_TYPE      NODE_CHANNEL_GENERIC


//Dependent paramaters

//Set this parameter if WIRELESS_COMM_MINI_ADDRESS_TYPE is not GENERIC
#define WIRELESS_COMM_MINI_ADDRESS          0

//Set this parameter if WIRELESS_COMM_MAIN_CHANNEL is not GENERIC
#define WIRELESS_COMM_MAIN_CHANNEL          0

//Set this parameter if WIRELESS_COMM_SUB_CHANNEL is not GENERIC
#define WIRELESS_COMM_SUB_CHANNEL           0

#endif /* WIRELESS_COMM_CONFIGS_H_ */
