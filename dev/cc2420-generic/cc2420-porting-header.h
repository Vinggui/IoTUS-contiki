/*
 * cc2420-porting-header.h
 *
 *  Created on: 15/12/2016
 *      Author: vinicius
 */

#ifndef DEV_CC2420_GENERIC_CC2420_PORTING_HEADER_H_
#define DEV_CC2420_GENERIC_CC2420_PORTING_HEADER_H_

#ifdef PACKETBUF_H_
    #define USE_PACKETBUF 1
#else
    #define USE_PACKETBUF 0
#endif

#if USE_EDYTEE_COMM == 1
    #define PORTABLE_GET_RADIO_TXPOWER()    1
    #define PORTABLE_ADD_CONTENTION_ATT()
    #define PORTABLE_ADD_LLTX_ATT()
    #define PORTABLE_CLEAR_PACKET_BUFF()
    #define PORTABLE_TIMESTAMP_PACKET_BUFF(timestamp)
    #define PORTABLE_PACKET_BUFF_DATAPTR()
    #define PORTABLE_PACKET_BUFF_SIZE     1
    #define PORTABLE_PACKET_BUFF_SET_DATALEN(len)   len
    #define PORTABLE_ADD_BADSYNCH_ATT()
    #define PORTABLE_ADD_TOOSHORT_ATT()
    #define PORTABLE_ADD_TOOLONG_ATT()
    #define PORTABLE_ADD_LLRX_ATT()
    #define PORTABLE_ADD_BADCRC_ATT()
    #define PORTABLE_PACKET_BUFF_SET_RSSI(rssi)
    #define PORTABLE_PACKET_BUFF_SET_LINK_QUALITY(lq)
#elif USE_PACKETBUF == 1
    #define PORTABLE_GET_RADIO_TXPOWER()    packetbuf_attr(PACKETBUF_ATTR_RADIO_TXPOWER)
    #define PORTABLE_ADD_CONTENTION_ATT()   RIMESTATS_ADD(contentiondrop)
    #define PORTABLE_ADD_LLTX_ATT()         RIMESTATS_ADD(lltx)
    #define PORTABLE_CLEAR_PACKET_BUFF()    packetbuf_clear()
    #define PORTABLE_TIMESTAMP_PACKET_BUFF(timestamp)    packetbuf_set_attr(PACKETBUF_ATTR_TIMESTAMP, timestamp)
    #define PORTABLE_PACKET_BUFF_DATAPTR()  packetbuf_dataptr()
    #define PORTABLE_PACKET_BUFF_SIZE       PACKETBUF_SIZE
    #define PORTABLE_PACKET_BUFF_SET_DATALEN(len)   packetbuf_set_datalen(len)
    #define PORTABLE_ADD_BADSYNCH_ATT()     RIMESTATS_ADD(badsynch)
    #define PORTABLE_ADD_TOOSHORT_ATT()     RIMESTATS_ADD(tooshort)
    #define PORTABLE_ADD_TOOLONG_ATT()     RIMESTATS_ADD(toolong)
    #define PORTABLE_ADD_LLRX_ATT()         RIMESTATS_ADD(llrx)
    #define PORTABLE_ADD_BADCRC_ATT()     RIMESTATS_ADD(badcrc)
    #define PORTABLE_PACKET_BUFF_SET_RSSI(rssi)     packetbuf_set_attr(PACKETBUF_ATTR_RSSI, rssi)
    #define PORTABLE_PACKET_BUFF_SET_LINK_QUALITY(lq)     packetbuf_set_attr(PACKETBUF_ATTR_LINK_QUALITY, lq)
    #define PORTABLE_PACKET_BUFF_SIZE()     PACKETBUF_SIZE
#endif

#endif /* DEV_CC2420_GENERIC_CC2420_PORTING_HEADER_H_ */
