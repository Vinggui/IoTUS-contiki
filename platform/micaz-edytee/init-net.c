/*
 * Copyright (c) 2010, University of Colombo School of Computing
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 * @(#)$$
 */

/**
 * \file
 *         Network initialization for the MICAz port.
 * \author
 *         Kasun Hewage <kasun.ch@gmail.com>
 */

#include <stdio.h>
#include <string.h>
#include <avr/pgmspace.h>

#include "contiki.h"
#include "cc2420.h"
#include "dev/rs232.h"
#include "dev/slip.h"
#include "dev/leds.h"
#include "net/netstack.h"
#include "net/mac/frame802154.h"
#include "net/linkaddr.h"
#include "net/queuebuf.h"

#include "dev/ds2401.h"
#include "sys/node-id.h"

#define UIP_OVER_MESH_CHANNEL 8

/*---------------------------------------------------------------------------*/
static void
set_rime_addr(void)
{
  linkaddr_t addr;
  int i;

  memset(&addr, 0, sizeof(linkaddr_t));
#if NETSTACK_CONF_WITH_IPV6
  memcpy(addr.u8, ds2401_id, sizeof(addr.u8));
#else
  if(node_id == 0) {
    for(i = 0; i < sizeof(linkaddr_t); ++i) {
      addr.u8[i] = ds2401_id[7 - i];
    }
  } else {
    addr.u8[0] = node_id & 0xff;
    addr.u8[1] = node_id >> 8;
  }
#endif
  linkaddr_set_node_addr(&addr);
  printf_P(PSTR("Rime started with address "));
  for(i = 0; i < sizeof(addr.u8) - 1; i++) {
    printf_P(PSTR("%d."), addr.u8[i]);
  }
  printf_P(PSTR("%d\n"), addr.u8[i]);
}

/*--------------------------------------------------------------------------*/
#if NETSTACK_CONF_WITH_IPV4
static void
set_gateway(void)
{
  if(!is_gateway) {
    leds_on(LEDS_RED);
    printf_P(PSTR("%d.%d: making myself the IP network gateway.\n\n"),
	              linkaddr_node_addr.u8[0], linkaddr_node_addr.u8[1]);
    printf_P(PSTR("IPv4 address of the gateway: %d.%d.%d.%d\n\n"),
	              uip_ipaddr_to_quad(&uip_hostaddr));
    uip_over_mesh_set_gateway(&linkaddr_node_addr);
    uip_over_mesh_make_announced_gateway();
    is_gateway = 1;
  }
}
#endif /* NETSTACK_CONF_WITH_IPV4 */
/*---------------------------------------------------------------------------*/
void
init_net(void)
{

  //set_rime_addr();
  cc2420_init();
  {
    uint8_t longaddr[8];
    uint16_t shortaddr;
    
    shortaddr = (linkaddr_node_addr.u8[0] << 8) +
                 linkaddr_node_addr.u8[1];
    memset(longaddr, 0, sizeof(longaddr));
    linkaddr_copy((linkaddr_t *)&longaddr, &linkaddr_node_addr);
    printf_P(PSTR("MAC %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n"),
             longaddr[0], longaddr[1], longaddr[2], longaddr[3],
             longaddr[4], longaddr[5], longaddr[6], longaddr[7]);
    
    cc2420_set_pan_addr(IEEE802154_PANID, shortaddr, longaddr);
  }

  NETSTACK_RDC.init();
  NETSTACK_MAC.init();
  NETSTACK_NETWORK.init();

  printf_P(PSTR("%s %s, channel check rate %d Hz, radio channel %d\n"),
         NETSTACK_MAC.name, NETSTACK_RDC.name,
         CLOCK_SECOND / (NETSTACK_RDC.channel_check_interval() == 0? 1:
                         NETSTACK_RDC.channel_check_interval()),
         CC2420_CONF_CHANNEL);
}
/*---------------------------------------------------------------------------*/
