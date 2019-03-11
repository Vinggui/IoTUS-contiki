
/**
 * \defgroup decription...
 *
 * This...
 *
 * @{
 */

/*
 * edytee-routing.c
 *
 *  Created on: Nov 18, 2017
 *      Author: vinicius
 */
#include <stdio.h>
#include "contiki.h"
#include "dev/leds.h"
#include "edytee_routing.h"
#include "iotus-api.h"
#include "iotus-netstack.h"
#include "layer-packet-manager.h"
#include "lib/random.h"
#include "neighbor_discovery.h"
#include "string.h"
#include "sys/timer.h"
#include "sys/rtimer.h"
#include "sys/ctimer.h"


#if USE_CSMA_MODULE == 1
  #include "tree_manager.h"
  #include "csma_contikimac.h"
#endif


#define DEBUG IOTUS_DONT_PRINT//IOTUS_PRINT_IMMEDIATELY
#define THIS_LOG_FILE_NAME_DESCRITOR "edyteeRouting"
#include "safe-printer.h"

#define RPL_DAO_PERIOD_TIME                   CONTIKIMAC_ND_PERIOD_TIME
#define RPL_ND_BACKOFF_TIME                   CONTIKIMAC_ND_BACKOFF_TIME
#define RPL_ND_SCAN_TIME                      CONTIKIMAC_ND_SCAN_TIME
#define RPL_DAO_PERIOD                        CONTIKIMAC_DAO_PERIOD
#define RPL_DAO_PERIOD_BACKOFF                CONTIKIMAC_DAO_PERIOD_BACKOFF
#define RPL_DAO_FOLLOW_DELAY                  2//sec
#define RPL_CONN_WATCHDOG                     CONTIKIMAC_WATCHDOG_TIME

//Timer for sending neighbor discovery
static struct ctimer sendNDTimer, sendDAOAckTimer, sendDAOTimer, connectionWathdog;
static struct timer NDScanTimer;
static clock_time_t backOffDifferenceDIO, backOffDifferenceDAO, randomAddTime;

static uint8_t gRoutingMsg[12];
static uint8_t gPkt_created = 0;

static iotus_node_t *gBestNode = NULL;

/*---------------------------------------------------------------------------*/
static iotus_netstack_return
send(iotus_packet_t *packet)
{
  if(NODES_BROADCAST == packet->finalDestinationNode) {
    packet->nextDestinationNode = NODES_BROADCAST;
  } else {
    //Get the final static destination
    uint8_t *finalDestLastAddress = nodes_get_address(IOTUS_ADDRESSES_TYPE_ADDR_SHORT,
                                          packet->finalDestinationNode);

    if(rootNode == packet->finalDestinationNode &&
       tree_connection_status == TREE_STATUS_CONNECTED) {
      //Get next address
      tree_next_addr_to_node_t *nextTreeNode = pieces_get_additional_info_var(
                                                    rootNode->additionalInfoList,
                                                    IOTUS_NODES_ADD_INFO_TYPE_NEXT_ADDR_TO_NODE);

      if(nextTreeNode != NULL) {
        packet->nextDestinationNode = nextTreeNode->nextNode;
      } else {
        printf("No path to root node!\n");
        return MAC_TX_ERR;
      }
    }
    // uint8_t address[2] = {1,0};
    // fatherNode = nodes_update_by_address(IOTUS_ADDRESSES_TYPE_ADDR_SHORT, address);

    // packet->nextDestinationNode = fatherNode;

    uint8_t bitSequence[1];
    bitSequence[0] = finalDestLastAddress[0];
    packet_push_bit_header(8, bitSequence, packet);
  }

#if USE_CSMA_MODULE == 1
  return csma_send_packet(packet);
#else
  return active_data_link_protocol->send(packet);
#endif
}

/*---------------------------------------------------------------------------*/
static iotus_netstack_return
send_from_up_layer(iotus_packet_t *packet)
{
  uint8_t nextType[1];
  nextType[0] = EDYTEE_COMMAND_TYPE_DATA;
  packet_push_bit_header(8, nextType, packet);
  send(packet);

  return MAC_TX_OK;
}

