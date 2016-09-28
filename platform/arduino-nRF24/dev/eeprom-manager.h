/*
 * eeprom-manager.h
 *
 *  Created on: 28/09/2015
 *      Author: vinicius
 */

#ifndef IAUT_RADIO_GATEWAY_EEPROM_MANAGER_H_
#define IAUT_RADIO_GATEWAY_EEPROM_MANAGER_H_

#include "platform-conf.h"
#include "mini-iic-protocol-def.h"

//#define EEPROM_NET_ADDRESS                      0
#define EEPROM_IIC_ADDRESS                      (MINI_IIC_NET_ADDRESS_SIZE)
#define EEPROM_IAUT_PASSWORD                    (EEPROM_IIC_ADDRESS+MINI_IIC_ADDRESS_SIZE)
#if (MINI_IIC_USE_PASSWORD != 0 && defined(MINI_IIC_PASSWORD_SIZE))
    #define EEPROM_DEVICES_IIC_ADDRESSES        (EEPROM_IAUT_PASSWORD+MINI_IIC_PASSWORD_SIZE)
#else
    #define EEPROM_DEVICES_IIC_ADDRESSES        (EEPROM_IAUT_PASSWORD)
#endif
#define EEPROM_APPLICATION_SPACE                (EEPROM_DEVICES_IIC_ADDRESSES+MINI_IIC_EEPROM_MEM_SIZE)

#endif /* EXAMPLES_IAUT_RADIO_GATEWAY_EEPROM_MANAGER_H_ */
