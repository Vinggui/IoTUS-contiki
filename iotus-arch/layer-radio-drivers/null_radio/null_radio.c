/**
 * \defgroup decription...
 *
 * This...
 *
 * @{
 */

/*
 * null_radio.c
 *
 *  Created on: Nov 18, 2017
 *      Author: vinicius
 */
#include <stdio.h>
#include <stdlib.h>
#include "iotus-core.h"
#include "platform-conf.h"

#define DEBUG IOTUS_PRINT_IMMEDIATELY
#define THIS_LOG_FILE_NAME_DESCRITOR "null-radio"
#include "safe-printer.h"

static void
start(void)
{
  SAFE_PRINTF_CLEAN("\tNull Radio\n");
}


static void
run(void)
{
}

static void
close(void)
{
}

const struct iotus_radio_driver_struct null_radio_radio_driver = {
  start,
  run,
  close
};

/* The following stuff ends the \defgroup block at the beginning of
   the file: */

/** @} */
