
/**
 * \defgroup description...
 *
 * This...
 *
 * @{
 */

/*
 * null-routing.c
 *
 *  Created on: Nov 18, 2017
 *      Author: vinicius
 */
#include <stdio.h>
#include "contiki.h"
#include "iotus-netstack.h"

#define DEBUG IOTUS_PRINT_IMMEDIATELY
#define THIS_LOG_FILE_NAME_DESCRITOR "nullRouting"
#include "safe-printer.h"

static void
input_packet(iotus_packet_t *packet)
{
  SAFE_PRINTF_CLEAN("Got packet: ");
  int i;
  for (i = 0; i < packet_get_payload_size(packet); ++i)
  {
    SAFE_PRINTF_CLEAN("%02x ", packet_get_payload_data(packet)[i]);
  }
  SAFE_PRINTF_CLEAN("\n");
}

static iotus_netstack_return
send(iotus_packet_t *packet)
{
  SAFE_PRINTF_LOG_INFO("Null route");
  packet->nextDestinationNode = packet->finalDestinationNode;
  //active_data_link_protocol->send(packet);
  return ROUTING_TX_OK;
}


static void
send_cb(iotus_packet_t *packet, iotus_netstack_return returnAns)
{
  SAFE_PRINTF_LOG_INFO("Frame processed %u", returnAns);
}

static void
start(void)
{
  printf("Starting null routing\n");
}


static void
run(void)
{
  iotus_core_netstack_idle_for(IOTUS_PRIORITY_ROUTING, 0XFFFF);
}

static void
close(void)
{}

struct iotus_routing_protocol_struct null_routing_protocol = {
  start,
  NULL,
  run,
  close,
  send,
  send_cb,
  input_packet
};
/* The following stuff ends the \defgroup block at the beginning of
   the file: */

/** @} */
