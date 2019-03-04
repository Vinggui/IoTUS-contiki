/**
 * \addtogroup rpl-like-net
 * @{
 */

#define DEBUG 0
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif


#include <stdio.h>
#include "contikimac.h"
#include "dev/leds.h"
#include "net/linkaddr.h"
#include "net/netstack.h"
#include "net/rime/rime.h"
#include "net/mac/mac.h"
#include "random.h"
#include "rpl-like-net.h"
#include "sys/ctimer.h"
#include "aggregation.h"

#include "lib/list.h"


#ifdef RIME_CONF_BROADCAST_ANNOUNCEMENT_CHANNEL
#define BROADCAST_ANNOUNCEMENT_CHANNEL RIME_CONF_BROADCAST_ANNOUNCEMENT_CHANNEL
#else /* RIME_CONF_BROADCAST_ANNOUNCEMENT_CHANNEL */
#define BROADCAST_ANNOUNCEMENT_CHANNEL 2
#endif /* RIME_CONF_BROADCAST_ANNOUNCEMENT_CHANNEL */

#ifdef RIME_CONF_BROADCAST_ANNOUNCEMENT_BUMP_TIME
#define BROADCAST_ANNOUNCEMENT_BUMP_TIME RIME_CONF_BROADCAST_ANNOUNCEMENT_BUMP_TIME
#else /* RIME_CONF_BROADCAST_ANNOUNCEMENT_BUMP_TIME */
#define BROADCAST_ANNOUNCEMENT_BUMP_TIME CLOCK_SECOND * 32 / NETSTACK_RDC_CHANNEL_CHECK_RATE
#endif /* RIME_CONF_BROADCAST_ANNOUNCEMENT_BUMP_TIME */

#ifdef RIME_CONF_BROADCAST_ANNOUNCEMENT_MIN_TIME
#define BROADCAST_ANNOUNCEMENT_MIN_TIME RIME_CONF_BROADCAST_ANNOUNCEMENT_MIN_TIME
#else /* RIME_CONF_BROADCAST_ANNOUNCEMENT_MIN_TIME */
#define BROADCAST_ANNOUNCEMENT_MIN_TIME CLOCK_SECOND * 60
#endif /* RIME_CONF_BROADCAST_ANNOUNCEMENT_MIN_TIME */

#ifdef RIME_CONF_BROADCAST_ANNOUNCEMENT_MAX_TIME
#define BROADCAST_ANNOUNCEMENT_MAX_TIME RIME_CONF_BROADCAST_ANNOUNCEMENT_MAX_TIME
#else /* RIME_CONF_BROADCAST_ANNOUNCEMENT_MAX_TIME */
#define BROADCAST_ANNOUNCEMENT_MAX_TIME CLOCK_SECOND * 3600UL
#endif /* RIME_CONF_BROADCAST_ANNOUNCEMENT_MAX_TIME */

#if STANDARD_CONTIKI_WITH_SERVICES == 0
  #if USE_NEW_FEATURES == 1
    #error "New features service is ON when standard contiki service is OFF!"
  #endif
#endif

#define NUMARGS(...)  (sizeof((uint8_t[]){0, ##__VA_ARGS__})/sizeof(uint8_t)-1)
#define STATIC_COORDINATORS_NUM       NUMARGS(STATIC_COORDINATORS)

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
rtimer_clock_t packetBuildingTime;
// uint8_t ticTocFlag = 0;
static clock_time_t backOffDifferenceDIO, backOffDifferenceDAO, randomAddTime;

static uint8_t gRoutingMsg[12];
static uint8_t gPkt_created = 0;

static void (* up_msg_confirm)(int status, int num_tx) = NULL;
static void (* up_msg_input)(const linkaddr_t *source) = NULL;

//RPL tree stuff
static uint8_t gPersonalTreeRank = 0xFF;
static tree_manager_conn_status gTreeStatus;

