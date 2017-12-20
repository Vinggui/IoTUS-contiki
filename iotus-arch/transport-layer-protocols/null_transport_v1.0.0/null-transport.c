
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
 #include "iotus-core.h"
 #include "null-transport.h"

 #define DEBUG 1
 #if DEBUG
 #define PRINTF(...) printf(__VA_ARGS__)
 #else /* DEBUG */
 #define PRINTF(...)
 #endif /* DEBUG */

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
  run,
  close
};
/* The following stuff ends the \defgroup block at the beginning of
   the file: */

/** @} */
