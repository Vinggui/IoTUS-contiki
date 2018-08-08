
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
#include "iotus-netstack.h"
#include "iotus-data-link.h"
#include "contikimac-framer.h"

#define DEBUG IOTUS_PRINT_IMMEDIATELY//IOTUS_DONT_PRINT
#define THIS_LOG_FILE_NAME_DESCRITOR "nullMac"
#include "safe-printer.h"


/*---------------------------------------------------------------------------*/
//send_packet(mac_callback_t mac_callback, void *mac_callback_ptr,
//      struct rdc_buf_list *buf_list,
//            int is_receiver_awake)
static int8_t
send_packet(iotus_packet_t *packet)
{

  if(contikimac_framer.create(packet) < 0) {
    printf("contikimac: framer failed\n");
    return MAC_TX_ERR_FATAL;
  }
  printf("test %u %u\n",packet_get_size(packet), packet_get_payload_size(packet));
  active_radio_driver->prepare(packet);
  active_radio_driver->transmit(packet);
  return MAC_TX_OK;
}
/*---------------------------------------------------------------------------*/
static iotus_netstack_return
input_packet(iotus_packet_t *packet)
{
  return RX_SEND_UP_STACK;
}
/*---------------------------------------------------------------------------*/
static void
init(void)
{
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
static void
run(void)
{ 
  packet_poll_by_priority(1);
}

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
