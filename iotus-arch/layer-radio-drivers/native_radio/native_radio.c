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
#include "iotus-netstack.h"
#include "platform-conf.h"
#include "radio-framer-802154.h"
#include <string.h>

#define DEBUG IOTUS_PRINT_IMMEDIATELY
#define THIS_LOG_FILE_NAME_DESCRITOR "native-radio"
#include "safe-printer.h"

#define OUTPUT_POWER_MAX   0
#define OUTPUT_POWER_MIN -25
#define FOOTER_LEN        2

static iotus_address_type g_used_address_type;
static uint8_t g_software_addr_filtering;
static Boolean g_expect_new_iotus_packet_hdr;

static uint8_t receive_on;
static int channel;

#define NATIVE_RADIO_AUTO_RECEIVE 1
uint8_t emu_msg[200];
uint8_t packet_indication;

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
      iotus_radio_selected_address_type = value;
      
      //Search for this address in the system
      SAFE_PRINT("Using address - ");
      for(i=0;i<ADDRESSES_GET_TYPE_SIZE(value);i++) {
        if(i!=0) {
          SAFE_PRINT(":");
        }
        SAFE_PRINTF_CLEAN("%02x",addresses_self_get_pointer(value)[i]);
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
  iotus_parameters_radio_events.radioStatus = RADIO_POWER_MODE_ON;
  return 1;
}

/*---------------------------------------------------------------------------*/

static int
off(void) {
  SAFE_PRINT("Radio is off!\n");
  receive_on = 0;
  iotus_parameters_radio_events.radioStatus = RADIO_POWER_MODE_OFF;
  return 1;
}

/*---------------------------------------------------------------------------*/
static int
prepare(iotus_packet_t *packet)
{
  uint8_t total_len;
  iotus_parameters_radio_events.transmission++;

  /* Wait for any previous transmission to finish. */
  
  // if(packet->params & PACKET_PARAMETERS_IS_NEW_PACKET_SYSTEM) {
  //   g_expect_new_iotus_packet_hdr = TRUE;
  //   /* verify if any layer is inserting the address... */
  //   if(IOTUS_PRIORITY_RADIO == iotus_get_layer_assigned_for(
  //                                   IOTUS_CHORE_INSERT_PKT_PREV_SRC_ADDRESS)) {
  //     if(0 == packet_append_last_header(
  //                 ADDRESSES_GET_TYPE_SIZE(g_used_address_type),
  //                 addresses_self_get_pointer(g_used_address_type),
  //                 packet)) {
  //       SAFE_PRINTF_CLEAN("Failed to insert source address.\n");
  //       return -1;
  //     }
  //     //SAFE_PRINTF_CLEAN("Prev addr inserted\n");
  //   }
  //   //In cases where broadcast is sent, only the source addr is necessary,
  //   //because the bit of broadcast in the iotus dynamic header is already set
  //   if(!(packet->iotusHeader & PACKET_IOTUS_HDR_IS_BROADCAST)) {
  //     //If this is not a broadcast, then we can try to insert send to a specific node.
  //     if(IOTUS_PRIORITY_RADIO == iotus_get_layer_assigned_for(
  //                                     IOTUS_CHORE_INSERT_PKT_NEXT_DST_ADDRESS)) {
  //       if(0 == packet_append_last_header(
  //                   ADDRESSES_GET_TYPE_SIZE(g_used_address_type),
  //                   nodes_get_address(g_used_address_type,packet->nextDestinationNode),
  //                   packet)) {
  //         SAFE_PRINTF_CLEAN("Failed to insert destination address.");
  //         return -1;
  //       }
  //       //SAFE_PRINTF_CLEAN("Next addr inserted\n");
  //     }
  //   }

  //   if(0 == packet_push_bit_header(PACKET_IOTUS_HDR_FIRST_BIT_POS,
  //                                 &(packet->iotusHeader),
  //                                 packet)) {
  //     SAFE_PRINTF_CLEAN("Failed insert iotus dyn hdr.");
  //     return -1;
  //   }
  // }

  if(IOTUS_PRIORITY_RADIO == iotus_get_layer_assigned_for(
                                IOTUS_CHORE_PKT_CHECKSUM)) {
    total_len = packet->data.size + 2;
    emu_msg[total_len-1] = 21;
    emu_msg[total_len-2] = 42;
  } else {
    total_len = packet->data.size;
  }

  //Nothing to do right now
  total_len = total_len;
  

  emu_msg[0]= total_len;
  memcpy(emu_msg+1, pieces_get_data_pointer(packet), packet->data.size);

  SAFE_PRINTF_CLEAN("radio: prepares %d bytes\n", total_len);
  return total_len;
}

