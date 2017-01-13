/*
 * edytee-leader-functions.h
 *
 *  Created on: 11/01/2017
 *      Author: vinicius
 */

#ifndef EDYTEE_FILES_EDYTEE_LEADER_FUNCTIONS_H_
#define EDYTEE_FILES_EDYTEE_LEADER_FUNCTIONS_H_

#include "wireless-comm-configs.h"

#ifndef WIRELESS_COMM_SUB_CHANNEL_TYPE
#   pragma message ("WIRELESS_COMM_SUB_CHANNEL_TYPE not defined. Using NODE_CHANNEL_GENERIC.")
#   define WIRELESS_COMM_SUB_CHANNEL_TYPE     NODE_CHANNEL_GENERIC
#elif WIRELESS_COMM_SUB_CHANNEL_TYPE != NODE_CHANNEL_GENERIC
#   ifndef WIRELESS_COMM_SUB_CHANNEL
#       error "WIRELESS_COMM_SUB_CHANNEL not defined!"
#   endif
#endif /*WIRELESS_COMM_SUB_CHANNEL_TYPE*/

void start_own_cluster_period(void *ptr);
void init_leader_functions(void);

#endif /* EDYTEE_FILES_EDYTEE_LEADER_FUNCTIONS_H_ */
