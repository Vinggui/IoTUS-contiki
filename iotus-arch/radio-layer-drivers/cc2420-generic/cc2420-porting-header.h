/*
 * cc2420-porting-header.h
 *
 *  Created on: 15/12/2016
 *      Author: vinicius
 */

#ifndef DEV_CC2420_GENERIC_CC2420_PORTING_HEADER_H_
#define DEV_CC2420_GENERIC_CC2420_PORTING_HEADER_H_
#include "../../edytee-files/edytee.h"

#if CONTIKI_COMM_STACK == IoTUS
    #define PORTABLE_ADD_CONTENTION_ATT()
    #define PORTABLE_ADD_LLTX_ATT()
    #define PORTABLE_PACKET_BUFF_SIZE     1
    #define PORTABLE_ADD_BADSYNCH_ATT()
    #define PORTABLE_ADD_TOOSHORT_ATT()
    #define PORTABLE_ADD_TOOLONG_ATT()
    #define PORTABLE_ADD_LLRX_ATT()
    #define PORTABLE_ADD_BADCRC_ATT()
#elif CONTIKI_WITH_RIME == 0
    #define PORTABLE_ADD_CONTENTION_ATT()   RIMESTATS_ADD(contentiondrop)
    #define PORTABLE_ADD_LLTX_ATT()         RIMESTATS_ADD(lltx)
    #define PORTABLE_ADD_BADSYNCH_ATT()     RIMESTATS_ADD(badsynch)
    #define PORTABLE_ADD_TOOSHORT_ATT()     RIMESTATS_ADD(tooshort)
    #define PORTABLE_ADD_TOOLONG_ATT()      RIMESTATS_ADD(toolong)
    #define PORTABLE_ADD_LLRX_ATT()         RIMESTATS_ADD(llrx)
    #define PORTABLE_ADD_BADCRC_ATT()       RIMESTATS_ADD(badcrc)
#endif

#endif /* DEV_CC2420_GENERIC_CC2420_PORTING_HEADER_H_ */
