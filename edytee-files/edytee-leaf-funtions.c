/*
 * edytee-leaf-funtions.c
 *
 *  Created on: 11/01/2017
 *      Author: vinicius
 */

#include "edytee-leaf-functions.h"
#include "edytee-comm.h"
#include "edytee-mac.h"
#include "net/packetbuf.h"
#include "net/queuebuf.h"
#include "net/netstack.h"
#include "wireless-comm-configs.h"

#define DEBUG 1
#if DEBUG
#include <stdio.h>
#define PRINTF(...)         printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

static uint8_t mini_address[WIRELESS_COMM_MINI_ADDRESS_SIZE];
static uint8_t main_channel;


static void channel_beacon_search_cb(void *ptr) {
    PRINTF("No beacon found in channel %d\n", main_channel);
#if WIRELESS_COMM_MAIN_CHANNEL_TYPE == NODE_CHANNEL_GENERIC
    if(main_channel < RADIO_LAST_CHANNEL) {
        main_channel += RADIO_CHANNEL_STEP_COUNT;
        NETSTACK_RADIO.set_value(RADIO_PARAM_CHANNEL, main_channel);
        ctimer_restart(ptr);
    } else {
        //TODO Study which channel to connect.
    }
#else
    //TODO Study what to do with the channel we are working with

#endif /*WIRELESS_COMM_MAIN_CHANNEL_TYPE == NODE_CHANNEL_GENERIC*/
}

void start_leaf_operations_period(void *ptr) {//ptr is the mac_ctimer
    //verify state of operation
    if(NETWORK_STATUS_DISCONNECTED == device_status) {
#if WIRELESS_COMM_MAIN_CHANNEL_TYPE == NODE_CHANNEL_GENERIC
        //Scan all channels for the best leader available
        main_channel = RADIO_FIRST_CHANNEL;
        NETSTACK_RADIO.set_value(RADIO_PARAM_CHANNEL, RADIO_FIRST_CHANNEL);
#else
        //Scan a specific channel directly
        main_channel = WIRELESS_COMM_MAIN_CHANNEL;
        NETSTACK_RADIO.set_value(RADIO_PARAM_CHANNEL, WIRELESS_COMM_MAIN_CHANNEL);
#endif
        NETSTACK_RADIO.on();
        ctimer_set(ptr, CLOCK_SECOND*MAC_MAX_PERIOD_DURATION,
                channel_beacon_search_cb, ptr);
        device_status = NETWORK_STATUS_SEARCHING_CLUSTER;
    }
}

void init_leaf_functions(void) {
    main_channel = RADIO_FIRST_CHANNEL;

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