/*---------------------------------------------------------------------------*/
static void
reset_connection(void *ptr)
{
  // printf("Reseting conn\n");
  ctimer_stop(&sendNDTimer);
  ctimer_stop(&sendDAOAckTimer);
  ctimer_stop(&sendDAOTimer);
  ctimer_stop(&connectionWathdog);

  leds_off(LEDS_BLUE);

  fatherNode = NULL;
  tree_connection_status = TREE_STATUS_DISCONNECTED;
  treePersonalRank = 0xFF;

  gBestNode = NULL;
  // CSMA_reset();
}

/*---------------------------------------------------------------------------*/
void
edytee_reset_connection(void)
{
  printf("Reset requested\n");
  reset_connection(NULL);
}
/*---------------------------------------------------------------------------*/
static void
send_cb(iotus_packet_t *packet, iotus_netstack_return returnAns)
{
  SAFE_PRINTF_LOG_INFO("Frame %p processed %u", packet, returnAns);
  // if(returnAns == MAC_TX_OK) {
    packet_destroy(packet);
  // }

  if(returnAns != MAC_TX_OK) {
    if(tree_connection_status == TREE_STATUS_WAITING_CONFIRM) {
      reset_connection(NULL);
      return;
    }
  }
}

/*---------------------------------------------------------------------------*/
static void
sendDAOToSink(void *ptr)
{
  uint8_t daoMsg[5], nextType[1];
  memcpy(daoMsg, "#DAO", 4);
  iotus_node_t *source = ptr;
  daoMsg[0] = nodes_get_address(IOTUS_ADDRESSES_TYPE_ADDR_SHORT, source)[0];

  iotus_packet_t *packetForward = NULL;
  packetForward = iotus_initiate_packet(
                      4,
                      daoMsg,
                      PACKET_PARAMETERS_ALLOW_PIGGYBACK | PACKET_PARAMETERS_WAIT_FOR_ACK,
                      IOTUS_PRIORITY_ROUTING,
                      ROUTING_PACKETS_TIMEOUT,
                      rootNode,
                      send_cb);

  if(NULL == packetForward) {
    SAFE_PRINTF_LOG_INFO("Packet failed");
    return;
  }

  //Define this commands as
  nextType[0] = EDYTEE_COMMAND_TYPE_COMMAND_DAO_TO_SINK;
  packet_push_bit_header(8, nextType, packetForward);

  packetForward->nextDestinationNode = fatherNode;

  #if DEBUG != IOTUS_DONT_PRINT
  iotus_netstack_return status = send(packetForward);
  #else
  send(packetForward);
  #endif
  SAFE_PRINTF_LOG_INFO("DAO-followed\n");
}

/*---------------------------------------------------------------------------*/
static void
continue_dao_msg(iotus_packet_t *packet)
{
  uint8_t nextType[1];
  SAFE_PRINTF_LOG_INFO("got DAO!\n");
  //devolver dao ack e enviar DAo para o sink
  iotus_node_t *source = packet_get_prevSource_node(packet);

  iotus_packet_t *packetForward = NULL;
  packetForward = iotus_initiate_packet(
                      4,
                      (uint8_t *)"DACK",
                      PACKET_PARAMETERS_ALLOW_PIGGYBACK | PACKET_PARAMETERS_WAIT_FOR_ACK,
                      IOTUS_PRIORITY_ROUTING,
                      ROUTING_PACKETS_TIMEOUT,
                      source,
                      send_cb);

  if(NULL == packetForward) {
    SAFE_PRINTF_LOG_INFO("Packet failed");
    return;
  }

  //Define this commands as
  nextType[0] = EDYTEE_COMMAND_TYPE_COMMAND_DAO_ACK;
  packet_push_bit_header(8, nextType, packetForward);

  packetForward->nextDestinationNode = source;

  #if DEBUG != IOTUS_DONT_PRINT
  iotus_netstack_return status = send(packetForward);
  #else
  send(packetForward);
  #endif
  SAFE_PRINTF_LOG_INFO("DAO-ACK replied");

  //If not the sink node...
  if(addresses_self_get_pointer(IOTUS_ADDRESSES_TYPE_ADDR_SHORT)[0] != 1) {
    clock_time_t backoff = CLOCK_SECOND*EDYTEE_RPL_DAOACK_DELAY;//ms
    ctimer_set(&sendDAOAckTimer, backoff, sendDAOToSink, source);
  }
}