static uint8_t treeRouter = 0;
static uint8_t treeRouterNodes[] = {STATIC_COORDINATORS};
static linkaddr_t gRPLTreeRoot;
static linkaddr_t gRPLTreeFather;
static uint8_t gRPLTreeFatherRank = 0xFF;
static linkaddr_t gBestNode;
static uint8_t gBestNodeRank = 0xFF;

static uint8_t gData_link_is_on = 0;

/* Every neighbor has its own packet queue */
struct neighbor_queue {
  struct neighbor_queue *next;
  linkaddr_t addr;
  linkaddr_t nextAddr;
  uint8_t treeRank;
};

/* The maximum number of co-existing neighbor queues */
#ifdef CSMA_CONF_MAX_NEIGHBOR_QUEUES
#define RPL_MAX_NEIGHBOR_QUEUES CSMA_CONF_MAX_NEIGHBOR_QUEUES
#else
#define RPL_MAX_NEIGHBOR_QUEUES 2
#endif /* CSMA_CONF_MAX_NEIGHBOR_QUEUES */

MEMB(neighbor_memb, struct neighbor_queue, RPL_MAX_NEIGHBOR_QUEUES);
LIST(neighbor_list);

static void check_data_link_connection(void *ptr);
/*---------------------------------------------------------------------------*/
static struct neighbor_queue *
neighbor_queue_from_addr(const linkaddr_t *addr)
{
  struct neighbor_queue *n = list_head(neighbor_list);
  while(n != NULL) {
    if(linkaddr_cmp(&n->addr, addr)) {
      return n;
    }
    n = list_item_next(n);
  }
  return NULL;
}

/*---------------------------------------------------------------------------*/
static void
reset_connection(void)
{
  // printf("Reseting conn\n");
  ctimer_stop(&sendNDTimer);
  ctimer_stop(&sendDAOAckTimer);
  ctimer_stop(&sendDAOTimer);
  ctimer_stop(&connectionWathdog);

  leds_off(LEDS_BLUE);

  gTreeStatus = TREE_STATUS_DISCONNECTED;

  gPersonalTreeRank = 0xFF;

  gRPLTreeFatherRank = 0xFF;
  linkaddr_copy(&gRPLTreeRoot, &linkaddr_null);
  linkaddr_copy(&gRPLTreeFather, &linkaddr_null);
  linkaddr_copy(&gBestNode, &linkaddr_null);
   gBestNodeRank = 0xFF;

  gData_link_is_on = 0;

  check_data_link_connection(NULL);
}

/*---------------------------------------------------------------------------*/
static void
packet_sent(void *ptr, int status, int num_tx)
{
  if(status != MAC_TX_OK) {
    if(gTreeStatus == TREE_STATUS_WAITING_ASNWER ||
       gTreeStatus == TREE_STATUS_WAITING_CONFIRM) {
      reset_connection();
      return;
    }
  }

  switch(status) {
  case MAC_TX_COLLISION:
    PRINTF("rpllikenet: collision after %d tx\n", num_tx);
    break; 
  case MAC_TX_NOACK:
    PRINTF("rpllikenet: noack after %d tx\n", num_tx);
    break;
  case MAC_TX_OK:
    PRINTF("rpllikenet: sent after %d tx\n", num_tx);
    break;
  default:
    PRINTF("rpllikenet: error %d after %d tx\n", status, num_tx);
  }
  up_msg_confirm(status, num_tx);
}

/*---------------------------------------------------------------------------*/
int
rpllikenet_output(void)
{
  RIMESTATS_ADD(tx);

  linkaddr_t *finalReceiver = packetbuf_addr(PACKETBUF_ADDR_RECEIVER);
  if(packetbuf_hdralloc(1)) {
    uint8_t *buf = packetbuf_hdrptr();
    buf[0] = finalReceiver->u8[0];
  } else {
    PRINTF("Failed to create packet");
    return 0;
  }

  static linkaddr_t addrNext;

  /* Look for the neighbor entry */
  struct neighbor_queue *n = neighbor_queue_from_addr(finalReceiver);
  if(n == NULL) {
    //Nothing found.... try to send to this address anyway
    // printf("aquui\n");
    linkaddr_copy(&addrNext, finalReceiver);
  } else {
    linkaddr_copy(&addrNext, &n->nextAddr);
  }
  // printf("era %u enviando %u %u\n", finalReceiver->u8[0], addrNext.u8[0], addrNext.u8[1]);

  packetbuf_set_addr(PACKETBUF_ADDR_RECEIVER, &addrNext);

  packetbuf_compact();
  NETSTACK_LLSEC.send(packet_sent, NULL);
  return 1;
}

