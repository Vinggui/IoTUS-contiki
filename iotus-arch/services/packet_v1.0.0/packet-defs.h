/*
 * packet-def.h
 *
 *  Created on: Jan 31, 2018
 *      Author: vinicius
 */

#ifndef IOTUS_ARCH_SERVICES_PACKET_SIMPLE_LOCAL_PACKET_DEF_H_
#define IOTUS_ARCH_SERVICES_PACKET_SIMPLE_LOCAL_PACKET_DEF_H_

//////////////////////////////////////////////////////////////
//                    Packet parameters                     //
//////////////////////////////////////////////////////////////
#define PACKET_PARAMETERS_ALLOW_AGGREGATION         0b10000000
#define PACKET_PARAMETERS_ALLOW_FRAGMENTATION       0b01000000
#define PACKET_PARAMETERS_ALLOW_PIGGYBACK           0b00100000
/*
 * New packet system requires that the radio add at least a single byte header.
 * That means that some packets can still cancel the use of the new header
 */
#define PACKET_PARAMETERS_IS_NEW_PACKET_SYSTEM      0b00010000


/////////////////////////////////////////////////////////////
//              Packet iotus dynamic header                //
/////////////////////////////////////////////////////////////
#define PACKET_IOTUS_HDR_FIRST_BIT          0b00001000
#define PACKET_IOTUS_HDR_FIRST_BIT_POS      4

#define PACKET_IOTUS_HDR_NEIGHBOR_DISC      0b00000100
#define PACKET_IOTUS_HDR_HAS_PIGGYBACK      0b00000010
#define PACKET_IOTUS_HDR_IS_BROADCAST       0b00000001


#endif /* IOTUS_ARCH_SERVICES_PACKET_SIMPLE_LOCAL_PACKET_DEF_H_ */


/* The following stuff ends the \defgroup block at the beginning of
   the file: */

/** @} */
