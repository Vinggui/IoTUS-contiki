
/**
 * \addtogroup nullNet
 * @{
 */

#ifndef STATICNET_H_
#define STATICNET_H_

#include "net/rime/announcement.h"
#include "net/rime/collect.h"
#include "net/rime/ipolite.h"
#include "net/rime/mesh.h"
#include "net/rime/multihop.h"
#include "net/rime/neighbor-discovery.h"
#include "net/rime/netflood.h"
#include "net/rime/polite-announcement.h"
#include "net/rime/polite.h"
#include "net/queuebuf.h"
#include "net/linkaddr.h"
#include "net/packetbuf.h"
#include "net/rime/rimestats.h"
#include "net/rime/rmh.h"
#include "net/rime/route-discovery.h"
#include "net/rime/route.h"
#include "net/rime/rucb.h"
#include "net/rime/runicast.h"
#include "net/rime/timesynch.h"
#include "net/rime/trickle.h"

#include "net/mac/mac.h"

typedef enum {
  EDYTEE_COMMAND_TYPE_DATA=1,
  EDYTEE_COMMAND_TYPE_COMMAND_DIO_BROADCAST,
  EDYTEE_COMMAND_TYPE_COMMAND_DIO,
  EDYTEE_COMMAND_TYPE_COMMAND_DIS,
  EDYTEE_COMMAND_TYPE_COMMAND_DAO,
  EDYTEE_COMMAND_TYPE_COMMAND_DAO_ACK,
  EDYTEE_COMMAND_TYPE_COMMAND_DAO_TO_SINK,
  EDYTEE_COMMAND_TYPE_BROADCAST,

  EDYTEE_COMMAND_TYPE_MAX
} edytee_net_commands;

/* Similar to 802.15.4 MAC
* Beacons
* Association request packet
* Association get packet
* Association Answer packet
* Association confirm
*/
typedef enum {
  TREE_PKT_ASSOCIANTION_GET = 1,  //Data get ("DIS")
  TREE_PKT_ASSOCIANTION_ANS,      //Data info ("DIO")
  TREE_PKT_ADVERTISEMENT,         //Data Advertisement
  TREE_PKT_ADVERTISEMENT_ACK,     //Data Advertisement Ack ("DAO-ACK")
  TREE_PKT_ADVERTISEMENT_to_root, //Data Advertisement - to the root

  //Do not erase this last option
  TREE_PKT_MAX_VALUE
} tree_pkt_types;

// #define STATIC_COORDINATORS           1,2,3//Use comma to add more routers
// #define STATIC_ROOT_ADDRESS           {1,0}//two bytes address (short)

typedef enum {
  TREE_STATUS_DISCONNECTED,
  TREE_STATUS_SCANNING,
  TREE_STATUS_WAITING_SEND_DIS,
  TREE_STATUS_WAITING_ASNWER,
  TREE_STATUS_WAITING_CONFIRM,
  TREE_STATUS_BUILDING,
  TREE_STATUS_CONNECTED,
  TREE_STATUS_RECONNECTING
} tree_manager_conn_status;

extern rtimer_clock_t packetBuildingTime;
extern uint8_t ticTocFlag;

////////////////////////////////////////////////////////////
//             TEMPORARY MEASURES                         //
////////////////////////////////////////////////////////////
#define TIC() packetBuildingTime = RTIMER_NOW();ticTocFlag++
#define TOC() \
  if(ticTocFlag > 0) {\
    printf("pkt %lu\n",((1000000*(RTIMER_NOW() - packetBuildingTime))/RTIMER_ARCH_SECOND));\
    ticTocFlag = 0;\
  }

void
staticnet_signup(void (* msg_confirm)(int status, int num_tx), void (* msg_input)(const linkaddr_t *source));

int
staticnet_output(void);

//For the application layer
int
rpllikenet_send(void);

extern const struct network_driver rpllikenet_driver;

#endif /* STATICNET_H_ */

/** @} */
/** @} */
