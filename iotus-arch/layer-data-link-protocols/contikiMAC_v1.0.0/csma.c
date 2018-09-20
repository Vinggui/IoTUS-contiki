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

static void packet_sent(void *ptr, int status, int num_transmissions);
static void transmit_packet_list(void *ptr);
// /*---------------------------------------------------------------------------*/
// static struct neighbor_queue *
// neighbor_queue_from_addr(const linkaddr_t *addr)
// {
//   struct neighbor_queue *n = list_head(neighbor_list);
//   while(n != NULL) {
//     if(linkaddr_cmp(&n->addr, addr)) {
//       return n;
//     }
//     n = list_item_next(n);
//   }
//   return NULL;
// }
/*---------------------------------------------------------------------------*/
static clock_time_t
backoff_period(void)
{
  clock_time_t time;
  /* The retransmission time must be proportional to the channel
     check interval of the underlying radio duty cycling layer. */
  time = active_data_link_protocol.channel_check_interval();

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
  printf("trying to transmit\n");
  iotus_packet_t *packet = ptr;
  if(packet) {
    //TOODDDDDDDOOOOOO Search in packet queue
    // struct rdc_buf_list *q = list_head(packet->queued_packet_list);
    // if(q != NULL) {
      // PRINTF("csma: preparing number %d %p, queue len %d\n", packet->transmissions, q,
          // list_length(n->queued_packet_list));
      /* Send packets in the neighbor's list */

      //TOODDDDDDDOOOOOO work on the list right here
      // NETSTACK_RDC.send_list(packet_sent, n, q);
    // }
  }
}
/*---------------------------------------------------------------------------*/
static void
schedule_transmission(iotus_packet_t *packet)
{
  clock_time_t delay;
  int backoff_exponent; /* BE in IEEE 802.15.4 */

  backoff_exponent = MIN(packet->collisions, CSMA_MAX_BE);

  /* Compute max delay as per IEEE 802.15.4: 2^BE-1 backoff periods  */
  delay = ((1 << backoff_exponent) - 1) * backoff_period();
  if(delay > 0) {
    /* Pick a time for next transmission */
    delay = random_rand() % delay;
  }

  PRINTF("csma: scheduling transmission in %u ticks, NB=%u, BE=%u\n",
      (unsigned)delay, packet->collisions, backoff_exponent);
  ctimer_set(&packet->transmit_timer, delay, transmit_packet_list, packet);
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
void
csma_packet_sent(void *ptr, int status, int num_transmissions)
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
static int8_t
send_packet(iotus_packet_t *packet)
{
  static uint8_t initialized = 0;
  static uint16_t seqno;

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
  // packetbuf_set_attr(PACKETBUF_ATTR_MAC_SEQNO, seqno++);
  packet_set_sequence_number(packet, seqno++);


  schedule_transmission(packet);

  // mac_call_sent_callback(sent, ptr, MAC_TX_ERR, 1);
}
/*---------------------------------------------------------------------------*/
