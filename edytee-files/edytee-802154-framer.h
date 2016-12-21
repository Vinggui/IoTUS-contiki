/**
 * \file
 *         MAC framer for edytee using 802154 radios
 * \author
 *         
 */

#ifndef EDYTEE_802154_FRAMER_H_
#define EDYTEE_802154_FRAMER_H_

#include "net/mac/framer.h"


#ifdef IEEE802154_CONF_PANID
#define IEEE802154_PANID           IEEE802154_CONF_PANID
#else /* IEEE802154_CONF_PANID */
#define IEEE802154_PANID           0xABCD
#endif /* IEEE802154_CONF_PANID */

extern const struct framer framer_edytee_802154;

#endif /* EDYTEE_802154_FRAMER_H_ */
