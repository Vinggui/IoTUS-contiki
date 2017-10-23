/* \defgroup core of the edytee stack
 *
 * This is the interface available for the application.
 *
 * @{
 */

/*
 * edytee.c
 *
 * Layer designed to interface any application process with the edytee full
 * stack.
 *  Created on: 15/12/2016
 *      Author: vinicius
 */

#include "edytee.h"

#include "net/linkaddr.h"
#include "net/netstack.h"


#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...)         printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif


/*---------------------------------------------------------------------------*/
static void packet_sent(void *ptr, int status, int num_tx)
{
    const linkaddr_t *dest = packetbuf_addr(PACKETBUF_ADDR_RECEIVER);
    edytee_msg_confirm(status, dest, num_tx);
}

/*---------------------------------------------------------------------------*/

static int wireless_send_msg(destination_type dest_type, linkaddr_t *recv_address,
        const char *path, const char *msg, uint16_t len)
{
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
    return 1;
}


/*---------------------------------------------------------------------------*/
/**
 * \brief      Packet the message to be delivered to the sink of the network.
 * \return     Informs the sink that the message was received or not.
 * \param msg  Message to be transmitted, can be a byte array.
 * \param len  Total length of the message to be sent.
 * \retval 0   Failed to decode message.
 * \retval 1   Message sent successfully.
 *
 *             This will pack and transmit a message for the sink node of
 *             the network.
 *
 *             NOTE: This function will not create a handler, so no need
 *             to disconnect later.
 *
 */
inline int edytee_send_to_sink(const char *msg, uint16_t len)
{
  return wireless_send_msg(MESSAGE_TO_ROOT, NULL, NULL, msg, len);
}

/*---------------------------------------------------------------------------*/

/**
 * \brief      Packet the message to be sent and deliver to lower layer.
 * \param dest_type Specifies the destination type, if sink or other node.
 * \param recv_address  The address of the other device.
 * \param msg  Message to be transmitted, can be a byte array.
 * \param len  Total length of the message to be sent.
 * \return     Returns the handler of the connection to the required recv_address.
 * \retval 0   Failed to send message.
 *
 *             This will transmit the message to the requested node, however
 *             the path to that node will be first requested to the sink node of the
 *             network. Hence, a buff will store its path to future use. If the
 *             application knows that it won't use this path anymore, just call
 *             the connection_close function with the handler as parameter.
 *
 *             NOTE: This function create a buffer and return a handler to it.
 *             Application must request to delete it, throw connection_close.
 *
 */
int edytee_send_msg(linkaddr_t *recv_address, const char *msg, uint16_t len)
{
  //TODO return wireless_send_msg((MESSAGE_TO_ROOT, NULL, NULL, msg, len));
  return 1;
}

/*---------------------------------------------------------------------------*/

/**
 * \brief      Packet the message to be sent and deliver to lower layer.
 * \param dest_type Specifies the destination type, if sink or other node.
 * \param recv_address  The address of the other device.
 * \param msg  Message to be transmitted, can be a byte array.
 * \param len  Total length of the message to be sent.
 * \return     Returns the handler of the connection to the required recv_address.
 * \retval 0   Failed to send message.
 *
 *             This will transmit the message to the requested node, however
 *             the path to that node will be first requested to the sink node of the
 *             network. Hence, a buff will store its path to future use. If the
 *             application knows that it won't use this path anymore, just call
 *             the connection_close function with the handler as parameter.
 *
 *             NOTE: This function create a buffer and return a handler to it.
 *             Application must request to delete it, throw connection_close.
 *
 */
int edytee_close_connection(uint8_t handler) {
  //TODO
  return 1;
}

/* The following stuff ends the \defgroup block at the beginning of
   the file: */

/** @} */