/*---------------------------------------------------------------------------*/
static void
RPL_like_DIS_process(void *ptr)
{
  // printf("Creating DIS msg\n");
  //DIS packets have 4 bytes of base size
  packetbuf_copyfrom("DIS#", 4);

  //Address null is Broadcast
  packetbuf_set_addr(PACKETBUF_ADDR_RECEIVER, &gBestNode);
  packetbuf_set_addr(PACKETBUF_ADDR_SENDER, &linkaddr_node_addr);

  if(packetbuf_hdralloc(1)) {
    uint8_t *buf = packetbuf_hdrptr();
    buf[0] = EDYTEE_COMMAND_TYPE_COMMAND_DIS;
  } else {
    PRINTF("Failed to create packet");
    return;
  }
  packetbuf_compact();
  rpllikenet_output();

  gTreeStatus = TREE_STATUS_WAITING_ASNWER;
  ctimer_set(&connectionWathdog, CLOCK_SECOND*RPL_CONN_WATCHDOG, reset_connection, NULL);
}

/*---------------------------------------------------------------------------*/
static void
create_DIO_msg(linkaddr_t *addrToSend)
{
  //DIO packets have 12 bytes of base size
  sprintf((char *)gRoutingMsg, "#Rank_&_data");
  gRoutingMsg[0] = gPersonalTreeRank;

  packetbuf_copyfrom(gRoutingMsg, 12);

  if(packetbuf_hdralloc(1)) {
    uint8_t *buf = packetbuf_hdrptr();
    buf[0] = EDYTEE_COMMAND_TYPE_COMMAND_DIO_BROADCAST;
  } else {
    PRINTF("Failed to create packet");
    return;
  }

  //Address null is Broadcast
  packetbuf_set_addr(PACKETBUF_ADDR_RECEIVER, addrToSend);
  packetbuf_set_addr(PACKETBUF_ADDR_SENDER, &linkaddr_node_addr);

  rpllikenet_output();
 
  PRINTF("DIO Broadcast created\n");
}

/*---------------------------------------------------------------------------*/
static void
send_DIO(void *ptr)
{
  clock_time_t backoff = CLOCK_SECOND*RPL_DAO_PERIOD_TIME - backOffDifferenceDIO;//ms
  backOffDifferenceDIO = (CLOCK_SECOND*((random_rand()%RPL_ND_BACKOFF_TIME)))/1000;
  backoff += backOffDifferenceDIO;

  ctimer_set(&sendNDTimer, backoff, send_DIO, NULL);

  //Address null is Broadcast
  create_DIO_msg(&linkaddr_null);
}

/*---------------------------------------------------------------------------*/
static void
sendDAOToSink(void *ptr)
{
  struct neighbor_queue *n = ptr;
  PRINTF("DAO to sink\n");

  // //Address null is Broadcast

  sprintf((char *)gRoutingMsg, "#DAO");
  gRoutingMsg[0] = n->addr.u8[0];

  packetbuf_copyfrom(gRoutingMsg, 4);
  packetbuf_set_addr(PACKETBUF_ADDR_RECEIVER, &gRPLTreeRoot);
  packetbuf_set_addr(PACKETBUF_ADDR_SENDER, &linkaddr_node_addr);


  if(packetbuf_hdralloc(1)) {
    uint8_t *buf = packetbuf_hdrptr();
    buf[0] = EDYTEE_COMMAND_TYPE_COMMAND_DAO_TO_SINK;
  } else {
    PRINTF("Failed to create packet");
    return;
  }

  packetbuf_compact();
  rpllikenet_output();
}

