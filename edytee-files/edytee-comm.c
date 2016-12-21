/*
 * edytee-comm.c
 *
 * Layer designed to interface any application process with the edytee full
 * stack.
 *  Created on: 15/12/2016
 *      Author: vinicius
 */

addr_att_t node_addr_att;
packet_fields_t packet_buff;
const addr_att_t addr_att_null = { { 0, 0 } };

void send_wireless_packet(destination_type dest_type, addr_att_t *recv_address,
        const char *path, const char *msg, uint16_t len) {
    packetbuf_copyfrom(msg, size);
    if(!linkaddr_cmp(&recv_address, &linkaddr_node_addr)) {
      //unicast_send(&uc, &addr);
    }
}

void addr_copy(addr_att_t *dest, const addr_att_t *src) {
    memcpy(dest, src, ADDR_SIZE);
}
/*---------------------------------------------------------------------------*/
int addr_cmp(const addr_att_t *addr1, const addr_att_t *addr2) {
    return (memcmp(addr1, addr2, ADDR_SIZE) == 0);
}
/*---------------------------------------------------------------------------*/
void set_node_addr_att(addr_att_t *t) {
    linkaddr_copy(&node_addr_att, t);
}
