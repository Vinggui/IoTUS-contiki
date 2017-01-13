/**
 * \file
 *         A MAC protocol implementation that does not do anything.
 * \author
 *         
 */

#ifndef NEDYTEE_MAC_H_
#define NEDYTEE_MAC_H_

#include "edytee-comm.h"
#include "edytee-radio-params.h"
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

typedef enum {
    PACKET_TYPE_BEACON=1,
    PACKET_TYPE_MESSAGE,
    PACKET_TYPE_ACK,
    PACKET_TYPE_REGISTER_REQ,
    PACKET_TYPE_REGISTER_ANS,
    PACKET_TYPE_LEADER_BROADCAST
} Packet_Type;

//Verify configurations of the operating mode
#ifndef  WIRELESS_COMM_ROLE
#pragma message ("WIRELESS_COMM_ROLE parameter not defined. Using generic setup.")
#define WIRELESS_COMM_ROLE          NODE_MODE_GENERIC
#endif


//Maths about the system
#define MAC_MAX_PERIOD_DURATION                     1//seconds
#define RADIO_LAST_CHANNEL                          (RADIO_FIRST_CHANNEL + RADIO_CHANNEL_STEP_COUNT*RADIO_CHANNEL_SIZE -1)

extern Device_Status device_status;
extern const struct mac_driver edytee_mac_driver;


#endif /* NEDYTEE_MAC_H_ */
