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
#include "lib/random.h"
#include <string.h>

#define DEBUG 0


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
create_frame(iotus_packet_t *packet)
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
  params.fcf.frame_pending = packet_verify_parameter(packet,PACKET_PARAMETERS_PENDING);
  if(packet_holds_broadcast(packet)) {
    params.fcf.ack_required = 0;
    /* Suppress seqno on broadcast if supported (frame v2 or more) */
    params.fcf.sequence_number_suppression = FRAME802154_VERSION >= FRAME802154_IEEE802154E_2012;
  } else {
    params.fcf.ack_required = packet_verify_parameter(packet,PACKET_PARAMETERS_WAIT_FOR_ACK);
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
  if(!do_create) {
    /* Only length calculation - no sequence number is needed and
       should not be consumed. */

  } else if(packetbuf_attr(PACKETBUF_ATTR_MAC_SEQNO)) {
    params.seq = packetbuf_attr(PACKETBUF_ATTR_MAC_SEQNO);

  } else {
    /* Ensure that the sequence number 0 is not used as it would bypass the above check. */
    if(mac_dsn == 0) {
      mac_dsn++;
    }
    params.seq = mac_dsn++;
    packetbuf_set_attr(PACKETBUF_ATTR_MAC_SEQNO, params.seq);
  }

  /* Complete the addressing fields. */
  /**
     \todo For phase 1 the addresses are all long. We'll need a mechanism
     in the rime attributes to tell the mac to use long or short for phase 2.
   */
  if(LINKADDR_SIZE == 2) {
    /* Use short address mode if linkaddr size is short. */
    params.fcf.src_addr_mode = FRAME802154_SHORTADDRMODE;
  } else {
    params.fcf.src_addr_mode = FRAME802154_LONGADDRMODE;
  }
  params.dest_pid = frame802154_get_pan_id();

  if(packetbuf_holds_broadcast()) {
    /* Broadcast requires short address mode. */
    params.fcf.dest_addr_mode = FRAME802154_SHORTADDRMODE;
    params.dest_addr[0] = 0xFF;
    params.dest_addr[1] = 0xFF;
  } else {
    linkaddr_copy((linkaddr_t *)&params.dest_addr,
                  packetbuf_addr(PACKETBUF_ADDR_RECEIVER));
    /* Use short address mode if linkaddr size is small */
    if(LINKADDR_SIZE == 2) {
      params.fcf.dest_addr_mode = FRAME802154_SHORTADDRMODE;
    } else {
      params.fcf.dest_addr_mode = FRAME802154_LONGADDRMODE;
    }
  }

  /* Set the source PAN ID to the global variable. */
  params.src_pid = frame802154_get_pan_id();

  /*
   * Set up the source address using only the long address mode for
   * phase 1.
   */
  linkaddr_copy((linkaddr_t *)&params.src_addr,
                packetbuf_addr(PACKETBUF_ADDR_SENDER));

  params.payload = packetbuf_dataptr();
  params.payload_len = packetbuf_datalen();
  hdr_len = frame802154_hdrlen(&params);
  if(!do_create) {
    /* Only calculate header length */
    return hdr_len;
  } else if(packetbuf_hdralloc(hdr_len)) {
    frame802154_create(&params, packetbuf_hdrptr());

    PRINTF("15.4-OUT: %2X", params.fcf.frame_type);
    PRINTADDR(params.dest_addr);
    PRINTF("%d %u (%u)\n", hdr_len, packetbuf_datalen(), packetbuf_totlen());

    return hdr_len;
  } else {
    PRINTF("15.4-OUT: too large header: %u\n", hdr_len);
    return FRAMER_FAILED;
  }
}
/*---------------------------------------------------------------------------*/
static int
hdr_length(void)
{
  return 10;
}

/*---------------------------------------------------------------------------*/
static int
parse(void)
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
