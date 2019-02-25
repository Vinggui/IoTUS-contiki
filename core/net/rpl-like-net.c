/**
 * \addtogroup rpl-like-net
 * @{
 */

#define DEBUG 1
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

//Timer for sending neighbor discovery
static struct ctimer sendNDTimer;
rtimer_clock_t packetBuildingTime;
// uint8_t ticTocFlag = 0;
static clock_time_t backOffDifference;

static uint8_t private_keep_alive[12];
static uint8_t gPkt_created = 0;

static void (* up_msg_confirm)(int status, int num_tx) = NULL;
static void (* up_msg_input)(const linkaddr_t *source) = NULL;

/*---------------------------------------------------------------------------*/
static void
packet_sent(void *ptr, int status, int num_tx)
{
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
  up_msg_confirm(status,num_tx);
}

/*---------------------------------------------------------------------------*/
int
rpllikenet_output(void)
{
  RIMESTATS_ADD(tx);

  linkaddr_t const *finalReceiver = packetbuf_addr(PACKETBUF_ADDR_RECEIVER);
  if(packetbuf_hdralloc(1)) {
    uint8_t *buf = packetbuf_hdrptr();
    buf[0] = finalReceiver->u8[0];
  } else {
    PRINTF("Failed to create packet");
    return 0;
  }

  static linkaddr_t addrNext;

  // addrNext.u8[0] = routing_table[linkaddr_node_addr.u8[0]][finalReceiver->u8[0]];

  addrNext.u8[1] = 0;
  packetbuf_set_addr(PACKETBUF_ADDR_RECEIVER, &addrNext);

  packetbuf_compact();
  NETSTACK_LLSEC.send(packet_sent, NULL);
  return 1;
}

/*---------------------------------------------------------------------------*/
static void
input(void)
{
  uint8_t *data = packetbuf_dataptr();

  packetbuf_hdrreduce(1);
  if(data[0] == linkaddr_node_addr.u8[0]) {
    up_msg_input(packetbuf_addr(PACKETBUF_ADDR_SENDER));
  } else {
    static linkaddr_t addrFinal;
    addrFinal.u8[0] = *data;
    addrFinal.u8[1] = 0;
    packetbuf_set_addr(PACKETBUF_ADDR_RECEIVER, &addrFinal);
    packetbuf_set_addr(PACKETBUF_ADDR_SENDER, &linkaddr_node_addr);

    rpllikenet_output();
  }
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
create_keep_alive_full_packet(void *ptr, int status, int num_tx)
{
  // packetbuf_copyfrom("123456789012345678901234567890", 30);
  packetbuf_copyfrom(private_keep_alive, 12);
  linkaddr_t addr;
  addr.u8[0] = 1;
  addr.u8[1] = 0;
  packetbuf_set_addr(PACKETBUF_ADDR_RECEIVER, &addr);
  packetbuf_set_addr(PACKETBUF_ADDR_SENDER, &linkaddr_node_addr);

  rpllikenet_output();
}

/*---------------------------------------------------------------------------*/
static void
send_keep_alive(void *ptr)
{
  if(gPkt_created >= MAX_GENERATED_KA) {
      return;
  }
  gPkt_created++;


#if USE_NEW_FEATURES == 1
  clock_time_t backoff = CLOCK_SECOND*KEEP_ALIVE_INTERVAL - backOffDifference;//ms
  backOffDifference = 0;
  backoff += backOffDifference;
  ctimer_set(&sendNDTimer, backoff, send_keep_alive, NULL);


  //A slightly cheat
  linkaddr_t addr;
  addr.u8[1] = 0;

  // addr.u8[0] = routing_table[linkaddr_node_addr.u8[0]][1];


  if(!create_aggregation_frame(private_keep_alive, 12, &addr, create_keep_alive_full_packet, ROUTING_PACKETS_TIMEOUT)) {
    PRINTF("Failed creating KA!\n");
  }

#else
  clock_time_t backoff = CLOCK_SECOND*KEEP_ALIVE_INTERVAL - backOffDifference;//ms
  backOffDifference = (CLOCK_SECOND*((random_rand()%BACKOFF_TIME)))/1000;
  backoff += backOffDifference;
  ctimer_set(&sendNDTimer, backoff, send_keep_alive, NULL);

  create_keep_alive_full_packet(NULL,0,1);
#endif

  printf("Net sending to 1\n");
}

/*---------------------------------------------------------------------------*/
static void
init(void)
{
  PRINTF("rpl-like-net started\n");
  queuebuf_init();

  packetbuf_clear();

  linkaddr_t addrThis;
  addrThis.u8[0] = 1;
  addrThis.u8[1] = 0;

  if(!linkaddr_cmp(&addrThis, &linkaddr_node_addr)) {
    // backOffDifference = (CLOCK_SECOND*((random_rand()%BACKOFF_TIME)))/1000;
    // clock_time_t backoff = CLOCK_SECOND*KEEP_ALIVE_INTERVAL + backOffDifference;//ms
    // ctimer_set(&sendNDTimer, backoff, send_keep_alive, NULL);
  }


  static uint8_t selfAddrValue;

  selfAddrValue = linkaddr_node_addr.u8[0];
  sprintf((char *)private_keep_alive, "### %02u %02u###", selfAddrValue,
                                                        selfAddrValue);
}


/*---------------------------------------------------------------------------*/
const struct network_driver rpllikenet_driver = {
  "rpl-like-net",
  init,
  input
};
/** @} */
