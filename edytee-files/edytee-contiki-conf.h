/*
 * edytee-contiki-conf.h
 *
 *  Created on: 12/01/2017
 *      Author: vinicius
 */

#ifndef EDYTEE_FILES_EDYTEE_CONTIKI_CONF_H_
#define EDYTEE_FILES_EDYTEE_CONTIKI_CONF_H_


#define CONTIKI_WITHOUT_NETWORK             1
#define UIP_CONF_TCP                        0
#define UIP_CONF_UDP                        0
#define WITH_NULLMAC                        0


#define NETSTACK_CONF_NETWORK edytee_net_driver
#define NETSTACK_CONF_MAC     edytee_mac_driver
#define NETSTACK_CONF_RDC     edytee_rdc_driver
#define NETSTACK_CONF_FRAMER  framer_edytee_802154


#define PACKETBUF_CONF_WITH_PACKET_TYPE             1//No value used
#define PACKETBUF_CONF_ATTRS_INLINE                 1

#endif /* EDYTEE_FILES_EDYTEE_CONTIKI_CONF_H_ */