/*---------------------------------------------------------------------------*/
static void
sendPeriodicDAO(void *ptr)
{
  printf("Net sending to %u\n", 1);
  sendDAOToSink(NODES_SELF);

  backOffDifferenceDAO = (CLOCK_SECOND*((random_rand()%RPL_DAO_PERIOD_BACKOFF)))/1000;
  clock_time_t backoff = CLOCK_SECOND*RPL_DAO_PERIOD + backOffDifferenceDAO;//ms
  ctimer_set(&sendDAOTimer, backoff, sendPeriodicDAO, NULL);
}

/*---------------------------------------------------------------------------*/
static iotus_netstack_return
input_packet(iotus_packet_t *packet)
{  
  iotus_node_t *finalDestNode = NULL;
  uint8_t finalDestAddr = packet_unwrap_pushed_byte(packet);
  uint8_t netCommand = packet_unwrap_pushed_byte(packet);

  if(finalDestAddr == addresses_self_get_pointer(IOTUS_ADDRESSES_TYPE_ADDR_SHORT)[0]) {
    //This is for us...
    if(netCommand == EDYTEE_COMMAND_TYPE_COMMAND_DAO) {
      continue_dao_msg(packet);
      return RX_PROCESSED;
    } else if(netCommand == EDYTEE_COMMAND_TYPE_COMMAND_DAO_ACK) {
      ctimer_stop(&connectionWathdog);
      SAFE_PRINTF_LOG_INFO("Got DAO-ACK");

      tree_next_addr_to_node_t *nextNodePointer = pieces_modify_additional_info_var(
                          rootNode->additionalInfoList,
                          IOTUS_NODES_ADD_INFO_TYPE_NEXT_ADDR_TO_NODE,
                          sizeof(tree_next_addr_to_node_t),
                          TRUE);
      if(NULL == nextNodePointer) {
        SAFE_PRINTF_LOG_ERROR("Set");
        return MAC_TX_ERR;
      }
      tree_next_addr_to_node_t tempNode;
      tempNode.nextNode = fatherNode;
      memcpy(nextNodePointer, &tempNode, sizeof(tree_next_addr_to_node_t));


      tree_connection_status = TREE_STATUS_CONNECTED;
      leds_on(LEDS_BLUE);
      //Start our periodic DAO sends
      if(addresses_self_get_pointer(IOTUS_ADDRESSES_TYPE_ADDR_SHORT)[0] != 1) {
        backOffDifferenceDAO = (CLOCK_SECOND*((random_rand()%RPL_DAO_PERIOD_BACKOFF)))/1000;
        clock_time_t backoff = CLOCK_SECOND*RPL_DAO_PERIOD + backOffDifferenceDAO;//ms
        ctimer_set(&sendDAOTimer, backoff, sendPeriodicDAO, NULL);
      }
      return RX_PROCESSED;
    } else if(netCommand == EDYTEE_COMMAND_TYPE_COMMAND_DAO_TO_SINK) {
      // SAFE_PRINTF_LOG_INFO("Got DAO-followed %u", packet_get_payload_data(packet)[0]);
      printf("Got DAO-followed:%u\n", packet_get_payload_data(packet)[0]);
    } else {
      active_transport_protocol->receive(packet);
    }
    return RX_PROCESSED;
  }


  iotus_packet_t *packetForward = NULL;
  //search for the next node...
  // uint8_t ourAddr = addresses_self_get_pointer(IOTUS_ADDRESSES_TYPE_ADDR_SHORT)[0];

  // printf("relaying %u %u %p\n", packet_get_payload_size(packet), packet_get_payload_data(packet)[0], packet);
  SAFE_PRINTF_LOG_INFO("relaying %u %u %p", packet_get_payload_size(packet), packet_get_payload_data(packet)[0], packet);

  uint8_t address2[2] = {finalDestAddr,0};
  finalDestNode = nodes_update_by_address(IOTUS_ADDRESSES_TYPE_ADDR_SHORT, address2);
  if(finalDestNode != NULL) {
    packet_clear_parameter(packet, PACKET_PARAMETERS_PACKET_PENDING);
    packet->params |= PACKET_PARAMETERS_WAIT_FOR_ACK | PACKET_PARAMETERS_ALLOW_PIGGYBACK;

    // printf("params were %x\n", packet->params);
    packetForward = iotus_initiate_packet(
                        packet_get_payload_size(packet),
                        packet_get_payload_data(packet),
                        packet->params,
                        IOTUS_PRIORITY_ROUTING,
                        ROUTING_PACKETS_TIMEOUT,
                        finalDestNode,
                        send_cb);

    if(NULL == packetForward) {
      SAFE_PRINTF_LOG_INFO("Packet failed");
      return RX_ERR_DROPPED;
    }

    //Define this commands as
    // printf("command was %u\n", netCommand);
    packet_push_bit_header(8, &netCommand, packetForward);

    // printf("relaying size %u\n", packet_get_payload_size(packetForward));

    packetForward->nextDestinationNode = fatherNode;

    #if DEBUG != IOTUS_DONT_PRINT
    iotus_netstack_return status = send(packetForward);
    #else
    send(packetForward);
    #endif
    SAFE_PRINTF_LOG_INFO("Packet %u forwarded %u stats %u\n", packet->pktID, packetForward->pktID, status);
  }
  return RX_PROCESSED;
}
/*---------------------------------------------------------------------------*/
static void
control_frames_nd_cb(iotus_packet_t *packet, iotus_netstack_return returnAns)
{
  SAFE_PRINTF_LOG_INFO("nd %p sent %u", packet, returnAns);
  // if(returnAns == MAC_TX_OK) {
  packet_destroy(packet);
  // }
}
/*---------------------------------------------------------------------------*/
static void
send_beacon(void *ptr)
{
  clock_time_t backoff = CLOCK_SECOND*RPL_DAO_PERIOD_TIME - backOffDifferenceDIO;//ms
  backOffDifferenceDIO = (CLOCK_SECOND*((random_rand()%RPL_ND_BACKOFF_TIME)))/1000;

  backoff += backOffDifferenceDIO;
  ctimer_set(&sendNDTimer, backoff, send_beacon, NULL);

  //DIO packets have 12 bytes of base size
  sprintf((char *)gRoutingMsg, "#Rank_&_data");
  gRoutingMsg[0] = treePersonalRank;

  iotus_packet_t *packet = iotus_initiate_packet(
                            12,
                            gRoutingMsg,
                            PACKET_PARAMETERS_WAIT_FOR_ACK|PACKET_PARAMETERS_ALLOW_PIGGYBACK,
                            IOTUS_PRIORITY_ROUTING,
                            5000,
                            NODES_BROADCAST,
                            control_frames_nd_cb);

  if(NULL == packet) {
    SAFE_PRINTF_LOG_INFO("Packet failed");
    return;
  }

  SAFE_PRINTF_LOG_INFO("Creating DAO\n");
 
  SAFE_PRINTF_LOG_INFO("DIO nd %u \n", packet->pktID);
  send(packet);
}

