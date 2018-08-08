/**
 * \defgroup decription...
 *
 * This...
 *
 * @{
 */

/*
 * packet.h
 *
 *  Created on: Nov 15, 2017
 *      Author: vinicius
 */

#ifndef IOTUS_ARCH_GLOBAL_FUNCTIONS_GLOBAL_FUNCTIONS_H_
#define IOTUS_ARCH_GLOBAL_FUNCTIONS_GLOBAL_FUNCTIONS_H_

#include "sys/rtimer.h"
#define member_size(type, member) sizeof(((type *)0)->member)


extern rtimer_clock_t packetBuildingTime;
extern uint32_t elapsedBuild;

////////////////////////////////////////////////////////////
//             TEMPORARY MEASURES                         //
////////////////////////////////////////////////////////////
#define TIC() packetBuildingTime = RTIMER_NOW()
#define TOC(msg) elapsedBuild = (1000000*(RTIMER_NOW() - packetBuildingTime))/RTIMER_ARCH_SECOND;\
      printf("%s %lu\n",msg,elapsedBuild)

uint16_t
get_safe_pdu_for_layer(uint8_t layer);

#endif /* IOTUS_ARCH_GLOBAL_FUNCTIONS_GLOBAL_FUNCTIONS_H_ */

/* The following stuff ends the \defgroup block at the beginning of
   the file: */

/** @} */
