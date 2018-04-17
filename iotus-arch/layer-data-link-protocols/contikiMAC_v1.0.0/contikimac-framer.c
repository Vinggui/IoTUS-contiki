/**
 * \defgroup decription...
 *
 * This...
 *
 * @{
 */

/*
 * contikimac-framer.h
 *
 *  Created on: Apr 11, 2017
 *      Author: vinicius
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "contikimac-framer.h"
#include "iotus-radio.h"

#define CONTIKIMAC_ID 0x00

/* SHORTEST_PACKET_SIZE is the shortest packet that ContikiMAC
   allows. Packets have to be a certain size to be able to be detected
   by two consecutive CCA checks, and here is where we define this
   shortest size.
   Padded packets will have the wrong ipv6 checksum unless CONTIKIMAC_HEADER
   is used (on both sides) and the receiver will ignore them.
   With no header, reduce to transmit a proper multicast RPL DIS. */
#ifdef CONTIKIMAC_FRAMER_CONF_SHORTEST_PACKET_SIZE
#define SHORTEST_PACKET_SIZE CONTIKIMAC_FRAMER_CONF_SHORTEST_PACKET_SIZE
#else /* CONTIKIMAC_FRAMER_CONF_SHORTEST_PACKET_SIZE */
#define SHORTEST_PACKET_SIZE 43
#endif /* CONTIKIMAC_FRAMER_CONF_SHORTEST_PACKET_SIZE */

#define DECORATED_FRAMER    (*(active_radio_driver->radio_framer))

#define DEBUG IOTUS_PRINT_IMMEDIATELY
#define THIS_LOG_FILE_NAME_DESCRITOR "ctkMAC-Frmr"
#include "safe-printer.h"

#define PRINTF(...)

/* 2-byte header for recovering padded packets.
   Wireshark will not understand such packets at present. */
struct hdr {
  uint8_t id;
  uint8_t len;
};

/*---------------------------------------------------------------------------*/
static int
hdr_length(void)
{
  return DECORATED_FRAMER.length() + sizeof(struct hdr);
}
/*---------------------------------------------------------------------------*/
static void
pad(iotus_packet_t *packet)
{
  int transmit_len;
  //uint8_t *ptr;
  uint8_t zeroes_count;
  
  transmit_len = packet_get_size(packet) + hdr_length();
  if(transmit_len < SHORTEST_PACKET_SIZE) {
    /* Padding required */
    zeroes_count = SHORTEST_PACKET_SIZE - transmit_len;

    // ptr = packetbuf_dataptr();
    // memset(ptr + packetbuf_datalen(), 0, zeroes_count);
    // packetbuf_set_datalen(packetbuf_datalen() + zeroes_count);
    uint8_t zeroArray[zeroes_count];
    memset(zeroArray, 0, zeroes_count);
    if(zeroes_count == packet_append_last_header(zeroes_count, zeroArray, packet)) {
      SAFE_PRINTF_LOG_ERROR("Zeros not appended");
    }
  }
}
/*---------------------------------------------------------------------------*/
static int
create(iotus_packet_t *packet)
{
  struct hdr chdr;
  int hdr_len;
  
  if(FALSE == packet_has_space(packet,sizeof(struct hdr))) {
    PRINTF("contikimac-framer: too large header\n");
    return FRAMER_FAILED;
  }

  chdr.id = CONTIKIMAC_ID;
  chdr.len = packet_get_size(packet);
  packet_push_bit_header(16, (uint8_t *)&chdr, packet);
  pad(packet);
  
  hdr_len = DECORATED_FRAMER.create(packet);
  if(hdr_len < 0) {
    PRINTF("contikimac-framer: decorated framer failed\n");
    return FRAMER_FAILED;
  }
  
  //packetbuf_compact();
  
  return hdr_len + sizeof(struct hdr);
}
/*---------------------------------------------------------------------------*/
static int
parse(iotus_packet_t *packet)
{
  return 0;
}
/*---------------------------------------------------------------------------*/
const struct framer contikimac_framer = {
  hdr_length,
  create,
  parse
};
/*---------------------------------------------------------------------------*/