/*---------------------------------------------------------------------------*/
static void
RPL_like_DIS_process(void *ptr)
{
  uint8_t nextType[1];
  SAFE_PRINTF_LOG_INFO("Creating DIS\n");
  nd_set_operation_msg(IOTUS_PRIORITY_ROUTING, ND_PKT_ASSOCIANTION_GET, 4, (uint8_t *)"DIS#");
  tree_connection_status = TREE_STATUS_WAITING_CONFIRM;

  if(IOTUS_PRIORITY_ROUTING == iotus_get_layer_assigned_for(IOTUS_CHORE_NEIGHBOR_DISCOVERY)) {
    //make resquest
    uint8_t *msg = nd_build_packet_type(ND_PKT_ASSOCIANTION_GET);

    iotus_packet_t *packet = iotus_initiate_packet(
                              msg[0],
                              msg,
                              PACKET_PARAMETERS_WAIT_FOR_ACK|PACKET_PARAMETERS_ALLOW_PIGGYBACK,
                              IOTUS_PRIORITY_ROUTING,
                              5000,
                              gBestNode,
                              control_frames_nd_cb);

    if(NULL == packet) {
      SAFE_PRINTF_LOG_INFO("Packet failed");
      return;
    }

    packet->nextDestinationNode = gBestNode;
   
    //Define this commands as
    nextType[0] = ND_PKT_ASSOCIANTION_REQ;
    packet_push_bit_header(8, nextType, packet);

    // active_data_link_protocol->send(packet);
    send(packet);
  }
  ctimer_set(&connectionWathdog, CLOCK_SECOND*RPL_CONN_WATCHDOG, reset_connection, NULL);
}

