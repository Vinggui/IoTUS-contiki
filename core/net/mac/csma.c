/*
 * Copyright (c) 2010, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 */

/**
 * \file
 *         A Carrier Sense Multiple Access (CSMA) MAC layer
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "dev/leds.h"
#include "net/mac/csma.h"
#include "net/packetbuf.h"
#include "net/queuebuf.h"

#include "sys/ctimer.h"
#include "sys/clock.h"

#include "lib/random.h"

#include "net/netstack.h"

#include "lib/list.h"
#include "lib/memb.h"

#include <string.h>

#include <stdio.h>

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else /* DEBUG */
#define PRINTF(...)
#endif /* DEBUG */

/* Constants of the IEEE 802.15.4 standard */

/* macMinBE: Initial backoff exponent. Range 0--CSMA_MAX_BE */
#ifdef CSMA_CONF_MIN_BE
#define CSMA_MIN_BE CSMA_CONF_MIN_BE
#else
#define CSMA_MIN_BE 0
#endif

/* macMaxBE: Maximum backoff exponent. Range 3--8 */
#ifdef CSMA_CONF_MAX_BE
#define CSMA_MAX_BE CSMA_CONF_MAX_BE
#else
#define CSMA_MAX_BE 4
#endif

/* macMaxCSMABackoffs: Maximum number of backoffs in case of channel busy/collision. Range 0--5 */
#ifdef CSMA_CONF_MAX_BACKOFF
#define CSMA_MAX_BACKOFF CSMA_CONF_MAX_BACKOFF
#else
#define CSMA_MAX_BACKOFF 5
#endif

/* macMaxFrameRetries: Maximum number of re-transmissions attampts. Range 0--7 */
#ifdef CSMA_CONF_MAX_FRAME_RETRIES
#define CSMA_MAX_MAX_FRAME_RETRIES CSMA_CONF_MAX_FRAME_RETRIES
#else
#define CSMA_MAX_MAX_FRAME_RETRIES 7
#endif

/* Packet metadata */
struct qbuf_metadata {
  mac_callback_t sent;
  void *cptr;
  uint8_t max_transmissions;
};

/* Every neighbor has its own packet queue */
struct neighbor_queue {
  struct neighbor_queue *next;
  linkaddr_t addr;
  struct ctimer transmit_timer;
  uint8_t transmissions;
  uint8_t collisions;
#if EXP_CONTIKIMAC_802_15_4 == 1
  uint8_t treeRank;
#endif
  LIST_STRUCT(queued_packet_list);
};

/* The maximum number of co-existing neighbor queues */
#ifdef CSMA_CONF_MAX_NEIGHBOR_QUEUES
#define CSMA_MAX_NEIGHBOR_QUEUES CSMA_CONF_MAX_NEIGHBOR_QUEUES
#else
#define CSMA_MAX_NEIGHBOR_QUEUES 2
#endif /* CSMA_CONF_MAX_NEIGHBOR_QUEUES */

/* The maximum number of pending packet per neighbor */
#ifdef CSMA_CONF_MAX_PACKET_PER_NEIGHBOR
#define CSMA_MAX_PACKET_PER_NEIGHBOR CSMA_CONF_MAX_PACKET_PER_NEIGHBOR
#else
#define CSMA_MAX_PACKET_PER_NEIGHBOR MAX_QUEUED_PACKETS
#endif /* CSMA_CONF_MAX_PACKET_PER_NEIGHBOR */

#define MAX_QUEUED_PACKETS QUEUEBUF_NUM
MEMB(neighbor_memb, struct neighbor_queue, CSMA_MAX_NEIGHBOR_QUEUES);
MEMB(packet_memb, struct rdc_buf_list, MAX_QUEUED_PACKETS);
MEMB(metadata_memb, struct qbuf_metadata, MAX_QUEUED_PACKETS);
LIST(neighbor_list);

