
/**
 * \defgroup decription...
 *
 * This...
 *
 * @{
 */

/*
 * null-transport.c
 *
 *  Created on: Nov 18, 2017
 *      Author: vinicius
 */
#include <stdio.h>
#include "contiki.h"
#include "iotus-netstack.h"

#define DEBUG IOTUS_PRINT_IMMEDIATELY
#define THIS_LOG_FILE_NAME_DESCRITOR "nullTrans"
#include "safe-printer.h"

static void
send(iotus_packet_t *packet)
{
  SAFE_PRINTF_LOG_INFO("Null trans");
  active_routing_protocol->send(packet);
}

static void
start(void)
{
  printf("Starting null transport\n");
}


static void
run(void)
{
}

static void
close(void)
{
}

const struct iotus_transport_protocol_struct null_transport_protocol = {
  start,
  NULL,
  run,
  close,
  send
};
/* The following stuff ends the \defgroup block at the beginning of
   the file: */

/** @} */
