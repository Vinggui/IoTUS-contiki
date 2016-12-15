/*
 * edytee-comm.h
 *
 *  Created on: 15/12/2016
 *      Author: vinicius
 */

#ifndef EDYTEE_COMM_H_
#define EDYTEE_COMM_H_

#include "contiki-conf.h"
#include "edytee-radio-params.h"

//************************************************************************
//                  General defines
//************************************************************************
#define NODE_ROLE_GENERIC                   0
#define NODE_ROLE_LEADER                    1
#define NODE_ROLE_LEAF                      2
#define NODE_ROLE_ROOT                      3

#define NODE_MINI_ADDRESS_GENERIC           0
#define NODE_MINI_ADDRESS_DEFINED           1

#define NODE_CHANNEL_GENERIC                0
#define NODE_CHANNEL_DEFINED                1

#define NODE_LEADER_ADDRESS_GENERIC         0
#define NODE_LEADER_ADDRESS_DEFINED         1

typedef enum {
    MESSAGE_TO_ROOT,
    MESSAGE_TO_NODE_NO_PATH,
    MESSAGE_TO_NODE_WITH_PATH} destination_type;


#ifdef ADDR_CONF_SIZE
#define ADDR_SIZE ADDR_CONF_SIZE
#define ADDR_MINI_SIZE ADDR_MINI_CONF_SIZE
#else /* ADDR_SIZE */
#define ADDR_SIZE 2
#define ADDR_MINI_SIZE 1
#endif /* ADDR_SIZE */


#define SEND_TO_ROOT(msg,len)           send_wireless_packet(\
        MESSAGE_TO_ROOT, addr_att_null, NULL, msg, len)

#define SEND_PACKET(addr, msg,len)      send_wireless_packet(\
    MESSAGE_TO_NODE_NO_PATH, MESSAGE_TO_NODE_NO_PATH, NULL, msg, len)

//************************************************************************
//                  Structs
//************************************************************************

typedef union {
  unsigned char u8[ADDR_SIZE];
  unsigned char mini_u8[ADDR_MINI_SIZE];
} addr_att_t;

typedef struct Packet_Fields_t {
    char payload[RADIO_MAX_PAYLOAD];
} packet_fields_t;

extern addr_att_t node_addr_att;
extern addr_att_t addr_att_null;
extern packet_fields_t packet_buff;
//************************************************************************
//                  Prototypes
//************************************************************************

//Functions that must be available from the application layer
void wireless_recv(const addr_att_t *from);
void Wireless_sent(int status, int num_tx);

//Available function to the application layer
void send_wireless_packet(destination_type dest_type, addr_att_t *recv_address,
        const char *path, const char *msg, uint16_t len);


#endif /* EDYTEE_COMM_H_ */
