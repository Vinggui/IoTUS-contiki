
/**
 * \defgroup description...
 *
 * This...
 *
 * @{
 */

/*
 * null_transport.c
 *
 *  Created on: Nov 18, 2017
 *      Author: vinicius
 */
#include <stdio.h>
#include "contiki.h"
#include "iotus-netstack.h"

#define DEBUG IOTUS_DONT_PRINT//IOTUS_PRINT_IMMEDIATELY
#define THIS_LOG_FILE_NAME_DESCRITOR "nullTrans"
#include "safe-printer.h"


static iotus_netstack_return
send(iotus_packet_t *packet)
{
  active_network_protocol->build_to_send(packet);
  return TRANSPORT_TX_OK;
}

static void
send_cb(iotus_packet_t *packet, iotus_netstack_return returnAns)
{
  SAFE_PRINTF_LOG_INFO("Frame processed %u", returnAns);
}

static void
start(void)
{
  printf("Starting null transport\n");
}

static void
close(void)
{
}

const struct iotus_transport_protocol_struct null_transport_protocol = {
  start,
  NULL,
  close,
  send,
  send_cb,
  NULL
};
/* The following stuff ends the \defgroup block at the beginning of
   the file: */

/** @} */
