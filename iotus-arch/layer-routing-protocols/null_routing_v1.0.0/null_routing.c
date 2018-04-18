
/**
 * \defgroup decription...
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
send(iotus_packet_t *packet)
{
  SAFE_PRINTF_LOG_INFO("Null route");
  packet->nextDestinationNode = packet->finalDestinationNode;
  active_data_link_protocol->send(packet);
}

static void
start(void)
{
  printf("Starting null routing\n");
}


static void
run(void)
{
}

static void
close(void)
{}

struct iotus_routing_protocol_struct null_routing_protocol = {
  start,
  NULL,
  run,
  close,
  send
};
/* The following stuff ends the \defgroup block at the beginning of
   the file: */

/** @} */
