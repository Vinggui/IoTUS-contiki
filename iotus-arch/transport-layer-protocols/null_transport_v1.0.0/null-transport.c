
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
 #include "contiki.h"
 #include "iotus-core.h"
 #include "null-transport.h"

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
{

}

const struct iotus_transport_protocol_struct null_transport_protocol = {
  start,
  run,
  close
};
/* The following stuff ends the \defgroup block at the beginning of
   the file: */

/** @} */
