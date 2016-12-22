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

void send_wireless_packet(destination_type dest_type, linkaddr_t *recv_address,
        const char *path, const char *msg, uint16_t len) {
    packetbuf_copyfrom(msg, len);
    if(!linkaddr_cmp(recv_address, &linkaddr_node_addr)) {
      //unicast_send(&uc, &addr);
    }
}

