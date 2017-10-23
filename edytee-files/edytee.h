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
#include "net/packetbuf.h"

//************************************************************************
//                  General defines
//************************************************************************
#define NODE_MODE_DISCONNECTED              0
#define NODE_MODE_LEAF                      1
#define NODE_MODE_ROUTER                    2
#define NODE_MODE_GENERIC                   3
#define NODE_MODE_ROOT                      4


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

//************************************************************************
//                  Structs
//************************************************************************


//************************************************************************
//                  Prototypes
//************************************************************************

//Functions that must be available from the application layer
void edytee_recv_msg(const linkaddr_t *from);
void edytee_msg_confirm(int status, const linkaddr_t *dest, int num_tx);

//Available function to the application layer
inline int edytee_send_to_sink(const char *msg, uint16_t len);
int edytee_send_msg(linkaddr_t *recv_address, const char *msg, uint16_t len);
int edytee_close_connection(uint8_t handler);

#endif /* EDYTEE_COMM_H_ */
