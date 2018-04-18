/**
 * \defgroup description...
 *
 * This...
 *
 * @{
 */

/*
 * pieces.h
 *
 *  Created on: Apr 11, 2018
 *      Author: vinicius
 */
#include <string.h>
#include "lib/random.h"
#include "iotus-frame802154.h"
#include "global-parameters.h"
#include "packet.h"
#include "packet-defs.h"
#include "nodes.h"


#define DEBUG IOTUS_PRINT_IMMEDIATELY
#define THIS_LOG_FILE_NAME_DESCRITOR "rad802frmr"
#include "safe-printer.h"
#define PRINTF(...)
#define PRINTADDR(addr)

/**  \brief The sequence number (0x00 - 0xff) added to the transmitted
 *   data or MAC command frame. The default is a random value within
 *   the range.
 */
static uint8_t mac_dsn;

static uint8_t initialized = 0;

/*---------------------------------------------------------------------------*/
static int
create_frame(iotus_packet_t *packet, Boolean doCreate)
{
  frame802154_t params;
  int hdr_len;

  if(frame802154_get_pan_id() == 0xffff) {
    return -1;
  }

  /* init to zeros */
  memset(&params, 0, sizeof(params));

  if(!initialized) {
    initialized = 1;
    mac_dsn = random_rand() & 0xff;
  }

  /* Build the FCF. */
  params.fcf.frame_type = packet->type;
  params.fcf.frame_pending = packet_get_parameter(packet,PACKET_PARAMETERS_PENDING);
  if(packet_holds_broadcast(packet)) {
    params.fcf.ack_required = 0;
    /* Suppress seqno on broadcast if supported (frame v2 or more) */
    params.fcf.sequence_number_suppression = FRAME802154_VERSION >= FRAME802154_IEEE802154E_2012;
  } else {
    params.fcf.ack_required = packet_get_parameter(packet,PACKET_PARAMETERS_WAIT_FOR_ACK);
    params.fcf.sequence_number_suppression = FRAME802154_SUPPR_SEQNO;
  }
  /* We do not compress PAN ID in outgoing frames, i.e. include one PAN ID (dest by default)
   * There is one exception, seemingly a typo in Table 2a: rows 2 and 3: when there is no
   * source nor destination address, we have dest PAN ID iff compression is *set*. */
  params.fcf.panid_compression = 0;

  /* Insert IEEE 802.15.4 version bits. */
  params.fcf.frame_version = FRAME802154_VERSION;
  
#if LLSEC802154_USES_AUX_HEADER
  if(packetbuf_attr(PACKETBUF_ATTR_SECURITY_LEVEL)) {
    params.fcf.security_enabled = 1;
  }
  /* Setting security-related attributes */
  params.aux_hdr.security_control.security_level = packetbuf_attr(PACKETBUF_ATTR_SECURITY_LEVEL);
#if LLSEC802154_USES_FRAME_COUNTER
  params.aux_hdr.frame_counter.u16[0] = packetbuf_attr(PACKETBUF_ATTR_FRAME_COUNTER_BYTES_0_1);
  params.aux_hdr.frame_counter.u16[1] = packetbuf_attr(PACKETBUF_ATTR_FRAME_COUNTER_BYTES_2_3);
#else /* LLSEC802154_USES_FRAME_COUNTER */
  params.aux_hdr.security_control.frame_counter_suppression = 1;
  params.aux_hdr.security_control.frame_counter_size = 1;
#endif /* LLSEC802154_USES_FRAME_COUNTER */
#if LLSEC802154_USES_EXPLICIT_KEYS
  params.aux_hdr.security_control.key_id_mode = packetbuf_attr(PACKETBUF_ATTR_KEY_ID_MODE);
  params.aux_hdr.key_index = packetbuf_attr(PACKETBUF_ATTR_KEY_INDEX);
  params.aux_hdr.key_source.u16[0] = packetbuf_attr(PACKETBUF_ATTR_KEY_SOURCE_BYTES_0_1);
#endif /* LLSEC802154_USES_EXPLICIT_KEYS */
#endif /* LLSEC802154_USES_AUX_HEADER */

  /* Increment and set the data sequence number. */
  if(FALSE == doCreate) {
    /* Only length calculation - no sequence number is needed and
       should not be consumed. */
  } else if(packet_get_sequence_number(packet)>0) {
    params.seq = packet_get_sequence_number(packet);

  } else {
    /* Ensure that the sequence number 0 is not used as it would bypass the above check. */
    if(mac_dsn == 0) {
      mac_dsn++;
    }
    params.seq = mac_dsn++;
    packet_set_sequence_number(packet, params.seq);
  }

  /* Complete the addressing fields. */
  /**
     \todo For phase 1 the addresses are all long. We'll need a mechanism
     in the rime attributes to tell the mac to use long or short for phase 2.
   */
  if(iotus_radio_selected_address_type == IOTUS_ADDRESSES_TYPE_ADDR_SHORT) {
    /* Use short address mode if linkaddr size is short. */
    params.fcf.src_addr_mode = FRAME802154_SHORTADDRMODE;
  } else {
    params.fcf.src_addr_mode = FRAME802154_LONGADDRMODE;
  }

  uint8_t *tempPanID = nodes_get_address(IOTUS_NODES_ADD_INFO_TYPE_ADDR_PANID,
                                         packet->nextDestinationNode);
  if(NULL == tempPanID) {
    tempPanID = addresses_self_get_pointer(IOTUS_ADDRESSES_TYPE_ADDR_PANID);
  }
  params.dest_pid = *((uint16_t *)tempPanID);

  if(packet_holds_broadcast(packet)) {
    /* Broadcast requires short address mode. */
    params.fcf.dest_addr_mode = FRAME802154_SHORTADDRMODE;
    params.dest_addr[0] = 0xFF;
    params.dest_addr[1] = 0xFF;
  } else {
    memcpy(&params.dest_addr,
            nodes_get_address(iotus_radio_selected_address_type,
                              packet->nextDestinationNode),
            ADDRESSES_GET_TYPE_SIZE(iotus_radio_selected_address_type));
    /* Use short address mode if linkaddr size is small */
    if(iotus_radio_selected_address_type == IOTUS_ADDRESSES_TYPE_ADDR_SHORT) {
      params.fcf.dest_addr_mode = FRAME802154_SHORTADDRMODE;
    } else {
      params.fcf.dest_addr_mode = FRAME802154_LONGADDRMODE;
    }
  }

  /* Set the source PAN ID to the global variable. */
  params.src_pid = *((uint16_t *)addresses_self_get_pointer(IOTUS_ADDRESSES_TYPE_ADDR_PANID));

  /*
   * Set up the source address using only the long address mode for
   * phase 1.
   */
  memcpy(&params.dest_addr,
          addresses_self_get_pointer(iotus_radio_selected_address_type),
          ADDRESSES_GET_TYPE_SIZE(iotus_radio_selected_address_type));

  params.payload = pieces_get_data_pointer(packet);
  params.payload_len = packet_get_size(packet);
  hdr_len = frame802154_hdrlen(&params);
  if(FALSE == doCreate) {
    /* Only calculate header length */
    return hdr_len;
  }
  if(TRUE == packet_has_space(packet, hdr_len)) {
    uint8_t header[hdr_len];
    frame802154_create(&params, header);

    printf("data len %u\n",hdr_len );
    printf(" data header: ");
    for(int k=0;k<hdr_len;k++) {
      printf("%02x ",header[k]);
    }
    printf("\nfim\n");

    uint8_t packetNewSize = packet_get_size(packet)+hdr_len;
    if(packetNewSize != packet_push_bit_header(hdr_len*8, header, packet)) {
      SAFE_PRINTF_LOG_ERROR("Diff hdr input");
      return FRAMER_FAILED;
    }
    PRINTF("15.4-OUT: %2X", params.fcf.frame_type);
    PRINTADDR(params.dest_addr);
    PRINTF("%d (%u)\n", hdr_len, packet_get_size(packet));

    return hdr_len;
  } else {
    PRINTF("15.4-OUT: too large header: %u\n", hdr_len);
    return FRAMER_FAILED;
  }
}
/*---------------------------------------------------------------------------*/
static int
hdr_length(iotus_packet_t *packet)
{
  return create_frame(packet, FALSE);
}

static int
create(iotus_packet_t *packet)
{
  return create_frame(packet, TRUE);
}

/*---------------------------------------------------------------------------*/
static int
parse(iotus_packet_t *packet)
{
  return 0;
}
/*---------------------------------------------------------------------------*/
const struct framer radio_framer_802154 = {
  hdr_length,
  create,
  parse
};
/*---------------------------------------------------------------------------*/