/*---------------------------------------------------------------------*/
static void
receive_nd_frames(struct packet_piece *packet, uint8_t type, uint8_t size, uint8_t *data)
{
  iotus_node_t *source = packet_get_prevSource_node(packet);
  uint8_t nextType[1];

  if(type == ND_PKT_BEACONS) {
    SAFE_PRINTF_LOG_INFO("Got DIO BROADCAST");

    if(tree_connection_status == TREE_STATUS_CONNECTED) {
      return;
    } else if(tree_connection_status == TREE_STATUS_SCANNING) {
      if(!timer_expired(&NDScanTimer)) {
        uint8_t sourceNodeRank = data[0];

        uint8_t *rankPointer = pieces_modify_additional_info_var(
                                    source->additionalInfoList,
                                    IOTUS_NODES_ADD_INFO_TYPE_TOPOL_TREE_RANK,
                                    1,
                                    TRUE);

        if(!rankPointer) {
          SAFE_PRINTF_LOG_ERROR("Rank ptr null");
        }

        *rankPointer = sourceNodeRank;

        if(gBestNode == NULL) {
          gBestNode = source;
          // printf("found %p\n", gBestNode);
        } else {
          rankPointer = pieces_get_additional_info_var(
                                  source->additionalInfoList,
                                  IOTUS_NODES_ADD_INFO_TYPE_TOPOL_TREE_RANK);
          if(*rankPointer < sourceNodeRank) {
            gBestNode = source;
            // printf("changing\n");
          } else {
          // printf("keeping\n");
          }
        }
      } else {
        //Select the best rank node and make a request
        // printf("t expired %p\n", gBestNode);
        if(gBestNode != NULL) {
          tree_connection_status = TREE_STATUS_BUILDING;

          if(IOTUS_PRIORITY_ROUTING == iotus_get_layer_assigned_for(IOTUS_CHORE_NEIGHBOR_DISCOVERY)) {
            randomAddTime = (CLOCK_SECOND*((random_rand()%RPL_ND_BACKOFF_TIME)))/1000;
            clock_time_t backoff = CLOCK_SECOND*RPL_DAO_PERIOD_TIME + randomAddTime;//ms
            ctimer_set(&sendNDTimer, backoff, RPL_like_DIS_process, NULL);
          } else {
            RPL_like_DIS_process(NULL);
          }
        } else {
          //Nothing found. Start over...
          timer_set(&NDScanTimer, nd_association_scan_duration);
          SAFE_PRINTF_LOG_INFO("No router found");
        }
      }
    }
  } else {
    #if DEBUG != IOTUS_DONT_PRINT
    uint8_t nodeSourceAddress = nodes_get_address(IOTUS_ADDRESSES_TYPE_ADDR_SHORT, source)[0];
    #endif

    if(type == ND_PKT_ASSOCIANTION_CONFIRM) {
      SAFE_PRINTF_LOG_INFO("DAO from %u\n", nodeSourceAddress);
      //TODO send info to application layer and confirm association

    } else if(type == ND_PKT_ASSOCIANTION_GET) {
      SAFE_PRINTF_LOG_INFO("DIS from %u\n", nodeSourceAddress);

      nd_node_nogotiating = source;

      sprintf((char *)gRoutingMsg, "#Rank_&_data");
      gRoutingMsg[0] = treePersonalRank;
      nd_set_operation_msg(IOTUS_PRIORITY_ROUTING, ND_PKT_ASSOCIANTION_ANS, 12, gRoutingMsg);

      if(IOTUS_PRIORITY_ROUTING == iotus_get_layer_assigned_for(IOTUS_CHORE_NEIGHBOR_DISCOVERY)) {
        //make resquest
        uint8_t *msg = nd_build_packet_type(ND_PKT_ASSOCIANTION_ANS);

        iotus_packet_t *packet = iotus_initiate_packet(
                                  msg[0],
                                  msg,
                                  PACKET_PARAMETERS_WAIT_FOR_ACK|PACKET_PARAMETERS_ALLOW_PIGGYBACK,
                                  IOTUS_PRIORITY_ROUTING,
                                  5000,
                                  source,
                                  control_frames_nd_cb);
        if(NULL == packet) {
          SAFE_PRINTF_LOG_INFO("Packet failed");
          return;
        }
        packet->nextDestinationNode = source;

        //Define this commands as
        nextType[0] = ND_PKT_ASSOCIANTION_ANS;
        packet_push_bit_header(8, nextType, packet);
       
        // active_data_link_protocol->send(packet);
        send(packet);
      }
    } else if(type == ND_PKT_ASSOCIANTION_ANS) {
      SAFE_PRINTF_LOG_INFO("DIO from %u\n", nodeSourceAddress);

      fatherNode = gBestNode;
      uint8_t *rankPointer = pieces_get_additional_info_var(
                                  source->additionalInfoList,
                                  IOTUS_NODES_ADD_INFO_TYPE_TOPOL_TREE_RANK);


      treePersonalRank = *rankPointer + 1;
      tree_connection_status = TREE_STATUS_CONNECTING;


      //TODO SEND DAO using tree manager
      // nd_set_operation_msg(IOTUS_PRIORITY_ROUTING, ND_PKT_ASSOCIANTION_CONFIRM, 4, (uint8_t *));
      //make resquest
      // uint8_t *msg = nd_build_packet_type(TREE_PKT_ADVERTISEMENT);

      iotus_packet_t *packet = iotus_initiate_packet(
                                4,
                                (uint8_t *)"DAO#",
                                PACKET_PARAMETERS_WAIT_FOR_ACK|PACKET_PARAMETERS_ALLOW_PIGGYBACK,
                                IOTUS_PRIORITY_ROUTING,
                                5000,
                                source,
                                control_frames_nd_cb);
      if(NULL == packet) {
        SAFE_PRINTF_LOG_INFO("Packet failed");
        return;
      }
      packet->nextDestinationNode = source;

      //Define this commands as
      nextType[0] = EDYTEE_COMMAND_TYPE_COMMAND_DAO;
      packet_push_bit_header(8, nextType, packet);
     
      // active_data_link_protocol->send(packet);
      send(packet);
      
      ctimer_restart(&connectionWathdog);

      //Now continue routing operation in the case of a router device
      if(treeRouter) {
        SAFE_PRINTF_LOG_INFO("RPL rank %u\n", treePersonalRank);

        sprintf((char *)gRoutingMsg, "#Rank_&_data");
        gRoutingMsg[0] = treePersonalRank;
        nd_set_operation_msg(IOTUS_PRIORITY_ROUTING, ND_PKT_BEACONS, 12, gRoutingMsg);

        if(IOTUS_PRIORITY_ROUTING == iotus_get_layer_assigned_for(IOTUS_CHORE_NEIGHBOR_DISCOVERY)) {
          backOffDifferenceDIO = (CLOCK_SECOND*((random_rand()%CONTIKIMAC_ND_BACKOFF_TIME)))/1000;
          clock_time_t backoff = CLOCK_SECOND*CONTIKIMAC_ND_PERIOD_TIME + backOffDifferenceDIO;//ms
          ctimer_set(&sendNDTimer, backoff, send_beacon, NULL);
        }
      }
    } else {
      //Nothing to do
    }
  }
}