/*
 * Modification to install the 802.15.4 registering idea...
*/
#if EXP_CONTIKIMAC_802_15_4 == 1
#define NUMARGS(...)  (sizeof((uint8_t[]){0, ##__VA_ARGS__})/sizeof(uint8_t)-1)
#define STATIC_COORDINATORS_NUM       NUMARGS(STATIC_COORDINATORS)
static uint8_t treeRouter = 0;
static uint8_t treeRouterNodes[] = {STATIC_COORDINATORS};
static uint8_t treePersonalRank = 0xFF;
static linkaddr_t treeRoot;
static linkaddr_t treeFather;
static uint8_t treeFatherRank = 0xFF;


static uint8_t private_nd_control[12];
//Timer for sending neighbor discovery
static struct ctimer sendNDTimer, connectionWathdog;
static linkaddr_t gBestNode;
static uint8_t gBestNodeRank = 0xFF;
static clock_time_t backOffDifference, randomAddTime;

static struct timer NDScanTimer;
typedef enum {
  DATA_LINK_ND_CONNECTION_STATUS_DISCONNECTED,
  DATA_LINK_ND_CONNECTION_STATUS_CONNECTED,
  DATA_LINK_ND_CONNECTION_STATUS_WAITING_REGISTER,
  DATA_LINK_ND_CONNECTION_STATUS_WAITING_ANSWER,
  DATA_LINK_ND_CONNECTION_STATUS_WAITING_CONFIRMATION
} csma_connection_status;
static csma_connection_status gConnectionStatus;

static void control_frames_nd_cb(void *ptr, int status, int num_transmissions);
#endif /* EXP_CONTIKIMAC_802_15_4 == 1 */