/*---------------------------------------------------------------------------*/
static void
sendPeriodicDAO(void *ptr)
{
  PRINTF("DAO to sink\n");

  // //Address null is Broadcast

  sprintf((char *)gRoutingMsg, "#DAO");
  gRoutingMsg[0] = linkaddr_node_addr.u8[0];

  packetbuf_copyfrom(gRoutingMsg, 4);
  packetbuf_set_addr(PACKETBUF_ADDR_RECEIVER, &gRPLTreeRoot);
  packetbuf_set_addr(PACKETBUF_ADDR_SENDER, &linkaddr_node_addr);


  if(packetbuf_hdralloc(1)) {
    uint8_t *buf = packetbuf_hdrptr();
    buf[0] = EDYTEE_COMMAND_TYPE_COMMAND_DAO_TO_SINK;
  } else {
    PRINTF("Failed to create packet");
    return;
  }

  packetbuf_compact();
  rpllikenet_output();

  backOffDifferenceDAO = (CLOCK_SECOND*((random_rand()%RPL_DAO_PERIOD_BACKOFF)))/1000;
  clock_time_t backoff = CLOCK_SECOND*RPL_DAO_PERIOD + backOffDifferenceDAO;//ms
  ctimer_set(&sendDAOTimer, backoff, sendPeriodicDAO, NULL);
}

