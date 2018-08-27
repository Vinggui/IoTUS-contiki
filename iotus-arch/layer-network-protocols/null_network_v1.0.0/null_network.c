
/**
 * \defgroup description...
 *
 * This...
 *
 * @{
 */

/*
 * null-routing.c
 *
 *  Created on: Nov 18, 2017
 *      Author: vinicius
 */
#include <stdio.h>
#include "contiki.h"
#include "iotus-api.h"
#include "iotus-netstack.h"
#include "piggyback.h"
#include "sys/ctimer.h"
#include "random.h"

#define DEBUG IOTUS_PRINT_IMMEDIATELY//IOTUS_DONT_PRINT//IOTUS_PRINT_IMMEDIATELY
#define THIS_LOG_FILE_NAME_DESCRITOR "nullRouting"
#include "safe-printer.h"

// Next dest table using final value{source, final destination}
int routing_table[13][13] =
{
  //0=> 1    2    3    4    5    6    7    8    9   10   11   12
  {0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0},
  //1=>
  {0,   0,   2,   3,   2,   2,   3,   3,   3,   2,   3,   3,   3},
  //2=>
  {0,   1,   0,   3,   4,   5,   3,   3,   3,   4,   3,   3,   3},
  //3=>
  {0,   1,   2,   0,   2,   2,   6,   7,   8,   2,   6,   6,   6},
  //4=>
  {0,   2,   2,   2,   0,   5,   2,   2,   2,   9,   2,   2,   2},
  //5=>
  {0,   2,   2,   2,   4,   0,   2,   2,   2,   4,   2,   2,   2},
  //6=>
  {0,   3,   3,   3,   3,   3,   0,   7,   8,   3,  10,  10,  10},
  //7=>
  {0,   3,   3,   3,   3,   3,   6,   0,   8,   3,   6,   6,   6},
  //8=>
  {0,   3,   3,   3,   3,   3,   6,   7,   0,   3,   6,   6,   6},
  //9=>
  {0,   4,   4,   4,   4,   4,   4,   4,   4,   0,   4,   4,   4},
  //10=>
  {0,   6,   6,   6,   6,   6,   6,   6,   6,   6,   0,  11,  12},
  //11=>
  {0,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,   0,  10},
  //12=>
  {0,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  11,   0}
};

//Timer for sending neighbor discovery
static struct ctimer sendNDTimer;
iotus_node_t *rootNode;
iotus_node_t *fatherNode;
static uint8_t private_keep_alive[12];


static iotus_netstack_return
send(iotus_packet_t *packet)
{
  if(NODES_BROADCAST == packet->finalDestinationNode) {
    packet->nextDestinationNode = NODES_BROADCAST;
  } else {
    //Get the final static destination
    uint8_t *finalDestLastAddress = nodes_get_address(IOTUS_ADDRESSES_TYPE_ADDR_SHORT,
                                          packet->finalDestinationNode);
#if EXP_STAR_LIKE == 1
    uint8_t nextHop = 1;
#else
    if(addresses_self_get_pointer(IOTUS_ADDRESSES_TYPE_ADDR_SHORT)[0] > 12 ||
       finalDestLastAddress[0] > 12) {
      SAFE_PRINTF_LOG_ERROR("wrong destination");
      return ROUTING_TX_ERR;
    }
    uint8_t nextHop = routing_table[addresses_self_get_pointer(IOTUS_ADDRESSES_TYPE_ADDR_SHORT)[0]][finalDestLastAddress[0]];
#endif

    if(nextHop == 0) {
      //This is for ourself. Cancel...
      return ROUTING_TX_ERR;
    }

    SAFE_PRINTF_LOG_INFO("Final %u next %u\n",finalDestLastAddress[0],nextHop);

    // uint8_t addressNext[2] = {nextHop,0};
    // nextHopNode = nodes_update_by_address(IOTUS_ADDRESSES_TYPE_ADDR_SHORT, addressNext);

    // if(NULL == nextHopNode) {
      // SAFE_PRINTF_LOG_ERROR("No next hop");
      // return ROUTING_TX_ERR;
    // }


    packet->nextDestinationNode = fatherNode;

    uint8_t bitSequence[1];
    bitSequence[0] = finalDestLastAddress[0];
    packet_push_bit_header(8, bitSequence, packet);
  }

  return active_data_link_protocol->send(packet);
}


static void
send_cb(iotus_packet_t *packet, iotus_netstack_return returnAns)
{
  SAFE_PRINTF_LOG_INFO("Frame %p processed %u", packet, returnAns);
  printf("vtc!!\n");
  packet_destroy(packet);
}

