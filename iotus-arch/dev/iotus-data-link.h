/**
 * \defgroup description...
 *
 * This...
 *
 * @{
 */

/*
 * iotus-data-link.h
 *
 *  Created on: Oct 23, 2017
 *      Author: vinicius
 */



/* The following stuff ends the \defgroup block at the beginning of
   the file: */

/** @} */
#ifndef IOTUS_DEV_DATA_LINK_H_
#define IOTUS_DEV_DATA_LINK_H_

#include "iotus-core.h"
#include "packet.h"
#include "platform-conf.h"


#ifndef NETSTACK_RDC_CHANNEL_CHECK_RATE
#ifdef NETSTACK_CONF_RDC_CHANNEL_CHECK_RATE
#define NETSTACK_RDC_CHANNEL_CHECK_RATE NETSTACK_CONF_RDC_CHANNEL_CHECK_RATE
#else /* NETSTACK_CONF_RDC_CHANNEL_CHECK_RATE */
#define NETSTACK_RDC_CHANNEL_CHECK_RATE 8
#endif /* NETSTACK_CONF_RDC_CHANNEL_CHECK_RATE */
#endif /* NETSTACK_RDC_CHANNEL_CHECK_RATE */

#if (NETSTACK_RDC_CHANNEL_CHECK_RATE & (NETSTACK_RDC_CHANNEL_CHECK_RATE - 1)) != 0
#error NETSTACK_RDC_CONF_CHANNEL_CHECK_RATE must be a power of two (i.e., 1, 2, 4, 8, 16, 32, 64, ...).
#error Change NETSTACK_RDC_CONF_CHANNEL_CHECK_RATE in contiki-conf.h, project-conf.h or in your Makefile.
#endif

struct iotus_data_link_protocol_struct {
  char *name;
  void (* start)(void);
  void (* post_start)(void);
  void (* close)(void);
  int8_t (* send)(iotus_packet_t *packet);
  void (* sent_cb)(iotus_packet_t *packetk);
  iotus_netstack_return (* receive)(iotus_packet_t *packet); 
  /** Returns the channel check interval, expressed in clock_time_t ticks. */
  uint8_t (* channel_check_interval)(void);
};

#endif /* IOTUS_DEV_DATA_LINK_H_ */

/* The following stuff ends the \defgroup block at the beginning of
   the file: */

/** @} */
