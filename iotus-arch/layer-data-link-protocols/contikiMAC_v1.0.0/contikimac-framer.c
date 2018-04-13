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

#include "contikimac-framer.h"
#include <string.h>

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

#ifdef CONTIKIMAC_FRAMER_CONF_DECORATED_FRAMER
#define DECORATED_FRAMER CONTIKIMAC_FRAMER_CONF_DECORATED_FRAMER
#else /* CONTIKIMAC_FRAMER_CONF_DECORATED_FRAMER */
#define DECORATED_FRAMER framer_802154
#endif /* CONTIKIMAC_FRAMER_CONF_DECORATED_FRAMER */

extern const struct framer DECORATED_FRAMER;


#define DEBUG IOTUS_PRINT_IMMEDIATELY
#define THIS_LOG_FILE_NAME_DESCRITOR "ctkMAC-Frmr"
#include "safe-printer.h"
#define PRINTF(...)
static void pad(void);

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
static int
create(void)
{
  struct hdr *chdr;
  int hdr_len;
  
  if(packetbuf_hdralloc(sizeof(struct hdr)) == 0) {
    PRINTF("contikimac-framer: too large header\n");
    return FRAMER_FAILED;
  }
  chdr = packetbuf_hdrptr();
  chdr->id = CONTIKIMAC_ID;
  chdr->len = packetbuf_datalen();
  pad();
  
  hdr_len = DECORATED_FRAMER.create();
  if(hdr_len < 0) {
    PRINTF("contikimac-framer: decorated framer failed\n");
    return FRAMER_FAILED;
  }
  
  packetbuf_compact();
  
  return hdr_len + sizeof(struct hdr);
}
/*---------------------------------------------------------------------------*/
static void
pad(void)
{
  int transmit_len;
  uint8_t *ptr;
  uint8_t zeroes_count;
  
  transmit_len = packetbuf_totlen() + hdr_length();
  if(transmit_len < SHORTEST_PACKET_SIZE) {
    /* Padding required */
    zeroes_count = SHORTEST_PACKET_SIZE - transmit_len;
    ptr = packetbuf_dataptr();
    memset(ptr + packetbuf_datalen(), 0, zeroes_count);
    packetbuf_set_datalen(packetbuf_datalen() + zeroes_count);
  }
}
/*---------------------------------------------------------------------------*/
static int
parse(void)
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
