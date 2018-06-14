/**
 * \addtogroup staticnet
 * @{
 */

#define DEBUG 1
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

#include "net/linkaddr.h"
#include "net/netstack.h"
#include "net/rime/rime.h"
#include "net/rime/chameleon.h"
#include "net/rime/route.h"
#include "net/rime/announcement.h"
#include "net/rime/broadcast-announcement.h"
#include "net/mac/mac.h"

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
int routing_table[9][9] =
{
  //0=>
  {0,   0,   0,   0,   0,   0,   0,   0,   0},
  //1=>
  {0,   0,   2,   3,   2,   2,   3,   3,   3},
  //2=>
  {0,   1,   0,   3,   4,   5,   3,   3,   3},
  //3=>
  {0,   1,   2,   0,   2,   2,   6,   7,   8},
  //4=>
  {0,   2,   2,   2,   0,   5,   2,   2,   2},
  //5=>
  {0,   2,   2,   2,   4,   0,   2,   2,   2},
  //6=>
  {0,   3,   3,   3,   3,   3,   0,   7,   8},
  //7=>
  {0,   3,   3,   3,   3,   3,   6,   0,   8},
  //8=>
  {0,   3,   3,   3,   3,   3,   6,   7,   0}
};

//Timer for sending neighbor discovery
static struct timer sendND;
void (* up_msg_confirm)(int status, int num_tx) = NULL;
void (* up_msg_input)(const linkaddr_t *source) = NULL;

/*---------------------------------------------------------------------------*/
static void
packet_sent(void *ptr, int status, int num_tx)
{
  switch(status) {
  case MAC_TX_COLLISION:
    PRINTF("nullNet: collision after %d tx\n", num_tx);
    break; 
  case MAC_TX_NOACK:
    PRINTF("nullNet: noack after %d tx\n", num_tx);
    break;
  case MAC_TX_OK:
    PRINTF("nullNet: sent after %d tx\n", num_tx);
    break;
  default:
    PRINTF("nullNet: error %d after %d tx\n", status, num_tx);
  }
  up_msg_confirm(status,num_tx);
}

/*---------------------------------------------------------------------------*/
int
staticnet_output(void)
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
  addrNext.u8[0] = routing_table[linkaddr_node_addr.u8[0]][finalReceiver->u8[0]];
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

    staticnet_output();
  }
}
/*---------------------------------------------------------------------------*/
static void
init(void)
{
  PRINTF("nullnet started\n");
  queuebuf_init();
  packetbuf_clear();
}

/*---------------------------------------------------------------------------*/
void
staticnet_signup(void (* msg_confirm)(int status, int num_tx), void (* msg_input)(const linkaddr_t *source))
{
  up_msg_confirm = msg_confirm;
  up_msg_input = msg_input;
}

/*---------------------------------------------------------------------------*/
const struct network_driver staticnet_driver = {
  "staticnet",
  init,
  input
};
/** @} */
