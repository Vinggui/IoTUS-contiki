/**
 * \file
 *         A MAC protocol implementation that does not do anything.
 * \author
 *         
 */

#ifndef NEDYTEE_MAC_H_
#define NEDYTEE_MAC_H_

#include "net/mac/mac.h"
#include "dev/radio.h"

typedef enum {
    OP_MODE_DISCONNECTED=0,
    OP_MODE_LEAF,
    OP_MODE_ROUTER
} Device_Mode;


typedef enum {
    STATUS_DISCONNECTED=0,
    STATUS_SEARCHING_CLUSTER,
    STATUS_REGISTERING,
    STATUS_CONNECTED_AS_LEAF,
    STATUS_CONNECTED_AS_ROUTER,
    STATUS_CONNECTED_AS_LEADER_ONLY
} Device_Status;

extern const struct mac_driver edytee_mac_driver;


#endif /* NEDYTEE_MAC_H_ */
