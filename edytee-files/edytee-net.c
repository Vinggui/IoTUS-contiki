/**
 * \file
 *         Rime initialization and common code
 * \author
 *         
 */



#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

#include "net/netstack.h"
#include "net/mac/mac.h"
#include "net/packetbuf.h"

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
  //struct rime_sniffer *s;
  //struct channel *c;

  //RIMESTATS_ADD(rx);
  /*c = chameleon_parse();
  
  for(s = list_head(sniffers); s != NULL; s = list_item_next(s)) {
    if(s->input_callback != NULL) {
      s->input_callback();
    }
  }
  
  if(c != NULL) {
    abc_input(c);
  }*/
}
/*---------------------------------------------------------------------------*/
static void
init(void)
{
  queuebuf_init();
  packetbuf_clear();
  //announcement_init();

  //chameleon_init();
}

const struct network_driver edytee_net_driver = {
  "EDYTEE",
  init,
  input
};