static void packet_sent(void *ptr, int status, int num_transmissions);
static void transmit_packet_list(void *ptr);
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
static clock_time_t
backoff_period(void)
{
  clock_time_t time;
  /* The retransmission time must be proportional to the channel
     check interval of the underlying radio duty cycling layer. */
  time = NETSTACK_RDC.channel_check_interval();

  /* If the radio duty cycle has no channel check interval, we use
   * the default in IEEE 802.15.4: aUnitBackoffPeriod which is
   * 20 symbols i.e. 320 usec. That is, 1/3125 second. */
  if(time == 0) {
    time = MAX(CLOCK_SECOND / 3125, 1);
  }
  return time;
}
/*---------------------------------------------------------------------------*/
static void
transmit_packet_list(void *ptr)
{
  struct neighbor_queue *n = ptr;
  if(n) {
    struct rdc_buf_list *q = list_head(n->queued_packet_list);
    if(q != NULL) {
      PRINTF("csma: preparing number %d %p, queue len %d\n", n->transmissions, q,
          list_length(n->queued_packet_list));
      /* Send packets in the neighbor's list */
      NETSTACK_RDC.send_list(packet_sent, n, q);
    }
  }
}
/*---------------------------------------------------------------------------*/
static void
schedule_transmission(struct neighbor_queue *n)
{
  clock_time_t delay;
  int backoff_exponent; /* BE in IEEE 802.15.4 */

  // backoff_exponent = MIN(n->collisions, CSMA_MAX_BE);
  backoff_exponent = CSMA_MAX_BE;

  /* Compute max delay as per IEEE 802.15.4: 2^BE-1 backoff periods  */
  delay = ((1 << backoff_exponent) - 1) * backoff_period();
  if(delay > 0) {
    /* Pick a time for next transmission */
    delay = random_rand() % delay;
  }

  PRINTF("csma: scheduling transmission in %u ticks, NB=%u, BE=%u\n",
      (unsigned)delay, n->collisions, backoff_exponent);
  ctimer_set(&n->transmit_timer, delay, transmit_packet_list, n);
}
/*---------------------------------------------------------------------------*/
static void
free_packet(struct neighbor_queue *n, struct rdc_buf_list *p, int status)
{
  if(p != NULL) {
    /* Remove packet from list and deallocate */
    list_remove(n->queued_packet_list, p);

    queuebuf_free(p->buf);
    memb_free(&metadata_memb, p->ptr);
    memb_free(&packet_memb, p);
    PRINTF("csma: free_queued_packet, queue length %d, free packets %d\n",
           list_length(n->queued_packet_list), memb_numfree(&packet_memb));
    if(list_head(n->queued_packet_list) != NULL) {
      /* There is a next packet. We reset current tx information */
      n->transmissions = 0;
      n->collisions = CSMA_MIN_BE;
      /* Schedule next transmissions */
      schedule_transmission(n);
    } else {
      /* This was the last packet in the queue, we free the neighbor */
      ctimer_stop(&n->transmit_timer);
      list_remove(neighbor_list, n);
      memb_free(&neighbor_memb, n);
    }
  }
}
/*---------------------------------------------------------------------------*/
static void
tx_done(int status, struct rdc_buf_list *q, struct neighbor_queue *n)
{
  mac_callback_t sent;
  struct qbuf_metadata *metadata;
  void *cptr;
  uint8_t ntx;

  metadata = (struct qbuf_metadata *)q->ptr;
  sent = metadata->sent;
  cptr = metadata->cptr;
  ntx = n->transmissions;

  switch(status) {
  case MAC_TX_OK:
    PRINTF("csma: rexmit ok %d\n", n->transmissions);
    break;
  case MAC_TX_COLLISION:
  case MAC_TX_NOACK:
    PRINTF("csma: drop with status %d after %d transmissions, %d collisions\n",
                 status, n->transmissions, n->collisions);
    break;
  default:
    PRINTF("csma: rexmit failed %d: %d\n", n->transmissions, status);
    break;
  }

  free_packet(n, q, status);
  mac_call_sent_callback(sent, cptr, status, ntx);
}
/*---------------------------------------------------------------------------*/
static void
rexmit(struct rdc_buf_list *q, struct neighbor_queue *n)
{
  schedule_transmission(n);
  /* This is needed to correctly attribute energy that we spent
     transmitting this packet. */
  queuebuf_update_attr_from_packetbuf(q->buf);
}
/*---------------------------------------------------------------------------*/
static void
collision(struct rdc_buf_list *q, struct neighbor_queue *n,
          int num_transmissions)
{
  struct qbuf_metadata *metadata;

  metadata = (struct qbuf_metadata *)q->ptr;

  n->collisions += num_transmissions;

  if(n->collisions > CSMA_MAX_BACKOFF) {
    n->collisions = CSMA_MIN_BE;
    /* Increment to indicate a next retry */
    n->transmissions++;
  }

  if(n->transmissions >= metadata->max_transmissions) {
    tx_done(MAC_TX_COLLISION, q, n);
  } else {
    PRINTF("csma: rexmit collision %d\n", n->transmissions);
    rexmit(q, n);
  }
}
/*---------------------------------------------------------------------------*/
static void
noack(struct rdc_buf_list *q, struct neighbor_queue *n, int num_transmissions)
{
  struct qbuf_metadata *metadata;

  metadata = (struct qbuf_metadata *)q->ptr;

  n->collisions = CSMA_MIN_BE;
  n->transmissions += num_transmissions;

  if(n->transmissions >= metadata->max_transmissions) {
    tx_done(MAC_TX_NOACK, q, n);
  } else {
    PRINTF("csma: rexmit noack %d\n", n->transmissions);
    rexmit(q, n);
  }
}
/*---------------------------------------------------------------------------*/
static void
tx_ok(struct rdc_buf_list *q, struct neighbor_queue *n, int num_transmissions)
{
  n->collisions = CSMA_MIN_BE;
  n->transmissions += num_transmissions;
  tx_done(MAC_TX_OK, q, n);
}
/*---------------------------------------------------------------------------*/
static void
packet_sent(void *ptr, int status, int num_transmissions)
{
  struct neighbor_queue *n;
  struct rdc_buf_list *q;

  n = ptr;
  if(n == NULL) {
    return;
  }

  /* Find out what packet this callback refers to */
  for(q = list_head(n->queued_packet_list);
      q != NULL; q = list_item_next(q)) {
    if(queuebuf_attr(q->buf, PACKETBUF_ATTR_MAC_SEQNO) ==
       packetbuf_attr(PACKETBUF_ATTR_MAC_SEQNO)) {
      break;
    }
  }

  if(q == NULL) {
    PRINTF("csma: seqno %d not found\n",
           packetbuf_attr(PACKETBUF_ATTR_MAC_SEQNO));
    return;
  } else if(q->ptr == NULL) {
    PRINTF("csma: no metadata\n");
    return;
  }

  switch(status) {
  case MAC_TX_OK:
    tx_ok(q, n, num_transmissions);
    break;
  case MAC_TX_NOACK:
    noack(q, n, num_transmissions);
    break;
  case MAC_TX_COLLISION:
    collision(q, n, num_transmissions);
    break;
  case MAC_TX_DEFERRED:
    break;
  default:
    tx_done(status, q, n);
    break;
  }
}
/*---------------------------------------------------------------------------*/
static void
send_packet(mac_callback_t sent, void *ptr)
{
  struct rdc_buf_list *q;
  struct neighbor_queue *n;
  static uint8_t initialized = 0;
  static uint16_t seqno;
  const linkaddr_t *addr = packetbuf_addr(PACKETBUF_ADDR_RECEIVER);
  if(!initialized) {
    initialized = 1;
    /* Initialize the sequence number to a random value as per 802.15.4. */
    seqno = random_rand();
  }

  if(seqno == 0) {
    /* PACKETBUF_ATTR_MAC_SEQNO cannot be zero, due to a pecuilarity
       in framer-802154.c. */
    seqno++;
  }
  packetbuf_set_attr(PACKETBUF_ATTR_MAC_SEQNO, seqno++);

  /* Look for the neighbor entry */
  n = neighbor_queue_from_addr(addr);
  if(n == NULL) {
    /* Allocate a new neighbor entry */
    n = memb_alloc(&neighbor_memb);
    if(n != NULL) {
      /* Init neighbor entry */
      linkaddr_copy(&n->addr, addr);
      n->transmissions = 0;
      n->collisions = CSMA_MIN_BE;
      /* Init packet list for this neighbor */
      LIST_STRUCT_INIT(n, queued_packet_list);
      /* Add neighbor to the list */
      list_add(neighbor_list, n);
    }
  }

  if(n != NULL) {
#if EXP_CONTIKIMAC_802_15_4 == 1
    n->treeRank = 0xFF;
#endif
    /* Add packet to the neighbor's queue */
    if(list_length(n->queued_packet_list) < CSMA_MAX_PACKET_PER_NEIGHBOR) {
      q = memb_alloc(&packet_memb);
      if(q != NULL) {
        q->ptr = memb_alloc(&metadata_memb);
        if(q->ptr != NULL) {
          q->buf = queuebuf_new_from_packetbuf();
          if(q->buf != NULL) {
            struct qbuf_metadata *metadata = (struct qbuf_metadata *)q->ptr;
            /* Neighbor and packet successfully allocated */
            if(packetbuf_attr(PACKETBUF_ATTR_MAX_MAC_TRANSMISSIONS) == 0) {
              /* Use default configuration for max transmissions */
              metadata->max_transmissions = CSMA_MAX_MAX_FRAME_RETRIES + 1;
            } else {
              metadata->max_transmissions =
                packetbuf_attr(PACKETBUF_ATTR_MAX_MAC_TRANSMISSIONS);
            }
            metadata->sent = sent;
            metadata->cptr = ptr;
#if PACKETBUF_WITH_PACKET_TYPE
            if(packetbuf_attr(PACKETBUF_ATTR_PACKET_TYPE) ==
               PACKETBUF_ATTR_PACKET_TYPE_ACK) {
              list_push(n->queued_packet_list, q);
            } else
#endif
            {
              list_add(n->queued_packet_list, q);
            }

            PRINTF("csma: send_packet, queue length %d, free packets %d\n",
                   list_length(n->queued_packet_list), memb_numfree(&packet_memb));
            /* If q is the first packet in the neighbor's queue, send asap */
            if(list_head(n->queued_packet_list) == q) {
              schedule_transmission(n);
            }
            return;
          }
          memb_free(&metadata_memb, q->ptr);
          PRINTF("csma: could not allocate queuebuf, dropping packet\n");
        }
        memb_free(&packet_memb, q);
        PRINTF("csma: could not allocate queuebuf, dropping packet\n");
      }
      /* The packet allocation failed. Remove and free neighbor entry if empty. */
      if(list_length(n->queued_packet_list) == 0) {
        list_remove(neighbor_list, n);
        memb_free(&neighbor_memb, n);
      }
    } else {
      PRINTF("csma: Neighbor queue full\n");
    }
    PRINTF("csma: could not allocate packet, dropping packet\n");
  } else {
    PRINTF("csma: could not allocate neighbor, dropping packet\n");
  }
  mac_call_sent_callback(sent, ptr, MAC_TX_ERR, 1);
}

