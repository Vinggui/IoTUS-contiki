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
#include "contikimac.h"
#include "dev/leds.h"
#include "net/linkaddr.h"
#include "net/netstack.h"
#include "net/rime/rime.h"
#include "net/mac/mac.h"
#include "random.h"
#include "sys/ctimer.h"

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
rtimer_clock_t packetBuildingTime;
uint8_t ticTocFlag = 0;
static clock_time_t backOffDifference;

static uint8_t private_keep_alive[12];

void (* up_msg_confirm)(int status, int num_tx) = NULL;
void (* up_msg_input)(const linkaddr_t *source) = NULL;

/*---------------------------------------------------------------------------*/
static void
packet_sent(void *ptr, int status, int num_tx)
{
  switch(status) {
  case MAC_TX_COLLISION:
    PRINTF("staticnet: collision after %d tx\n", num_tx);
    break; 
  case MAC_TX_NOACK:
    PRINTF("staticnet: noack after %d tx\n", num_tx);
    break;
  case MAC_TX_OK:
    PRINTF("staticnet: sent after %d tx\n", num_tx);
    break;
  default:
    PRINTF("staticnet: error %d after %d tx\n", status, num_tx);
  }
  up_msg_confirm(status,num_tx);
}

/*---------------------------------------------------------------------------*/
int
staticnet_output(void)
{
  RIMESTATS_ADD(tx);

#if BROADCAST_EXAMPLE == 0
  linkaddr_t const *finalReceiver = packetbuf_addr(PACKETBUF_ADDR_RECEIVER);
  if(packetbuf_hdralloc(1)) {
    uint8_t *buf = packetbuf_hdrptr();
    buf[0] = finalReceiver->u8[0];
  } else {
    PRINTF("Failed to create packet");
    return 0;
  }

  static linkaddr_t addrNext;

#if EXP_STAR_LIKE == 1
  addrNext.u8[0] = 1;
#else
  addrNext.u8[0] = routing_table[linkaddr_node_addr.u8[0]][finalReceiver->u8[0]];
#endif
  addrNext.u8[1] = 0;
  packetbuf_set_addr(PACKETBUF_ADDR_RECEIVER, &addrNext);
#endif

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

    staticnet_output();
  }
}

/*---------------------------------------------------------------------------*/
void
staticnet_signup(void (* msg_confirm)(int status, int num_tx), void (* msg_input)(const linkaddr_t *source))
{
  up_msg_confirm = msg_confirm;
  up_msg_input = msg_input;
}

/*---------------------------------------------------------------------------*/
static void
send_keep_alive(void *ptr)
{
  clock_time_t backoff = CLOCK_SECOND*KEEP_ALIVE_INTERVAL - backOffDifference;//ms
  backOffDifference = (CLOCK_SECOND*(random_rand()%BACKOFF_TIME))/1000;
  backoff += backOffDifference;
  ctimer_set(&sendNDTimer, backoff, send_keep_alive, NULL);

  // packetbuf_copyfrom("123456789012345678901234567890", 30);
  packetbuf_copyfrom(private_keep_alive, 12);
  linkaddr_t addr;
  addr.u8[0] = 1;
  addr.u8[1] = 0;
  packetbuf_set_addr(PACKETBUF_ADDR_RECEIVER, &addr);
  packetbuf_set_addr(PACKETBUF_ADDR_SENDER, &linkaddr_node_addr);

  printf("Net sending to 1\n");
  staticnet_output();
}

/*---------------------------------------------------------------------------*/
static void
init(void)
{
  PRINTF("staticnet started\n");
  queuebuf_init();
  packetbuf_clear();


  linkaddr_t addrThis;
  addrThis.u8[0] = 1;
  addrThis.u8[1] = 0;

#if BROADCAST_EXAMPLE == 0
  #if DOUBLE_NODE_NULL == 0
    if(!linkaddr_cmp(&addrThis, &linkaddr_node_addr)) {
      backOffDifference = (CLOCK_SECOND*(random_rand()%BACKOFF_TIME))/1000;
      clock_time_t backoff = CLOCK_SECOND*KEEP_ALIVE_INTERVAL + backOffDifference;//ms
      ctimer_set(&sendNDTimer, backoff, send_keep_alive, NULL);
    }
  #endif
#if DOUBLE_NODE_NULL == 1
  if(linkaddr_cmp(&addrThis, &linkaddr_node_addr)) {
    NETSTACK_RDC.on();
  }
#endif
#endif


  static uint8_t selfAddrValue;

  selfAddrValue = linkaddr_node_addr.u8[0];
  sprintf((char *)private_keep_alive, "### %02u %02u###", selfAddrValue,
                                                        selfAddrValue);
}


/*---------------------------------------------------------------------------*/
const struct network_driver staticnet_driver = {
  "staticnet",
  init,
  input
};
/** @} */
