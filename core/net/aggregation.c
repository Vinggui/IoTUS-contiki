/**
 * \addtogroup staticnet
 * @{
 */

#define DEBUG 1
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif


#include <stdio.h>
#include "net/linkaddr.h"
#include "net/netstack.h"
#include "lib/memb.h"
#include "lib/mmem.h"
#include "sys/ctimer.h"
#include "lib/list.h"
#include "aggregation.h"


MEMB(queued_packets_memb, struct aggregation_queueitem, CSMA_CONF_MAX_PACKET_PER_NEIGHBOR);
LIST(gAggregationFramesList);
LIST(gAggregationFramesInsertedList);



/*---------------------------------------------------------------------*/
/*
 */
static void
timeout_handler(void *ptr) {
  printf("AGGR timedout\n");
}

/*---------------------------------------------------------------------------*/
void
destroy_aggregation_frame(struct aggregation_queueitem *p)
{
  if(p == NULL) {
    return;
  }

  ctimer_stop(&(p->timeout));

  mmem_free(&(p->data));
  memb_free(&queued_packets_memb,p);
  PRINTF("Aggr frame destroyed!");
}

/*---------------------------------------------------------------------------*/
uint8_t
create_aggregation_frame(const char *msg, uint8_t size, linkaddr_t *node_addr, mac_callback_t mac_callback, uint16_t timeout)
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

  ctimer_set(&(p->timeout), timeout, timeout_handler, NULL);


  list_push(gAggregationFramesList, p);
  PRINTF("Aggreg frame created\n");
  return 1;
}

/*---------------------------------------------------------------------------*/
void
aggregation_apply()
{
  //Look for header pieces that match this packet conditions
  struct aggregation_queueitem *h;
  struct aggregation_queueitem *nextH;
  uint8_t piecesInserted = 0;
  uint16_t packetOldSize;


  aqui
  packetOldSize = packet_get_size(packet_piece);

  h = list_head(gPiggybackFramesList);
  if(h == NULL) {
    SAFE_PRINTF_LOG_ERROR("Piggy search NULL");
  }
  while(h != NULL) {
    nextH = list_item_next(h);
    SAFE_PRINTF_LOG_INFO("Piggy search ok");
    Boolean toFinalDestination = (h->finalDestinationNode == packet_get_final_destination(packet_piece));
    if(toFinalDestination ||
       h->finalDestinationNode == packet_get_next_destination(packet_piece)) {

      if(TRUE == insert_piggyback_to_packet(packet_piece,
                                             h,
                                             toFinalDestination,
                                             availableSpace)) {
        piggyPiecesInserted++;
        break;
      }
    }
    h = nextH;
  }

  if(0 == packet_append_last_header(1,
                                    &piggyPiecesInserted,
                                    packet_piece)) {
    SAFE_PRINTF_LOG_ERROR("Append");
  }

  return packet_get_size(packet_piece)-packetOldSize;
}

/*---------------------------------------------------------------------------*/
void
aggregation_init(void)
{
  memb_init(&queued_packets_memb);
  list_init(gAggregationFramesList);
  list_init(gAggregationFramesInsertedList);
}
