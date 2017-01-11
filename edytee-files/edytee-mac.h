/**
 * \file
 *         A MAC protocol implementation that does not do anything.
 * \author
 *         
 */

#ifndef NEDYTEE_MAC_H_
#define NEDYTEE_MAC_H_

#include "edytee-comm.h"
#include "dev/radio.h"
#include "net/mac/mac.h"
#include "wireless-comm-configs.h"

typedef enum {
    NETWORK_STATUS_DISCONNECTED=0,
    NETWORK_STATUS_SEARCHING_CLUSTER,
    NETWORK_STATUS_REGISTERING,
    NETWORK_STATUS_CONNECTED_AS_LEAF,
    NETWORK_STATUS_CONNECTED_AS_ROUTER,
    NETWORK_STATUS_CONNECTED_AS_LEADER_ONLY
} Device_Status;


//Verify configurations of the operating mode
#ifndef  WIRELESS_COMM_ROLE
#pragma message ("WIRELESS_COMM_ROLE parameter not defined. Using generic setup.")
#define WIRELESS_COMM_ROLE          NODE_MODE_GENERIC
#endif


extern const struct mac_driver edytee_mac_driver;


#endif /* NEDYTEE_MAC_H_ */