/*---------------------------------------------------------------------------*/
#if EXP_CONTIKIMAC_802_15_4 == 1
static void
reset_connection(void)
{
  if(treeRouter &&
     linkaddr_node_addr.u8[0] == 1) {
    return;
  }
  ctimer_stop(&sendNDTimer);
  ctimer_stop(&connectionWathdog);
  gConnectionStatus = DATA_LINK_ND_CONNECTION_STATUS_DISCONNECTED;


  leds_off(LEDS_ALL);
  treeRouter = 0;
  treePersonalRank = 0xFF;
  linkaddr_copy(&treeRoot, &linkaddr_null);
  linkaddr_copy(&treeFather, &linkaddr_null);
  treeFatherRank = 0xFF;
  linkaddr_copy(&gBestNode, &linkaddr_null);
  static uint8_t gBestNodeRank = 0xFF;

  NETSTACK_RDC.off(1);
  timer_set(&NDScanTimer, CLOCK_SECOND*CONTIKIMAC_ND_SCAN_TIME);
}

/*---------------------------------------------------------------------------*/
void
csma_802like_answer_process(void *ptr){
  //ready to request asnwer from router
  if(!linkaddr_cmp(&gBestNode, &linkaddr_null)) {
    //make resquest

    packetbuf_copyfrom("answer", 6);

    packetbuf_set_addr(PACKETBUF_ADDR_RECEIVER, &gBestNode);
    packetbuf_set_addr(PACKETBUF_ADDR_SENDER, &linkaddr_node_addr);

    packetbuf_set_attr(PACKETBUF_ATTR_FRAME_TYPE, FRAME802154_CMDFRAME);
    packetbuf_compact();
    send_packet(control_frames_nd_cb, NULL);

    // contikiMAC_back_on();
    gConnectionStatus = DATA_LINK_ND_CONNECTION_STATUS_WAITING_CONFIRMATION;
    ctimer_restart(&connectionWathdog);
  } else {
    PRINTF("No router found");
  }
}