/*---------------------------------------------------------------------------*/
static int
transmit(iotus_packet_t *packet)
{
  int8_t tempTxPower = packet_get_tx_power(packet);
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


  SAFE_PRINTF_CLEAN("Null radio sending %u bytes:\n<",emu_msg[0]);
  SAFE_PRINT_BUF((char *)pieces_get_data_pointer(packet),packet->data.size);
  SAFE_PRINT(">\n");


  printf("data len %u\n",packet_get_size(packet) );
  printf(" data: ");
  for(int k=0;k<packet_get_size(packet);k++) {
    printf("%02x ",pieces_get_data_pointer(packet)[k]);
  }
  printf("\nfim\n");

  #if NATIVE_RADIO_AUTO_RECEIVE == 1
    packet_indication = 1;
  #endif
    
  return RADIO_RESULT_OK;
}

/*---------------------------------------------------------------------------*/

static iotus_packet_t *
read(void)
{
  uint8_t footer[FOOTER_LEN];
  uint8_t len;

  //Emulated input...
  len=emu_msg[0];

  if(len > 200) {
    /* Oops, we must be out of sync. */
    iotus_parameters_radio_events.badSynch++;
  } else if(len <= FOOTER_LEN) {
    iotus_parameters_radio_events.badRxPacketShort++;
  } else {
    /* reserve space for this packet */
    iotus_packet_t *packet = packet_create_msg(
                                len - FOOTER_LEN,
                                IOTUS_PRIORITY_RADIO,
                                0,
                                NULL,
                                FALSE,
                                NULL,
                                NULL);
    if(packet == NULL) {
      SAFE_PRINTF_LOG_ERROR("No storage to receive pkt\n");
      //Drop packet or try later...
      return NULL;
    }

    memcpy(pieces_get_data_pointer(packet),emu_msg+1, len - FOOTER_LEN);
    memcpy(footer, emu_msg+1+len, FOOTER_LEN);

    //if(footer[1] & FOOTER1_CRC_OK) {
    if(1) {//Jump CS verification...
      //cc2420_last_rssi = footer[0] + RSSI_OFFSET;
      //cc2420_last_correlation = footer[1] & FOOTER1_CORRELATION;


      //Check the address and filtering options
      // if(g_expect_new_iotus_packet_hdr == TRUE) {
      //   packet_parse(packet);

      //   if(!(packet->iotusHeader & PACKET_IOTUS_HDR_IS_BROADCAST)) {
      //     /**
      //      * In cases where broadcast is sent, only the source addr is received
      //      * If this is not a broadcast, then we can try to receive to a specific node.
      //      */

      //     if(IOTUS_PRIORITY_RADIO == iotus_get_layer_assigned_for(
      //                                     IOTUS_CHORE_INSERT_PKT_NEXT_DST_ADDRESS)) {
      //       uint8_t address[ADDRESSES_GET_TYPE_SIZE(g_used_address_type)];
      //       if(FAILURE == packet_unwrap_appended_byte(
      //                         packet,
      //                         address,
      //                         ADDRESSES_GET_TYPE_SIZE(g_used_address_type))) {
      //         SAFE_PRINTF_LOG_ERROR("Failed couldn't unwrap.");
      //         packet_destroy(packet);
      //         return NULL;
      //       }

      //       SAFE_PRINTF_LOG_ERROR("Got source addr %u %u\n",address[1],address[0]);
      //       if(FALSE == addresses_compare(address,
      //                     addresses_self_get_pointer(g_used_address_type),
      //                     ADDRESSES_GET_TYPE_SIZE(g_used_address_type))) {
      //         //This message is not for us... Drop it?
      //         SAFE_PRINTF_LOG_ERROR("Dropping pckt!, wrong dest.");
      //         packet_destroy(packet);
      //         return NULL;
      //       }
      //       //iotus_node_t *node = nodes_update_by_address(g_used_address_type,address);
      //       //EXTRAIR INFORMACOES PARA O PACOTE
      //     }
      //   }

      //   if(IOTUS_PRIORITY_RADIO == iotus_get_layer_assigned_for(
      //                                   IOTUS_CHORE_INSERT_PKT_PREV_SRC_ADDRESS)) {
      //     uint8_t address[ADDRESSES_GET_TYPE_SIZE(g_used_address_type)];
      //     if(FAILURE == packet_unwrap_appended_byte(
      //                       packet,
      //                       address,
      //                       ADDRESSES_GET_TYPE_SIZE(g_used_address_type))) {
      //       SAFE_PRINTF_LOG_ERROR("Failed to get prev address.");
      //       packet_destroy(packet);
      //       return NULL;
      //     }
      //     uint8_t *addrPointer = pieces_modify_additional_info_var(packet->additionalInfoList,
      //                                IOTUS_PACKET_INFO_TYPE_PREV_SOURCE_ADDRESS,
      //                                ADDRESSES_GET_TYPE_SIZE(g_used_address_type),
      //                                TRUE);
      //     if(NULL == addrPointer) {
      //       SAFE_PRINTF_LOG_ERROR("Failed to create additional info");
      //     }
      //     memcpy(addrPointer, address, ADDRESSES_GET_TYPE_SIZE(g_used_address_type));
      //   }
      // }

      iotus_parameters_radio_events.lastRSSI = footer[0];
      iotus_parameters_radio_events.lastLinkQuality = footer[1];
      //Add the other information in this packet
      if(FAILURE == packet_set_rx_linkQuality_RSSI(
                      packet,
                      footer[1],
                      footer[0])) {
        SAFE_PRINTF_LOG_ERROR("Failed add rx block info.");
      }

      iotus_parameters_radio_events.receptions++;
    } else {
      iotus_parameters_radio_events.badRxChecksumFail++;
      len = FOOTER_LEN;
      packet_destroy(packet);
      return NULL;
    }

    return packet;
  }
  return NULL;
}

