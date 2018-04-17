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
  void (* run)(void);
  void (* close)(void);
  int8_t (* send)(iotus_packet_t *packet);
  void (* sent_cb)(iotus_packet_t *packetk);
  void (* receive)(iotus_packet_t *packet);
};

/* Generic MAC return values. */
enum {
  /**< The MAC layer transmission was OK. */
  MAC_TX_OK,

  /**< The MAC layer transmission could not be performed due to a
     collision. */
  MAC_TX_COLLISION,

  /**< The MAC layer did not get an acknowledgement for the packet. */
  MAC_TX_NOACK,

  /**< The MAC layer deferred the transmission for a later time. */
  MAC_TX_DEFERRED,

  /**< The MAC layer transmission could not be performed because of an
     error. The upper layer may try again later. */
  MAC_TX_ERR,

  /**< The MAC layer transmission could not be performed because of a
     fatal error. The upper layer does not need to try again, as the
     error will be fatal then as well. */
  MAC_TX_ERR_FATAL,
};

#endif /* IOTUS_DEV_DATA_LINK_H_ */

/* The following stuff ends the \defgroup block at the beginning of
   the file: */

/** @} */