/*---------------------------------------------------------------------------*/
void
csma_802like_register_process(void *ptr){
  NETSTACK_RDC.on();
  // printf("register\n");
  packetbuf_copyfrom("register", 8);

  //Address null is Broadcast
  packetbuf_set_addr(PACKETBUF_ADDR_RECEIVER, &gBestNode);
  packetbuf_set_addr(PACKETBUF_ADDR_SENDER, &linkaddr_node_addr);

  packetbuf_set_attr(PACKETBUF_ATTR_FRAME_TYPE, FRAME802154_CMDFRAME);
  packetbuf_compact();
  send_packet(control_frames_nd_cb, NULL);

  gConnectionStatus = DATA_LINK_ND_CONNECTION_STATUS_WAITING_ANSWER;
  

  randomAddTime = (CLOCK_SECOND*((random_rand()%CONTIKIMAC_ND_BACKOFF_TIME)))/1000;
  clock_time_t backoff = CLOCK_SECOND + randomAddTime;//ms
  ctimer_set(&sendNDTimer, backoff, csma_802like_answer_process, NULL);
  ctimer_set(&connectionWathdog, CLOCK_SECOND*CONTIKIMAC_WATCHDOG_TIME, reset_connection, NULL);
}

/*---------------------------------------------------------------------------*/
static void
control_frames_nd_cb(void *ptr, int status, int num_transmissions)
{
  PRINTF("nd send");
  // printf("got ansrwe %u %u\n", status, num_transmissions);
  if(status != MAC_TX_OK) {
    if(gConnectionStatus == DATA_LINK_ND_CONNECTION_STATUS_WAITING_ANSWER) {
      reset_connection();
    }
  }
}

