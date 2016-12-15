/*
 * edytee-comm.c
 *
 * Layer designed to interface any application process with the edytee full
 * stack.
 *  Created on: 15/12/2016
 *      Author: vinicius
 */

addr_att_t node_addr_att;
const addr_att_t addr_att_null = { { 0, 0 } };

void send_wireless_packet(destination_type dest_type, addr_att_t *recv_address,
        const char *path, const char *msg, uint16_t len) {
    packetbuf_copyfrom(msg, size);
    if(!linkaddr_cmp(&recv_address, &linkaddr_node_addr)) {
      //unicast_send(&uc, &addr);
    }
}
