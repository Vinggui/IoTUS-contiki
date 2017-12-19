
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
#include "contiki.h"

static void
start(void)
{
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
  run,
  close
};
/* The following stuff ends the \defgroup block at the beginning of
   the file: */

/** @} */