/*---------------------------------------------------------------------*/
static void
receive_nd_frames(uint8_t finalDestAddr, uint8_t netCommand)
{
  struct neighbor_queue *n;
  linkaddr_t sourceAddr;
  uint8_t weAreRoot = 0;
  linkaddr_copy(&sourceAddr, packetbuf_addr(PACKETBUF_ADDR_SENDER));
  // printf("addresses1 %u %u | %u %u\n", gRPLTreeRoot.u8[0], gRPLTreeRoot.u8[1], linkaddr_node_addr.u8[0], linkaddr_node_addr.u8[1]);
  if(linkaddr_cmp(&gRPLTreeRoot, &linkaddr_node_addr)) {
    weAreRoot = 1;
  }


  if(netCommand == EDYTEE_COMMAND_TYPE_COMMAND_DIO_BROADCAST) {
    if(gTreeStatus == TREE_STATUS_CONNECTED) {
      //Ignore msg
    } else if(gTreeStatus == TREE_STATUS_WAITING_ASNWER) {
      //Nothing to do      
    } else if(gTreeStatus == TREE_STATUS_WAITING_CONFIRM) {
      //Nothing to do now
    } else {
      // printf("disco\n");
      if(!timer_expired(&NDScanTimer)) {
        // printf("Time ok\n");
        uint8_t *data = packetbuf_dataptr();
        uint8_t sourceNodeRank = data[0];
        // printf("heard RPL rank %u\n", sourceNodeRank);

        if(linkaddr_cmp(&gBestNode, &linkaddr_null)) {
          linkaddr_copy(&gBestNode, &sourceAddr);
          gBestNodeRank = sourceNodeRank;
          // printf("found neighbor\n");
        } else if(sourceNodeRank < gBestNodeRank) {
            linkaddr_copy(&gBestNode, &sourceAddr);
            gBestNodeRank = sourceNodeRank;
            // printf("changing\n");
        } else {
        // printf("keeping\n");
        }
      } else {
        // printf("Time over\n");
        //Select the best rank node and make a request
        if(!linkaddr_cmp(&gBestNode, &linkaddr_null)) {
          // printf("Have option %u %u\n", gBestNode.u8[1], gBestNode.u8[0]);
          //make resquest

          randomAddTime = (CLOCK_SECOND*((random_rand()%RPL_ND_BACKOFF_TIME)))/1000;
          clock_time_t backoff = (random_rand()%RPL_DAO_PERIOD_TIME)*CLOCK_SECOND*RPL_DAO_PERIOD_TIME + randomAddTime;//ms
          ctimer_set(&sendNDTimer, backoff, RPL_like_DIS_process, NULL);
        } else {
          // printf("No option returng2\n");
          //Nothing found. Start over...
          timer_set(&NDScanTimer, CLOCK_SECOND*RPL_ND_SCAN_TIME);
          leds_on(LEDS_RED);
          PRINTF("No router found");
        }
      }
    }
  } else if(netCommand == EDYTEE_COMMAND_TYPE_COMMAND_DIS) {
    PRINTF("DIS cmm from %u\n", sourceAddr.u8[0]);
    //make resquest
    sprintf((char *)gRoutingMsg, "#Rank_&_data");
    gRoutingMsg[0] = gPersonalTreeRank;

    packetbuf_copyfrom(gRoutingMsg, 12);
    packetbuf_set_addr(PACKETBUF_ADDR_RECEIVER, &sourceAddr);
    packetbuf_set_addr(PACKETBUF_ADDR_SENDER, &linkaddr_node_addr);


    if(packetbuf_hdralloc(1)) {
      uint8_t *buf = packetbuf_hdrptr();
      buf[0] = EDYTEE_COMMAND_TYPE_COMMAND_DIO;
    } else {
      PRINTF("Failed to create packet");
      return;
    }

    packetbuf_compact();
    rpllikenet_output();
  } else if(netCommand == EDYTEE_COMMAND_TYPE_COMMAND_DIO) {
    PRINTF("DIO cmm from %u\n", sourceAddr.u8[0]);
    // //Address null is Broadcast

    sprintf((char *)gRoutingMsg, "#DAO");
    gRoutingMsg[0] = linkaddr_node_addr.u8[0];

    packetbuf_copyfrom(gRoutingMsg, 4);
    packetbuf_set_addr(PACKETBUF_ADDR_RECEIVER, &gBestNode);
    packetbuf_set_addr(PACKETBUF_ADDR_SENDER, &linkaddr_node_addr);


    if(packetbuf_hdralloc(1)) {
      uint8_t *buf = packetbuf_hdrptr();
      buf[0] = EDYTEE_COMMAND_TYPE_COMMAND_DAO;
    } else {
      PRINTF("Failed to create packet");
      return;
    }

    gTreeStatus = TREE_STATUS_WAITING_CONFIRM;

    ctimer_restart(&connectionWathdog);
    packetbuf_compact();
    rpllikenet_output();

  } else if(netCommand == EDYTEE_COMMAND_TYPE_COMMAND_DAO) {
    PRINTF("DAO cmm from %u\n", sourceAddr.u8[0]);
    
    /* Look for the neighbor entry */
    struct neighbor_queue *n = neighbor_queue_from_addr(&sourceAddr);
    if(n == NULL) {
      /* Allocate a new neighbor entry */
      n = memb_alloc(&neighbor_memb);
      if(n != NULL) {
        /* Init neighbor entry */
        linkaddr_copy(&n->addr, &sourceAddr);
        list_add(neighbor_list, n);
        n->treeRank = gPersonalTreeRank + 1;
        linkaddr_copy(&n->nextAddr, &sourceAddr);
      }
    }

    if(n != NULL) {

      //If not the sink node... Continue this DAO to sink
      // printf("addresses %u %u | %u %u\n", gRPLTreeRoot.u8[0], gRPLTreeRoot.u8[1], linkaddr_node_addr.u8[0], linkaddr_node_addr.u8[1]);
      if(weAreRoot == 0) {
        clock_time_t backoff = CLOCK_SECOND*RPL_DAO_FOLLOW_DELAY;//ms
        ctimer_set(&sendDAOAckTimer, backoff, sendDAOToSink, n);
      }

      //make resquest
      packetbuf_copyfrom("DACK", 4);
      packetbuf_set_addr(PACKETBUF_ADDR_RECEIVER, &sourceAddr);
      packetbuf_set_addr(PACKETBUF_ADDR_SENDER, &linkaddr_node_addr);


      if(packetbuf_hdralloc(1)) {
        uint8_t *buf = packetbuf_hdrptr();
        buf[0] = EDYTEE_COMMAND_TYPE_COMMAND_DAO_ACK;
      } else {
        PRINTF("Failed to create packet");
        return;
      }

      packetbuf_compact();
      rpllikenet_output();
    } else {
      printf("No neighbor slot...\n");
    }
  } else if(netCommand == EDYTEE_COMMAND_TYPE_COMMAND_DAO_ACK) {
    PRINTF("DAO ACK from %u\n", sourceAddr.u8[0]);
    linkaddr_copy(&gRPLTreeFather, &gBestNode);

    /* Look for the neighbor entry */
    struct neighbor_queue *n = neighbor_queue_from_addr(&gRPLTreeFather);
    if(n == NULL) {
      /* Allocate a new neighbor entry */
      n = memb_alloc(&neighbor_memb);
      if(n != NULL) {
        /* Init neighbor entry */
        linkaddr_copy(&n->addr, &gBestNode);
        list_add(neighbor_list, n);
        n->treeRank = gBestNodeRank;
        linkaddr_copy(&n->nextAddr, &gBestNode);
      }
    }

    if(n != NULL) {
      n = neighbor_queue_from_addr(&gRPLTreeRoot);
      if(n == NULL) {
        /* Allocate a new neighbor entry */
        n = memb_alloc(&neighbor_memb);
        if(n != NULL) {
          /* Init neighbor entry */
          linkaddr_copy(&n->addr, &gRPLTreeRoot);
          list_add(neighbor_list, n);
          n->treeRank = 1;
          linkaddr_copy(&n->nextAddr, &gBestNode);
        }
      }

      if(n != NULL) {
        ctimer_stop(&connectionWathdog);
        gPersonalTreeRank = gBestNodeRank + 1;
        gTreeStatus = TREE_STATUS_CONNECTED;
        NETSTACK_MAC.on();
        leds_off(LEDS_RED);
        leds_on(LEDS_BLUE);
        printf("Connected RPL rank %u\n", gPersonalTreeRank);

        //Now continue routing operation in the case of a router device
        if(treeRouter) {

          backOffDifferenceDIO = (CLOCK_SECOND*((random_rand()%RPL_ND_BACKOFF_TIME)))/1000;
          clock_time_t backoff = CLOCK_SECOND*RPL_DAO_PERIOD_TIME + backOffDifferenceDIO;//ms
          ctimer_set(&sendNDTimer, backoff, send_DIO, NULL);
        }

        //Start our periodic DAO sends
        if(weAreRoot == 0) {
          PRINTF("Start DAO periodic\n");
          backOffDifferenceDAO = (CLOCK_SECOND*((random_rand()%RPL_DAO_PERIOD_BACKOFF)))/1000;
          clock_time_t backoff = CLOCK_SECOND*RPL_DAO_PERIOD + backOffDifferenceDAO;//ms
          ctimer_set(&sendDAOTimer, backoff, sendPeriodicDAO, NULL);
        }
      }
    }
  } else if(netCommand == EDYTEE_COMMAND_TYPE_COMMAND_DAO_TO_SINK) {
    uint8_t DAOsender = ((uint8_t *)packetbuf_dataptr())[0];
    if(finalDestAddr == linkaddr_node_addr.u8[0]) {
      uint8_t DAOsender = ((uint8_t *)packetbuf_dataptr())[0];
      printf("Got relayed DAO %u\n", DAOsender);

      //TODO add this node to the list...
    } else {
      PRINTF("Foward DAO");

      sprintf((char *)gRoutingMsg, "#DAO");
      gRoutingMsg[0] = DAOsender;

      packetbuf_copyfrom(gRoutingMsg, 4);
      static linkaddr_t addrFinal;
      addrFinal.u8[0] = finalDestAddr;
      addrFinal.u8[1] = 0;
      packetbuf_set_addr(PACKETBUF_ADDR_RECEIVER, &addrFinal);
      packetbuf_set_addr(PACKETBUF_ADDR_SENDER, &linkaddr_node_addr);


      if(packetbuf_hdralloc(1)) {
        uint8_t *buf = packetbuf_hdrptr();
        buf[0] = EDYTEE_COMMAND_TYPE_COMMAND_DAO_TO_SINK;
      } else {
        PRINTF("Failed to create packet");
        return;
      }

      rpllikenet_output();
    }
  }
}