/*---------------------------------------------------------------------------*/
static void
send_beacon(void *ptr)
{
  clock_time_t backoff = CLOCK_SECOND*CONTIKIMAC_ND_PERIOD_TIME - backOffDifference;//ms
  backOffDifference = (CLOCK_SECOND*((random_rand()%CONTIKIMAC_ND_BACKOFF_TIME)))/1000;

  backoff += backOffDifference;
  ctimer_set(&sendNDTimer, backoff, send_beacon, NULL);

  packetbuf_copyfrom(&treePersonalRank, 1);

  //Address null is Broadcast
  packetbuf_set_addr(PACKETBUF_ADDR_RECEIVER, &linkaddr_null);
  packetbuf_set_addr(PACKETBUF_ADDR_SENDER, &linkaddr_node_addr);

  packetbuf_set_attr(PACKETBUF_ATTR_FRAME_TYPE, FRAME802154_BEACONFRAME);
  packetbuf_compact();
  send_packet(control_frames_nd_cb, NULL);
  PRINTF("Beacon nd \n");
}

/*---------------------------------------------------------------------------*/
static void
csma_control_frame_receive(void)
{
  struct neighbor_queue *n;
  linkaddr_t sourceAddr;
  linkaddr_copy(&sourceAddr, packetbuf_addr(PACKETBUF_ADDR_SENDER));

  if(packetbuf_attr(PACKETBUF_ATTR_FRAME_TYPE) == FRAME802154_BEACONFRAME) {
    if(gConnectionStatus == DATA_LINK_ND_CONNECTION_STATUS_CONNECTED) {
      //Ignore msg
    } else if(gConnectionStatus == DATA_LINK_ND_CONNECTION_STATUS_WAITING_ANSWER) {
      //Nothing to do      
    } else if(gConnectionStatus == DATA_LINK_ND_CONNECTION_STATUS_WAITING_CONFIRMATION) {
      //Nothing to do now
    } else {
      // printf("disco\n");
      if(!timer_expired(&NDScanTimer)) {
        // printf("Time ok\n");
        uint8_t *data = packetbuf_dataptr();
        uint8_t sourceNodeRank = data[0];

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
        // printf("t expired\n");
        if(!linkaddr_cmp(&gBestNode, &linkaddr_null)) {
          // printf("Have option\n");
          //make resquest
          gConnectionStatus = DATA_LINK_ND_CONNECTION_STATUS_WAITING_REGISTER;
          
          randomAddTime = (CLOCK_SECOND*((random_rand()%CONTIKIMAC_ND_BACKOFF_TIME)))/1000;
          clock_time_t backoff = randomAddTime;//ms
          ctimer_set(&sendNDTimer, backoff, csma_802like_register_process, NULL);
        } else {
          // printf("No option returng\n");
          //Nothing found. Start over...
          timer_set(&NDScanTimer, CLOCK_SECOND*CONTIKIMAC_ND_SCAN_TIME);
          leds_on(LEDS_RED);
          PRINTF("No router found");
        }
      }
    }
  } else if(packetbuf_attr(PACKETBUF_ATTR_FRAME_TYPE) == FRAME802154_CMDFRAME) {
    uint8_t *data = packetbuf_dataptr();
    uint8_t commandType = data[0];

    if(commandType == 'r') {
      PRINTF("register cmm from %u\n", sourceAddr.u8[0]);
      //TODO send info to application layer and confirm association
    } else if(commandType == 'a') {
      PRINTF("answer cmm from %u\n", sourceAddr.u8[0]);
      //Address null is Broadcast

      packetbuf_copyfrom("join", 4);
      packetbuf_set_addr(PACKETBUF_ADDR_RECEIVER, &sourceAddr);
      packetbuf_set_addr(PACKETBUF_ADDR_SENDER, &linkaddr_node_addr);


      packetbuf_set_attr(PACKETBUF_ATTR_FRAME_TYPE, FRAME802154_CMDFRAME);
      packetbuf_compact();
      send_packet(control_frames_nd_cb, NULL);

    } else if(commandType == 'c') {
      PRINTF("confirm cmm from %u\n", sourceAddr.u8[0]);

    } else if(commandType == 'j') {
      ctimer_stop(&connectionWathdog);
      PRINTF("join cmm from %u\n", sourceAddr.u8[0]);
      linkaddr_copy(&treeRoot, &gBestNode);

      treePersonalRank = gBestNodeRank + 1;
      gConnectionStatus = DATA_LINK_ND_CONNECTION_STATUS_CONNECTED;
      leds_off(LEDS_RED);
      leds_on(LEDS_GREEN);
      printf("Connected CSMA rank %u\n", treePersonalRank);

      //Now continue routing operation in the case of a router device
      if(treeRouter) {

        backOffDifference = (CLOCK_SECOND*((random_rand()%CONTIKIMAC_ND_BACKOFF_TIME)))/1000;
        clock_time_t backoff = CLOCK_SECOND*CONTIKIMAC_ND_PERIOD_TIME + backOffDifference;//ms
        ctimer_set(&sendNDTimer, backoff, send_beacon, NULL);
      }
    } else {
      //Nothing to do
    }
  } else {
    NETSTACK_LLSEC.input();
  }
}
#endif
/*---------------------------------------------------------------------------*/
static void
input_packet(void)
{
  if(packetbuf_holds_broadcast() ||
     packetbuf_attr(PACKETBUF_ATTR_FRAME_TYPE) == FRAME802154_CMDFRAME) {
    csma_control_frame_receive();
  } else {
    NETSTACK_LLSEC.input();
  }
}
/*---------------------------------------------------------------------------*/
static int
on(void)
{
  return NETSTACK_RDC.on();
}
/*---------------------------------------------------------------------------*/
static int
off(int keep_radio_on)
{
  return NETSTACK_RDC.off(keep_radio_on);
}
/*---------------------------------------------------------------------------*/
static unsigned short
channel_check_interval(void)
{
  if(gConnectionStatus != DATA_LINK_ND_CONNECTION_STATUS_CONNECTED) {
    return 0;
  }
  if(NETSTACK_RDC.channel_check_interval) {
    return NETSTACK_RDC.channel_check_interval();
  }
  return 0;
}

