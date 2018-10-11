/**
 * \addtogroup staticnet
 * @{
 */

#define DEBUG 0
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif


#include <stdio.h>
#include "net/linkaddr.h"
#include "net/packetbuf.h"
#include "net/netstack.h"
#include "lib/memb.h"
#include "lib/mmem.h"
#include "sys/ctimer.h"
#include "lib/list.h"
#include "aggregation.h"


MEMB(queued_packets_memb, struct aggregation_queueitem, CSMA_CONF_MAX_PACKET_PER_NEIGHBOR);
LIST(gAggregationFramesList);
LIST(gAggregationFramesInsertedList);


/*---------------------------------------------------------------------------*/
void
destroy_aggregation_frame(struct aggregation_queueitem *p)
{
  if(p == NULL) {
    return;
  }

  ctimer_stop(&(p->timeout));

  list_remove(gAggregationFramesList, p);
  list_remove(gAggregationFramesInsertedList, p);
  mmem_free(&(p->data));
  memb_free(&queued_packets_memb,p);
  PRINTF("Aggr frame destroyed!\n");
}


/*---------------------------------------------------------------------*/
/*
 */
static void
timeout_handler(void *ptr) {
  struct aggregation_queueitem *h = ptr;
  PRINTF("AGGR timedout\n");
  destroy_aggregation_frame(h);
  (h->mac_callback)(NULL,0,0);
}

/*---------------------------------------------------------------------------*/
uint8_t
create_aggregation_frame(const uint8_t *msg, uint8_t size, linkaddr_t *node_addr, mac_callback_t mac_callback, uint32_t timeout)
{
  struct aggregation_queueitem *p;
  p = memb_alloc(&queued_packets_memb);
  if(p != NULL) {
    //Item created
    if(mmem_alloc(&(p->data), size) == 0) {
      PRINTF("Alloc aggr mmem prob");
      memb_free(&queued_packets_memb, p);
      return 0;
    }
    if(NULL != msg) {
      memcpy(p->data.ptr, msg, size);
    } else {
      memset(p->data.ptr, 0, size);
    }
  }
  p->mac_callback = mac_callback;
  linkaddr_copy(&(p->node_addr), node_addr);

  ctimer_set(&(p->timeout), timeout, timeout_handler, p);


  list_push(gAggregationFramesList, p);
  PRINTF("Aggreg frame created\n");
  return 1;
}

/*---------------------------------------------------------------------------*/
static uint8_t
insert_aggregation_frame(struct aggregation_queueitem *h)
{
  if(packetbuf_remaininglen() >= h->data.size+2) {
    uint16_t len = packetbuf_datalen();
    packetbuf_set_datalen(len+h->data.size+2);
    uint8_t *pointer = packetbuf_dataptr()+len;
    uint8_t i = 0;
    
    for(; i<h->data.size; i++) {
      pointer[i] = ((uint8_t *)h->data.ptr)[h->data.size-i-1];
    }

    pointer += h->data.size;
    *pointer = 0xFF;
    pointer++;
    *pointer = 0xFF;

    destroy_aggregation_frame(h);
    return 1;
  }
  return 0;
}

/*---------------------------------------------------------------------------*/
void
aggregation_apply(void)
{
  //Look for header pieces that match this packet conditions
  struct aggregation_queueitem *h;
  struct aggregation_queueitem *nextH;
  uint8_t piecesInserted = 0;
  uint16_t packetOldSize;

  packetOldSize = packetbuf_totlen();

  if(packetbuf_datalen() > 25) {
    //TODO remove this to add more pieces...
    return;
  }

  h = list_head(gAggregationFramesList);
  if(h == NULL) {
    PRINTF("Aggr search NULL\n");
  }
  while(h != NULL) {
    nextH = list_item_next(h);
    PRINTF("Aggr search ok\n");
    if(linkaddr_cmp(&(h->node_addr), packetbuf_addr(PACKETBUF_ADDR_RECEIVER))) {
      PRINTF("Aggr found\n");
      if(insert_aggregation_frame(h)) {
        piecesInserted++;
        //TODO remove this break to add more pieces...
        break;
      }
    }
    h = nextH;
  }

  return;
}

/*---------------------------------------------------------------------------*/
void
aggregation_init(void)
{
  memb_init(&queued_packets_memb);
  list_init(gAggregationFramesList);
  list_init(gAggregationFramesInsertedList);
}