/*---------------------------------------------------------------------------*/
static void
input(void)
{
  uint8_t *data = packetbuf_dataptr();
  uint8_t finalDestAddr = data[0];
  uint8_t routingCommand = data[1];
  // printf("commands %u %u\n", finalDestAddr, routingCommand);

  if(gData_link_is_on == 0) {
    //Data link is still connecting
    return;
  }

  packetbuf_hdrreduce(2);
  if(packetbuf_holds_broadcast()) {
    //nothing
  } else if(finalDestAddr == linkaddr_node_addr.u8[0]) {
    if(routingCommand == EDYTEE_COMMAND_TYPE_DATA) {
      up_msg_input(packetbuf_addr(PACKETBUF_ADDR_SENDER));
      return;
    }
  }
  
  receive_nd_frames(finalDestAddr, routingCommand);
    // static linkaddr_t addrFinal;
    // addrFinal.u8[0] = finalDestAddr;
    // addrFinal.u8[1] = 0;
    // packetbuf_set_addr(PACKETBUF_ADDR_RECEIVER, &addrFinal);
    // packetbuf_set_addr(PACKETBUF_ADDR_SENDER, &linkaddr_node_addr);

    // rpllikenet_output();
}

/*---------------------------------------------------------------------------*/
void
rpllikenet_signup(void (* msg_confirm)(int status, int num_tx), void (* msg_input)(const linkaddr_t *source))
{
  up_msg_confirm = msg_confirm;
  up_msg_input = msg_input;
}

