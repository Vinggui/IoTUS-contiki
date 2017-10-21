/*
 * edytee-comm.c
 *
 * Layer designed to interface any application process with the edytee full
 * stack.
 *  Created on: 15/12/2016
 *      Author: vinicius
 */

#include "edytee-comm.h"
#include "net/linkaddr.h"
#include "net/netstack.h"


#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...)         printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

static void packet_sent(void *ptr, int status, int num_tx) {
    const linkaddr_t *dest = packetbuf_addr(PACKETBUF_ADDR_RECEIVER);
    wireless_sent(status, dest, num_tx);
}

void send_wireless_packet(destination_type dest_type, linkaddr_t *recv_address,
        const char *path, const char *msg, uint16_t len) {
    PRINTF("%d.%d: unicast_send to %d.%d\n",
       linkaddr_node_addr.u8[0],linkaddr_node_addr.u8[1],
       recv_address->u8[0], recv_address->u8[1]);
    packetbuf_copyfrom(msg, len);
    if(!linkaddr_cmp(recv_address, &linkaddr_node_addr)) {
        packetbuf_set_addr(PACKETBUF_ADDR_RECEIVER, recv_address);
        packetbuf_set_addr(PACKETBUF_ADDR_SENDER, &linkaddr_node_addr);
        packetbuf_set_attr(PACKETBUF_ATTR_CHANNEL, 21);
        packetbuf_compact();

        NETSTACK_LLSEC.send(packet_sent, NULL);
    }
}

