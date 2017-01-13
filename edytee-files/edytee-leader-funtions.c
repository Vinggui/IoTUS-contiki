/*
 * edytee-leader-funtions.c
 *
 *  Created on: 11/01/2017
 *      Author: vinicius
 */

#include "edytee-leader-functions.h"
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

void start_own_cluster_period(void *ptr) {
    //Create our first beacon
    packetbuf_copyfrom("BEACON", 6);
    packetbuf_set_addr(PACKETBUF_ADDR_SENDER, &linkaddr_node_addr);
    packetbuf_set_attr(PACKETBUF_ATTR_CHANNEL, 21);
    packetbuf_set_attr(PACKETBUF_ATTR_PACKET_TYPE, PACKET_TYPE_BEACON);
    packetbuf_compact();

    int ret;
    if(NETSTACK_RADIO.send(packetbuf_hdrptr(), packetbuf_totlen()) == RADIO_TX_OK) {
        PRINTF("MAC sent packet\n");
      ret = MAC_TX_OK;
    } else {
        PRINTF("MAC failed in sending packet\n");
      ret =  MAC_TX_ERR;
    }

    ctimer_restart((struct ctimer *)ptr);
}

void init_leader_functions(void) {
    //TODO Create and set the initial table of leafs

#if WIRELESS_COMM_ROLE == NODE_MODE_GENERIC
    //TODO CHeck if channel to leader is already defined
#elif WIRELESS_COMM_ROLE == NODE_MODE_ROOT
    //Just choose the best channel and create a cluster with it

    //TODO Scan channels and get the best

#endif /*WIRELESS_COMM_ROLE == NODE_MODE_GENERIC*/
}
