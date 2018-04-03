/**
 * \defgroup description...
 *
 * This...
 *
 * @{
 */

/*
 * native_radio.c
 *
 *  Created on: Nov 18, 2017
 *      Author: vinicius
 */
#include <stdio.h>
#include <stdlib.h>
#include "iotus-core.h"
#include "iotus-radio.h"
#include "packet.h"
#include "platform-conf.h"

#define DEBUG IOTUS_PRINT_IMMEDIATELY
#define THIS_LOG_FILE_NAME_DESCRITOR "native-radio"
#include "safe-printer.h"

static iotus_address_type iotus_used_address_type;

static radio_result_t
get_value(radio_param_t param, radio_value_t *value)
{
  if(!value) {
    return RADIO_RESULT_INVALID_VALUE;
  }
  switch(param) {
  case RADIO_PARAM_POWER_MODE:
  case RADIO_PARAM_CHANNEL:
  case RADIO_PARAM_RX_MODE:
  case RADIO_PARAM_TX_MODE:
  case RADIO_PARAM_TXPOWER:
  case RADIO_PARAM_CCA_THRESHOLD:
  case RADIO_PARAM_RSSI:
  case RADIO_CONST_CHANNEL_MIN:
  case RADIO_CONST_CHANNEL_MAX:
  case RADIO_CONST_TXPOWER_MIN:
  case RADIO_CONST_TXPOWER_MAX:
    return RADIO_RESULT_NOT_SUPPORTED;
  case RADIO_CONST_ADDRESSES_OPTIONS:
    //*value = (1<<7) | (1<<1)
    *value = 0b0000000010000010;
    return RADIO_RESULT_OK;
  case RADIO_PARAM_ADDRESS_USE_TYPE:
    *value = iotus_used_address_type;
    return RADIO_RESULT_OK;
  default:
    return RADIO_RESULT_NOT_SUPPORTED;
  }
}


static radio_result_t
set_value(radio_param_t param, radio_value_t value)
{
  int i;
  switch(param) {
  case RADIO_PARAM_CHANNEL:
  case RADIO_PARAM_RX_MODE:
  case RADIO_PARAM_TX_MODE:
  case RADIO_PARAM_TXPOWER:
  case RADIO_PARAM_CCA_THRESHOLD:
    return RADIO_RESULT_NOT_SUPPORTED;
  case RADIO_PARAM_ADDRESS_USE_TYPE:
    //Verify if this size is actually supported
    if(ADDRESSES_GET_TYPE_SIZE(value) == 2 || ADDRESSES_GET_TYPE_SIZE(value) == 8) {
      iotus_used_address_type = value;
      //Search for this address in the system
      SAFE_PRINT("Using address - ");
      for(i=0;i<ADDRESSES_GET_TYPE_SIZE(value);i++) {
        if(i!=0) {
          SAFE_PRINT(":");
        }
        SAFE_PRINTF_CLEAN("%02x",addresses_get_pointer(value)[i]);
      }
      SAFE_PRINT("\n");
    }
    return RADIO_RESULT_OK;
  default:
    return RADIO_RESULT_NOT_SUPPORTED;
  }
}

static int
send(iotus_packet_t *packet)
{
  SAFE_PRINTF_CLEAN("Null radio sending %u bytes:\n<",packet->data.size);
  SAFE_PRINT_BUF((char *)pieces_get_data_pointer(packet),packet->data.size);
  SAFE_PRINT(">\n");
  return 1;
}

int
on(void) {
  SAFE_PRINT("Radio is on!\n");
  return 1;
}


int
off(void) {
  SAFE_PRINT("Radio is off!\n");
  return 1;
}

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

const struct iotus_radio_driver_struct native_radio_radio_driver = {
  start,
  NULL,//post_start
  run,
  close,
  NULL,//cc2420_prepare,
  NULL,//cc2420_transmit,
  send,
  NULL,//cc2420_read,
  NULL,//cc2420_cca,
  NULL,//cc2420_receiving_packet,
  NULL,//pending_packet,
  on,//cc2420_on,
  off,//cc2420_off,
  get_value,
  set_value,
  NULL,//get_object,
  NULL,//set_object
};

/* The following stuff ends the \defgroup block at the beginning of
   the file: */

/** @} */