/*---------------------------------------------------------------------------*/

static int
send(iotus_packet_t *packet)
{
  prepare(packet);
  transmit(packet);

  return 1;
}

/*---------------------------------------------------------------------------*/
static void
start(void)
{
  SAFE_PRINTF_CLEAN("\tNull Radio\n");
  iotus_packet_dimensions.totalSize = 120;

  ADDRESSES_SET_TYPE_SIZE(IOTUS_ADDRESSES_TYPE_ADDR_PANID,2);
  ADDRESSES_SET_TYPE_SIZE(IOTUS_ADDRESSES_TYPE_ADDR_SHORT,2);
  ADDRESSES_SET_TYPE_SIZE(IOTUS_ADDRESSES_TYPE_ADDR_LONG,8);
  ADDRESSES_SET_TYPE_SIZE(IOTUS_ADDRESSES_TYPE_ADDR_BROADCAST,2);

  iotus_subscribe_for_chore(IOTUS_PRIORITY_RADIO, IOTUS_CHORE_PKT_CHECKSUM);
  g_software_addr_filtering = 0;
  g_expect_new_iotus_packet_hdr = TRUE;
  packet_indication = 0;
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
  if(packet_indication) {
    packet_indication = 0;
    emu_msg[6] = 4;
    emu_msg[7] = 2;
    iotus_packet_t *packet = read();
    if(packet == NULL) {
      SAFE_PRINTF_LOG_ERROR("Packet read is null");
    }
    active_data_link_protocol->receive(packet);
  }
}

/*---------------------------------------------------------------------------*/

static void
close(void)
{
}

static int
cca(void)
{
  return 1;
}

static int
receiving(void)
{
  return 0;
}

static int
pending_packet(void)
{
  return 0;
}
/*---------------------------------------------------------------------------*/

const struct iotus_radio_driver_struct native_radio_radio_driver = {
  "native radio",
  start,
  post_start,
  run,
  close,
  prepare,
  transmit,
  send,
  read,
  cca,
  receiving,
  pending_packet,
  on,
  off,
  get_value,
  set_value,
  NULL,//get_object,
  NULL,//set_object
  &radio_framer_802154
};

/* The following stuff ends the \defgroup block at the beginning of
   the file: */

/** @} */