static iotus_netstack_return
input_packet(iotus_packet_t *packet)
{
  // SAFE_PRINTF_CLEAN("Got packet: ");
  // int i;
  // for (i = 0; i < packet_get_payload_size(packet); ++i)
  // {
  //   SAFE_PRINTF_CLEAN("%02x ", packet_get_payload_data(packet)[i]);
  // }
  // SAFE_PRINTF_CLEAN("\n");
  uint8_t finalDestAddr = packet_unwrap_pushed_byte(packet);

  if(finalDestAddr == addresses_self_get_pointer(IOTUS_ADDRESSES_TYPE_ADDR_SHORT)[0]) {
    //This is for us...

    return RX_PROCESSED;
  } else {
    iotus_packet_t *packetForward = NULL;


    //search for the next node...
    uint8_t ourAddr = addresses_self_get_pointer(IOTUS_ADDRESSES_TYPE_ADDR_SHORT)[0];
    uint8_t nextHop = routing_table[ourAddr][finalDestAddr];

    if(nextHop != 0) {
        if(rootNode != NULL) {
          packetForward = packet_create_msg(
                            packet_get_payload_size(packet),
                            packet_get_payload_data(packet),
                            IOTUS_PRIORITY_ROUTING,
                            ROUTING_PACKETS_TIMEOUT,
                            TRUE,
                            rootNode);

          if(NULL == packetForward) {
            SAFE_PRINTF_LOG_INFO("Packet failed");
            return RX_ERR_DROPPED;
          }
          packet_set_parameter(packetForward, packet->params | PACKET_PARAMETERS_WAIT_FOR_ACK);
          packet_set_confirmation_cb(packet, send_cb);
          SAFE_PRINTF_LOG_INFO("Packet %p forwarded into %p", packet, packetForward);

          
          iotus_netstack_return status = send(packetForward);
            printf("uhhhhhaaaaa?! %p \n",packet->confirm_cb);
          if (!(MAC_TX_OK == status ||
              MAC_TX_DEFERRED == status)) {
            packet_destroy(packet);
          }
        }
    }
    return RX_PROCESSED;
  }

}



/*---------------------------------------------------------------------------*/
static void
send_keep_alive(void *ptr)
{
  static uint8_t selfAddrValue;
  selfAddrValue = addresses_self_get_pointer(IOTUS_ADDRESSES_TYPE_ADDR_SHORT)[0];

  if(selfAddrValue != 1) {
    clock_time_t backoff = CLOCK_SECOND*(KEEP_ALIVE_INTERVAL) +(CLOCK_SECOND*(random_rand()%BACKOFF_TIME))/1000;//ms
    ctimer_set(&sendNDTimer, backoff, send_keep_alive, NULL);

#if USE_NEW_FEATURES == 1
    SAFE_PRINTF_LOG_INFO("Creating piggy routing\n");
    piggyback_create_piece(12, private_keep_alive, IOTUS_PRIORITY_ROUTING, rootNode, ROUTING_PACKETS_TIMEOUT);
#else
    SAFE_PRINTF_LOG_INFO("Create KA alone\n");
    if(rootNode != NULL) {
        // iotus_initiate_msg(
        //         12,
        //         private_keep_alive,
        //         PACKET_PARAMETERS_WAIT_FOR_ACK,
        //         IOTUS_PRIORITY_APPLICATION,
        //         5000,
        //         rootNode);


        packet = packet_create_msg(
                      12,
                      private_keep_alive,
                      IOTUS_PRIORITY_APPLICATION,
                      5000,
                      TRUE,
                      rootNode);

        if(NULL == packet) {
          return NULL;
        }
        packet_set_parameter(packet,PACKET_PARAMETERS_WAIT_FOR_ACK);
        packet_set_confirmation_cb(packet, send_cb);

        iotus_netstack_return status = send(packet);
        if (!(MAC_TX_OK ==  status||
            MAC_TX_DEFERRED == status)) {
          printf("FAILED NET?!\n");
          packet_destroy(packet);
        }
      }
#endif
  }
}

/*---------------------------------------------------------------------------*/
static void
start(void)
{
  SAFE_PRINTF_LOG_INFO("Starting null routing\n");

  iotus_subscribe_for_chore(IOTUS_PRIORITY_ROUTING, IOTUS_CHORE_NEIGHBOR_DISCOVERY);

  uint8_t rootValue = 0;
#if EXP_STAR_LIKE == 0
  rootValue = routing_table[addresses_self_get_pointer(IOTUS_ADDRESSES_TYPE_ADDR_SHORT)[0]][1];
#else
  rootValue = 1;
#endif

  uint8_t address[2] = {rootValue,0};
  fatherNode = nodes_update_by_address(IOTUS_ADDRESSES_TYPE_ADDR_SHORT, address);

  uint8_t address2[2] = {1,0};
  rootNode = nodes_update_by_address(IOTUS_ADDRESSES_TYPE_ADDR_SHORT, address2);

  uint8_t selfAddrValue;
  selfAddrValue = addresses_self_get_pointer(IOTUS_ADDRESSES_TYPE_ADDR_SHORT)[0];
  sprintf((char *)private_keep_alive, "### %02u %02u###", selfAddrValue,
                                                        selfAddrValue);
}

/*---------------------------------------------------------------------------*/
static void
post_start(void)
{
#if BROADCAST_EXAMPLE == 0
  if(IOTUS_PRIORITY_ROUTING == iotus_get_layer_assigned_for(IOTUS_CHORE_NEIGHBOR_DISCOVERY)) {
    clock_time_t backoff = CLOCK_SECOND*(KEEP_ALIVE_INTERVAL) +(CLOCK_SECOND*(random_rand()%BACKOFF_TIME))/1000;//ms
    ctimer_set(&sendNDTimer, backoff, send_keep_alive, NULL);
  }
#endif
}

static void
close(void)
{
  
}

struct iotus_network_protocol_struct null_network_protocol = {
  start,
  post_start,
  close,
  send,
  send_cb,
  input_packet
};
/* The following stuff ends the \defgroup block at the beginning of
   the file: */

/** @} */
