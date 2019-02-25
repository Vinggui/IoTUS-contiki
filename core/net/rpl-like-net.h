
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

extern const struct network_driver rpllikenet_driver;

#endif /* STATICNET_H_ */

/** @} */
/** @} */
