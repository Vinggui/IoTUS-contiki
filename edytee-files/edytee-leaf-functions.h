/*
 * edytee-leaf-functions.h
 *
 *  Created on: 11/01/2017
 *      Author: vinicius
 */

#ifndef EDYTEE_FILES_EDYTEE_LEAF_FUNCTIONS_H_
#define EDYTEE_FILES_EDYTEE_LEAF_FUNCTIONS_H_

#include "wireless-comm-configs.h"

#ifndef WIRELESS_COMM_MINI_ADDRESS_TYPE
#   pragma message ("WIRELESS_COMM_MINI_ADDRESS_TYPE not defined. Using NODE_MINI_ADDRESS_GENERIC.")
#   define WIRELESS_COMM_MINI_ADDRESS_TYPE     NODE_MINI_ADDRESS_GENERIC
#elif WIRELESS_COMM_MINI_ADDRESS_TYPE != NODE_MINI_ADDRESS_GENERIC
#   ifndef WIRELESS_COMM_MINI_ADDRESS
#       error "WIRELESS_COMM_MINI_ADDRESS not defined!"
#   endif
#endif /*WIRELESS_COMM_MINI_ADDRESS_TYPE*/

#ifndef WIRELESS_COMM_MINI_ADDRESS_SIZE
#pragma message ("WIRELESS_COMM_MINI_ADDRESS_SIZE not defined. Using 1 byte.")
#define WIRELESS_COMM_MINI_ADDRESS_SIZE             1
#endif /*WIRELESS_COMM_MINI_ADDRESS_SIZE*/

#ifndef WIRELESS_COMM_MAIN_CHANNEL_TYPE
#   pragma message ("WIRELESS_COMM_MAIN_CHANNEL_TYPE not defined. Using NODE_CHANNEL_GENERIC.")
#   define WIRELESS_COMM_MAIN_CHANNEL_TYPE     NODE_CHANNEL_GENERIC
#elif WIRELESS_COMM_MAIN_CHANNEL_TYPE != NODE_CHANNEL_GENERIC
#   ifndef WIRELESS_COMM_MAIN_CHANNEL
#       error "WIRELESS_COMM_MAIN_CHANNEL not defined!"
#   endif
#endif /*WIRELESS_COMM_MAIN_CHANNEL_TYPE*/

void start_leaf_operations_period(void *ptr);
void init_leaf_functions(void);

#endif /* EDYTEE_FILES_EDYTEE_LEAF_FUNCTIONS_H_ */