/*---------------------------------------------------------------------------*/
static void
receive_piggyback_cb(iotus_packet_t *packet, uint8_t size, uint8_t *data)
{
  // SAFE_PRINTF_LOG_INFO("nd %p sent %u", packet, returnAns);
  printf("Net piggy: %s\n", data);
}

/*---------------------------------------------------------------------------*/
static void
start(void)
{
  SAFE_PRINTF_LOG_INFO("Starting edytee routing\n");

  iotus_subscribe_for_chore(IOTUS_PRIORITY_ROUTING, IOTUS_CHORE_APPLY_PIGGYBACK);
  iotus_subscribe_for_chore(IOTUS_PRIORITY_ROUTING, IOTUS_CHORE_NEIGHBOR_DISCOVERY);
  // iotus_subscribe_for_chore(IOTUS_PRIORITY_ROUTING, IOTUS_CHORE_ONEHOP_BROADCAST);
  iotus_subscribe_for_chore(IOTUS_PRIORITY_ROUTING, IOTUS_CHORE_FLOODING);
  iotus_subscribe_for_chore(IOTUS_PRIORITY_ROUTING, IOTUS_CHORE_TREE_BUILDING);
  iotus_subscribe_for_chore(IOTUS_PRIORITY_ROUTING, IOTUS_CHORE_MSG_TO_SINK);
  iotus_subscribe_for_chore(IOTUS_PRIORITY_ROUTING, IOTUS_CHORE_INSERT_PKT_P2P_SRC_ADDRESS);
  iotus_subscribe_for_chore(IOTUS_PRIORITY_ROUTING, IOTUS_CHORE_INSERT_PKT_P2P_DST_ADDRESS);

  piggyback_subscribe(IOTUS_PRIORITY_ROUTING, receive_piggyback_cb);

  nd_set_layer_operations(IOTUS_PRIORITY_ROUTING, ND_PKT_BEACONS);
  nd_set_layer_operations(IOTUS_PRIORITY_ROUTING, ND_PKT_ASSOCIANTION_REQ);
  nd_set_layer_operations(IOTUS_PRIORITY_ROUTING, ND_PKT_ASSOCIANTION_GET);
  nd_set_layer_operations(IOTUS_PRIORITY_ROUTING, ND_PKT_ASSOCIANTION_ANS);
  nd_set_layer_operations(IOTUS_PRIORITY_ROUTING, ND_PKT_ASSOCIANTION_CONFIRM);
  nd_set_layer_cb(IOTUS_PRIORITY_ROUTING, receive_nd_frames);
}