/*---------------------------------------------------------------------------*/
static void
check_data_link_connection(void *ptr)
{
  if(NETSTACK_MAC.channel_check_interval() != 0) {
      gData_link_is_on = 1;
    if(treeRouter &&
       linkaddr_cmp(&gRPLTreeRoot, &linkaddr_node_addr)) {
      gPersonalTreeRank = 1;
      gTreeStatus = TREE_STATUS_CONNECTED;

      backOffDifferenceDIO = (CLOCK_SECOND*((random_rand()%RPL_ND_BACKOFF_TIME)))/1000;
      clock_time_t backoff = CLOCK_SECOND*RPL_DAO_PERIOD_TIME + backOffDifferenceDIO;//ms
      ctimer_set(&sendNDTimer, backoff, send_DIO, NULL);
    } else {
      NETSTACK_MAC.off(1);
      gTreeStatus = TREE_STATUS_SCANNING;
      timer_set(&NDScanTimer, CLOCK_SECOND*RPL_ND_SCAN_TIME);
    }
  } else {
    clock_time_t backoff = CLOCK_SECOND;//ms
    ctimer_set(&sendNDTimer, backoff, check_data_link_connection, NULL);
  }
}

/*---------------------------------------------------------------------------*/
static void
init(void)
{
  PRINTF("rpl-like-net started\n");
  queuebuf_init();
  memb_init(&neighbor_memb);

  packetbuf_clear();

  linkaddr_copy(&gBestNode, &linkaddr_null);
  uint8_t tempAddr[] = STATIC_ROOT_ADDRESS;
  gRPLTreeRoot.u8[0] = tempAddr[0];
  gRPLTreeRoot.u8[1] = tempAddr[1];
  
  uint8_t i=0;
  for(; i<STATIC_COORDINATORS_NUM; i++) {
    if(linkaddr_node_addr.u8[0] == treeRouterNodes[i]) {
      treeRouter = 1;
      PRINTF("I'm router!\n");
    }
  }

  gTreeStatus = TREE_STATUS_DISCONNECTED;
  check_data_link_connection(NULL);
}


/*---------------------------------------------------------------------------*/
const struct network_driver rpllikenet_driver = {
  "rpl-like-net",
  init,
  input
};
/** @} */
