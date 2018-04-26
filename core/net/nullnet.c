/**
 * \addtogroup nullnet
 * @{
 */

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

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


/*---------------------------------------------------------------------------*/
static void
input(void)
{

  //call application callback
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
}
/*---------------------------------------------------------------------------*/
int
nullnet_output(void)
{
  PRINTF("Sending through nullnet\n");
  RIMESTATS_ADD(tx);
  packetbuf_compact();
  NETSTACK_LLSEC.send(packet_sent, NULL);
  return 1;
}
/*---------------------------------------------------------------------------*/
const struct network_driver nullnet_driver = {
  "nullnet",
  init,
  input
};
/** @} */