/*---------------------------------------------------------------------------*/
static void
post_start(void)
{

  // if(IOTUS_PRIORITY_ROUTING == iotus_get_layer_assigned_for(IOTUS_CHORE_APPLY_PIGGYBACK)) {
  //   SAFE_PRINTF_LOG_INFO("Network assign piggyback");
  // }
  // if(IOTUS_PRIORITY_ROUTING == iotus_get_layer_assigned_for(IOTUS_CHORE_NEIGHBOR_DISCOVERY)) {
  //   SAFE_PRINTF_LOG_INFO("Network assign ND");
  // }

#if EXP_CONTIKIMAC_802_15_4 == 1
  if(treeRouter &&
     addresses_self_get_pointer(IOTUS_ADDRESSES_TYPE_ADDR_SHORT)[0] == 1) {
    //This is the root...

    if(IOTUS_PRIORITY_ROUTING == iotus_get_layer_assigned_for(IOTUS_CHORE_TREE_BUILDING)) {
      SAFE_PRINTF_LOG_INFO("Network assign tree");
      treePersonalRank = 1;
    }
    if(IOTUS_PRIORITY_ROUTING == iotus_get_layer_assigned_for(IOTUS_CHORE_NEIGHBOR_DISCOVERY)) {
      SAFE_PRINTF_LOG_INFO("Network assign ND");

      //Start beacons in ND for deployment of DIO msg
      backOffDifferenceDIO = (CLOCK_SECOND*((random_rand()%RPL_ND_BACKOFF_TIME)))/1000;
      clock_time_t backoff = CLOCK_SECOND*RPL_DAO_PERIOD_TIME + backOffDifferenceDIO;//ms
      ctimer_set(&sendNDTimer, backoff, send_beacon, NULL);
    } else {
      //Set DIO msg right away
      sprintf((char *)gRoutingMsg, "#Rank_&_data");
      gRoutingMsg[0] = treePersonalRank;
      nd_set_operation_msg(IOTUS_PRIORITY_ROUTING, ND_PKT_BEACONS, 12, gRoutingMsg);
    }
  } else {
    tree_connection_status = TREE_STATUS_SCANNING;
    timer_set(&NDScanTimer, CLOCK_SECOND*CONTIKIMAC_ND_SCAN_TIME);
  }
#endif /* EXP_CONTIKIMAC_802_15_4 */
}

static void
close(void)
{
  
}

const struct iotus_network_protocol_struct edytee_routing_protocol = {
  start,
  post_start,
  close,
  send_from_up_layer,
  send_cb,
  input_packet
};

/* The following stuff ends the \defgroup block at the beginning of
   the file: */

/** @} */
