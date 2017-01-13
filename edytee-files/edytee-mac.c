/**
 * \file
 *         The EDYTEE MAC protocol
 * \author
 *         
 */

#include "edytee-mac.h"
#include "net/netstack.h"
#include "net/packetbuf.h"
#include "net/netstack.h"

#if WIRELESS_COMM_ROLE != NODE_MODE_LEAF
#include "edytee-leader-functions.h"
#endif
#if WIRELESS_COMM_ROLE != NODE_MODE_ROOT
#include "edytee-leaf-functions.h"
#endif


Device_Status device_status;

//One synchronous timer for the entire mac layer
static struct ctimer mac_ctimer;

/*---------------------------------------------------------------------------*/
static void
send_packet(mac_callback_t sent, void *ptr)
{
  NETSTACK_RDC.send(sent, ptr);
}
/*---------------------------------------------------------------------------*/
static void
packet_input(void)
{
  NETSTACK_LLSEC.input();
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
  return 0;
}
/*---------------------------------------------------------------------------*/
static void
init(void)
{
    device_status = NETWORK_STATUS_DISCONNECTED;


#if WIRELESS_COMM_ROLE == NODE_MODE_DISCONNECTED
    return;
#else

    //First set all variable to the right start point
#   if WIRELESS_COMM_ROLE != NODE_MODE_ROOT
    init_leaf_functions();
#   endif /*WIRELESS_COMM_ROLE == NODE_MODE_ROOT*/

#   if WIRELESS_COMM_ROLE != NODE_MODE_LEAF
    init_leader_functions();
#   endif /*WIRELESS_COMM_ROLE == NODE_MODE_ROOT*///set and start

#   if WIRELESS_COMM_ROLE == NODE_MODE_ROOT
    //Root device can start their cluster immediately
    ctimer_set(&mac_ctimer, CLOCK_SECOND*MAC_MAX_PERIOD_DURATION,
            start_own_cluster_period, &mac_ctimer);
#   elif WIRELESS_COMM_ROLE != NODE_MODE_ROOT
    //Any other type of node must first search for a cluster to connect
    start_leaf_operations_period(&mac_ctimer);

#endif /*WIRELESS_COMM_ROLE == NODE_MODE_ROOT*/

#endif /*WIRELESS_COMM_ROLE == NODE_MODE_DISCONNECTED*/
}


/*---------------------------------------------------------------------------*/
const struct mac_driver edytee_mac_driver = {
  "nullmac",
  init,
  send_packet,
  packet_input,
  on,
  off,
  channel_check_interval,
};
/*---------------------------------------------------------------------------*/