/*---------------------------------------------------------------------------*/
static void
init(void)
{
  memb_init(&packet_memb);
  memb_init(&metadata_memb);
  memb_init(&neighbor_memb);

#if EXP_CONTIKIMAC_802_15_4 == 1
  linkaddr_copy(&gBestNode, &linkaddr_null);
  uint8_t tempAddr[] = STATIC_ROOT_ADDRESS;
  treeRoot.u8[0] = tempAddr[0];
  treeRoot.u8[1] = tempAddr[1];
  
  uint8_t i=0;
  for(; i<STATIC_COORDINATORS_NUM; i++) {
    if(linkaddr_node_addr.u8[0] == treeRouterNodes[i]) {
      treeRouter = 1;
      PRINTF("I'm router!\n");
    }
  }

  gConnectionStatus = DATA_LINK_ND_CONNECTION_STATUS_DISCONNECTED;
  if(treeRouter &&
     linkaddr_node_addr.u8[0] == 1) {
    //This is the root...
    treePersonalRank = 1;
    gConnectionStatus = DATA_LINK_ND_CONNECTION_STATUS_CONNECTED;

    NETSTACK_RDC.on();

    backOffDifference = (CLOCK_SECOND*((random_rand()%CONTIKIMAC_ND_BACKOFF_TIME)))/1000;
    clock_time_t backoff = CLOCK_SECOND*CONTIKIMAC_ND_PERIOD_TIME + backOffDifference;//ms
    ctimer_set(&sendNDTimer, backoff, send_beacon, NULL);
  } else {
    NETSTACK_RDC.off(1);
    timer_set(&NDScanTimer, CLOCK_SECOND*CONTIKIMAC_ND_SCAN_TIME);
  }
#endif /* EXP_CONTIKIMAC_802_15_4 */
}
/*---------------------------------------------------------------------------*/
const struct mac_driver csma_driver = {
  "CSMA",
  init,
  send_packet,
  input_packet,
  on,
  off,
  channel_check_interval,
};
/*---------------------------------------------------------------------------*/
