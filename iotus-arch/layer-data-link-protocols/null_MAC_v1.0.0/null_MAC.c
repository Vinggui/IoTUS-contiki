
/**
 * \defgroup decription...
 *
 * This...
 *
 * @{
 */

/*
 * null-MAC.c
 *
 *  Created on: Nov 18, 2017
 *      Author: vinicius
 */
#include <stdio.h>
#include "contiki.h"
#include "iotus-radio.h"
#include "iotus-netstack.h"
#include "iotus-data-link.h"
#include "contikimac-framer.h"


#include "sys/rtimer.h"
#include "dev/watchdog.h"


#ifndef NULLRDC_802154_AUTOACK
#ifdef NULLRDC_CONF_802154_AUTOACK
#define NULLRDC_802154_AUTOACK NULLRDC_CONF_802154_AUTOACK
#else
#define NULLRDC_802154_AUTOACK 1
#endif /* NULLRDC_CONF_802154_AUTOACK */
#endif /* NULLRDC_802154_AUTOACK */

#ifdef NULLRDC_CONF_ACK_WAIT_TIME
#define ACK_WAIT_TIME NULLRDC_CONF_ACK_WAIT_TIME
#else /* NULLRDC_CONF_ACK_WAIT_TIME */
#define ACK_WAIT_TIME                      RTIMER_SECOND / 2500
#endif /* NULLRDC_CONF_ACK_WAIT_TIME */
#ifdef NULLRDC_CONF_AFTER_ACK_DETECTED_WAIT_TIME
#define AFTER_ACK_DETECTED_WAIT_TIME NULLRDC_CONF_AFTER_ACK_DETECTED_WAIT_TIME
#else /* NULLRDC_CONF_AFTER_ACK_DETECTED_WAIT_TIME */
#define AFTER_ACK_DETECTED_WAIT_TIME       RTIMER_SECOND / 1500
#endif /* NULLRDC_CONF_AFTER_ACK_DETECTED_WAIT_TIME */

#define ACK_LEN 3

#define DEBUG IOTUS_DONT_PRINT//IOTUS_PRINT_IMMEDIATELY
#define THIS_LOG_FILE_NAME_DESCRITOR "nullMac"
#include "safe-printer.h"

uint16_t gPkt_tx_successful = 0;
uint16_t gpkt_tx_attemps = 0;
uint16_t gpkt_tx_first_attemps = 0;

/*---------------------------------------------------------------------------*/
//send_packet(mac_callback_t mac_callback, void *mac_callback_ptr,
//      struct rdc_buf_list *buf_list,
//            int is_receiver_awake)
static int8_t
send_packet(iotus_packet_t *packet)
{
  int ret;
  int is_broadcast = 0;
  uint8_t dsn;

  gpkt_tx_attemps++;
  if(contikimac_framer.create(packet) < 0) {
    printf("contikimac: framer failed\n");
    return MAC_TX_ERR_FATAL;
  }
  active_radio_driver->prepare(packet);

  dsn = packet_get_sequence_number(packet) & 0xff;

  
  if(TRUE == packet_holds_broadcast(packet)) {
    is_broadcast = 1;
  }

  if(active_radio_driver->receiving_packet() ||
     (!is_broadcast && active_radio_driver->pending_packet())) {

    /* Currently receiving a packet over air or the radio has
       already received a packet that needs to be read before
       sending with auto ack. */
    ret = MAC_TX_COLLISION;
  } else {
    uint8_t status;
    active_radio_driver->on();
    status = active_radio_driver->transmit(packet);

    if(0 == gpkt_tx_first_attemps) {
      gpkt_tx_first_attemps = gpkt_tx_attemps;
      gpkt_tx_attemps = 0;
    }
    if (RADIO_TX_OK == status) {
      gPkt_tx_successful++;
    }

    switch(status) {
    case RADIO_TX_OK:
      if(is_broadcast) {
        ret = MAC_TX_OK;
      } else {
        rtimer_clock_t wt;

        /* Check for ack */
        wt = RTIMER_NOW();
        watchdog_periodic();
        while(RTIMER_CLOCK_LT(RTIMER_NOW(), wt + ACK_WAIT_TIME)) {
        }

        ret = MAC_TX_NOACK;
        if(active_radio_driver->receiving_packet() ||
           active_radio_driver->pending_packet() ||
           active_radio_driver->channel_clear() == 0) {

          if(AFTER_ACK_DETECTED_WAIT_TIME > 0) {
            wt = RTIMER_NOW();
            watchdog_periodic();
            while(RTIMER_CLOCK_LT(RTIMER_NOW(),
                                  wt + AFTER_ACK_DETECTED_WAIT_TIME)) {
            }
          }

          if(active_radio_driver->pending_packet()) {
              ret = MAC_TX_OK;
            iotus_packet_t *ack = active_radio_driver->read();
            if(NULL == ack) {
              /* Not an ack or ack not for us: collision */
              ret = MAC_TX_COLLISION;
            } else {
              uint8_t ackLength = packet_get_payload_size(ack);
              uint8_t ackSeqno = pieces_get_data_pointer(ack)[ACK_LEN - 1];
              packet_destroy(ack);
              if(ackLength == ACK_LEN &&
                 dsn == ackSeqno) {
                ret = MAC_TX_OK;
              }
            }
          }
        } else {
          SAFE_PRINTF_CLEAN("null_MAC tx noack\n");
        }
      }
      break;
    case RADIO_TX_COLLISION:
      ret = MAC_TX_COLLISION;
      break;
    default:
      ret = MAC_TX_ERR;
      break;
    }
  }

  active_radio_driver->off();
  return ret;
}
/*---------------------------------------------------------------------------*/
static iotus_netstack_return
input_packet(iotus_packet_t *packet)
{
  active_network_protocol->receive(packet);
  return RX_PROCESSED;
}
/*---------------------------------------------------------------------------*/
static void
init(void)
{
#if DOUBLE_NODE_NULL == 1
  if(addresses_self_get_pointer(IOTUS_ADDRESSES_TYPE_ADDR_SHORT)[0] == 1) {
    active_radio_driver->on();
  }
#endif
}

/*---------------------------------------------------------------------------*/
static void
post_start(void)
{
  // if(IOTUS_PRIORITY_ROUTING == iotus_get_layer_assigned_for(IOTUS_CHORE_ONEHOP_BROADCAST)) {
  //   timer_set(&sendBC, CLOCK_SECOND*10);
  // }
}
/*---------------------------------------------------------------------------*/

const struct iotus_data_link_protocol_struct null_MAC_protocol = {
  "nullMAC",
  init,
  post_start,
  NULL,
  send_packet,
  NULL,
  input_packet
};

/* The following stuff ends the \defgroup block at the beginning of
   the file: */

/** @} */
