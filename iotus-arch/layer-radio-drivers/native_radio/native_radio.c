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
#include "iotus-data-link.h"
#include "global-functions.h"
#include "global-parameters.h"
#include "packet.h"
#include "packet-defs.h"
#include "packet-default-additional-info.h"
#include "platform-conf.h"

#define DEBUG IOTUS_PRINT_IMMEDIATELY
#define THIS_LOG_FILE_NAME_DESCRITOR "native-radio"
#include "safe-printer.h"

#define OUTPUT_POWER_MAX   0
#define OUTPUT_POWER_MIN -25

static iotus_address_type g_used_address_type;
static uint8_t g_software_addr_filtering;
static Boolean g_expect_new_iotus_packet_hdr;

static uint8_t receive_on;
static int channel;

/*---------------------------------------------------------------------------*/
static radio_result_t
get_value(radio_param_t param, radio_value_t *value)
{
  if(!value) {
    return RADIO_RESULT_INVALID_VALUE;
  }
  switch(param) {
  case RADIO_PARAM_POWER_MODE:
  case RADIO_PARAM_CHANNEL:
    *value = channel;
    return RADIO_RESULT_OK;
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
    *value = g_used_address_type;
    return RADIO_RESULT_OK;
  default:
    return RADIO_RESULT_NOT_SUPPORTED;
  }
}
/*---------------------------------------------------------------------------*/

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
      g_used_address_type = value;
      
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

/*---------------------------------------------------------------------------*/

static int
on(void) {
  SAFE_PRINT("Radio is on!\n");
  receive_on = 1;
  return 1;
}

/*---------------------------------------------------------------------------*/

static int
off(void) {
  SAFE_PRINT("Radio is off!\n");
  receive_on = 0;
  return 1;
}

/*---------------------------------------------------------------------------*/
static int
prepare(iotus_packet_t *packet)
{
  uint8_t total_len;
  
  SAFE_PRINTF_CLEAN("radio: sending %d bytes\n", packet->data.size);

  iotus_parameters_radio_events.transmission++;

  /* Wait for any previous transmission to finish. */
  
  if(packet->params & PACKET_PARAMETERS_IS_NEW_PACKET_SYSTEM) {
    g_expect_new_iotus_packet_hdr = TRUE;
    /* verify if any layer is inserting the address... */
    if(IOTUS_PRIORITY_RADIO == iotus_get_layer_assigned_for(
                                    IOTUS_CHORE_INSERT_PKT_PREV_SRC_ADDRESS)) {
      if(0 == packet_append_last_header(
                  ADDRESSES_GET_TYPE_SIZE(g_used_address_type),
                  addresses_get_pointer(g_used_address_type),
                  packet)) {
        SAFE_PRINTF_CLEAN("Failed to insert source address.\n");
        return -1;
      }
      //SAFE_PRINTF_CLEAN("Prev addr inserted\n");
    }
    //In cases where broadcast is sent, only the source addr is necessary,
    //because the bit of broadcast in the iotus dynamic header is already set
    if(!(packet->iotusHeader & PACKET_IOTUS_HDR_IS_BROADCAST)) {
      //If this is not a broadcast, then we can try to insert send to a specific node.
      if(IOTUS_PRIORITY_RADIO == iotus_get_layer_assigned_for(
                                      IOTUS_CHORE_INSERT_PKT_NEXT_DST_ADDRESS)) {
        if(0 == packet_append_last_header(
                    ADDRESSES_GET_TYPE_SIZE(g_used_address_type),
                    nodes_get_address(g_used_address_type,packet->nextDestinationNode),
                    packet)) {
          SAFE_PRINTF_CLEAN("Failed to insert destination address.");
          return -1;
        }
        //SAFE_PRINTF_CLEAN("Next addr inserted\n");
      }
    }

    if(0 == packet_push_bit_header(PACKET_IOTUS_HDR_FIRST_BIT_POS,
                                  &(packet->iotusHeader),
                                  packet)) {
      SAFE_PRINTF_CLEAN("Failed insert iotus dyn hdr.");
      return -1;
    }
  }

  if(IOTUS_PRIORITY_RADIO == iotus_get_layer_assigned_for(
                                IOTUS_CHORE_PKT_CHECKSUM)) {
    total_len = packet->data.size + 2;
  } else {
    total_len = packet->data.size;
  }

  //Nothing to do right now
  total_len = total_len;
  return 0;
}

/*---------------------------------------------------------------------------*/
static int
transmit(iotus_packet_t *packet)
{
  int8_t tempTxPower = packet_get_tx_power(packet);
  SAFE_PRINTF_CLEAN("Power set to %d",tempTxPower);
  if(receive_on) {
    SAFE_PRINTF_LOG_INFO("Radio is still on");
  } else {
    /* We need to explicitly turn off the radio,
     * since STXON[CCA] -> TX_ACTIVE -> RX_ACTIVE */
    off();
  }

  if(tempTxPower >= OUTPUT_POWER_MIN && tempTxPower <= OUTPUT_POWER_MAX) {
    /* Restore the transmission power */
    SAFE_PRINTF_LOG_INFO("Power set by packet to %d",tempTxPower);
  }

  return RADIO_RESULT_OK;
}

/*---------------------------------------------------------------------------*/

static int
send(iotus_packet_t *packet)
{
  prepare(packet);
  SAFE_PRINTF_CLEAN("Null radio sending %u bytes:\n<",packet->data.size);
  SAFE_PRINT_BUF((char *)pieces_get_data_pointer(packet),packet->data.size);
  SAFE_PRINT(">\n");
  return 1;
}

/*---------------------------------------------------------------------------*/
static void
start(void)
{
  SAFE_PRINTF_CLEAN("\tNull Radio\n");
  iotus_packet_dimensions.total_size = 120;

  ADDRESSES_SET_TYPE_SIZE(IOTUS_ADDRESSES_TYPE_ADDR_PANID,2);
  ADDRESSES_SET_TYPE_SIZE(IOTUS_ADDRESSES_TYPE_ADDR_SHORT,2);
  ADDRESSES_SET_TYPE_SIZE(IOTUS_ADDRESSES_TYPE_ADDR_LONG,8);
  ADDRESSES_SET_TYPE_SIZE(IOTUS_ADDRESSES_TYPE_ADDR_BROADCAST,2);

  iotus_subscribe_for_chore(IOTUS_PRIORITY_RADIO, IOTUS_CHORE_PKT_CHECKSUM);
  g_software_addr_filtering = 0;
  g_expect_new_iotus_packet_hdr = TRUE;
}


/*---------------------------------------------------------------------------*/
static void
post_start(void)
{
  //Self config in case no layer took control for it...
  if(-1 == iotus_get_layer_assigned_for(IOTUS_CHORE_SET_ADDR_FOR_RADIO)) {
    set_value(RADIO_PARAM_ADDRESS_USE_TYPE,IOTUS_ADDRESSES_TYPE_ADDR_SHORT);
  }
}

/*---------------------------------------------------------------------------*/

static void
run(void)
{
}

/*---------------------------------------------------------------------------*/

static void
close(void)
{
}

/*---------------------------------------------------------------------------*/

const struct iotus_radio_driver_struct native_radio_radio_driver = {
  start,
  post_start,
  run,
  close,
  prepare,
  transmit,
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
