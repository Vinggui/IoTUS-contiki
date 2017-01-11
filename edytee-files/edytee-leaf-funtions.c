/*
 * edytee-leaf-funtions.c
 *
 *  Created on: 11/01/2017
 *      Author: vinicius
 */

#include "edytee-leaf-functions.h"
#include "edytee-comm.h"
#include "edytee-mac.h"
#include "wireless-comm-configs.h"

static uint8_t mini_address[WIRELESS_COMM_MINI_ADDRESS_SIZE];

void init_leaf_functions(void) {
#if WIRELESS_COMM_MINI_ADDRESS_TYPE == NODE_MINI_ADDRESS_GENERIC
    //Nothing to do, mini address will be set as it works...
#else
    uint8_t count;
    for(count=0; count < WIRELESS_COMM_MINI_ADDRESS_SIZE; count ++) {
        mini_address[count] = ((WIRELESS_COMM_MINI_ADDRESS >> count*8) & 0xFF);
    }
#endif /*WIRELESS_COMM_MINI_ADDRESS_TYPE == NODE_MINI_ADDRESS_GENERIC*/
    //Start our searching for clusters nearby
}
