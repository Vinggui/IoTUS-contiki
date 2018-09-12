/*
 * Copyright (c) 2010, Swedish Institute of Computer Science.
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
 */

/**
 * \file
 *         Powertrace: periodically print out power consumption
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "contiki.h"
#include "contiki-lib.h"
#include "sys/compower.h"
#include "powertrace.h"
#include "net/rime/rime.h"

#include <stdio.h>
#include <string.h>

struct powertrace_sniff_stats {
  struct powertrace_sniff_stats *next;
  unsigned long num_input, num_output;
  unsigned long input_txtime, input_rxtime;
  unsigned long output_txtime, output_rxtime;
#if NETSTACK_CONF_WITH_IPV6
  uint16_t proto; /* includes proto + possibly flags */
#endif
  uint16_t channel;
  unsigned long last_input_txtime, last_input_rxtime;
  unsigned long last_output_txtime, last_output_rxtime;
};

#define INPUT  1
#define OUTPUT 0

#define MAX_NUM_STATS  16

LIST(stats_list);

PROCESS(powertrace_process, "Periodic power output");
/*---------------------------------------------------------------------------*/
void
powertrace_print(char *str)
{
  static unsigned long last_cpu, last_lpm, last_transmit, last_listen;
  static unsigned long last_idle_transmit, last_idle_listen;

  unsigned long cpu, lpm, transmit, listen;
  unsigned long all_cpu, all_lpm, all_transmit, all_listen;
  unsigned long idle_transmit, idle_listen;
  unsigned long all_idle_transmit, all_idle_listen;

  static unsigned long seqno;

  unsigned long radio, all_radio;
  
  struct powertrace_sniff_stats *s;

  energest_flush();

  all_cpu = energest_type_time(ENERGEST_TYPE_CPU);
  all_lpm = energest_type_time(ENERGEST_TYPE_LPM);
  all_transmit = energest_type_time(ENERGEST_TYPE_TRANSMIT);
  all_listen = energest_type_time(ENERGEST_TYPE_LISTEN);
  all_idle_transmit = compower_idle_activity.transmit;
  all_idle_listen = compower_idle_activity.listen;

  cpu = all_cpu - last_cpu;
  lpm = all_lpm - last_lpm;
  transmit = all_transmit - last_transmit;
  listen = all_listen - last_listen;
  idle_transmit = compower_idle_activity.transmit - last_idle_transmit;
  idle_listen = compower_idle_activity.listen - last_idle_listen;

  last_cpu = energest_type_time(ENERGEST_TYPE_CPU);
  last_lpm = energest_type_time(ENERGEST_TYPE_LPM);
  last_transmit = energest_type_time(ENERGEST_TYPE_TRANSMIT);
  last_listen = energest_type_time(ENERGEST_TYPE_LISTEN);
  last_idle_listen = compower_idle_activity.listen;
  last_idle_transmit = compower_idle_activity.transmit;

  radio = transmit + listen;
  // time = cpu + lpm;
  // all_time = all_cpu + all_lpm;
  all_radio = energest_type_time(ENERGEST_TYPE_LISTEN) +
    energest_type_time(ENERGEST_TYPE_TRANSMIT);

  printf("%s %lu P %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %u %u %u\n",
         str,
         clock_time(), seqno,
         all_cpu, all_lpm, all_transmit, all_listen, all_idle_transmit, all_idle_listen,
         cpu, lpm, transmit, listen, idle_transmit, idle_listen,
         gPkt_tx_successful,
         gpkt_tx_attemps);

  for(s = list_head(stats_list); s != NULL; s = list_item_next(s)) {

#if ! NETSTACK_CONF_WITH_IPV6
    printf("%s %lu SP %lu %u %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu (channel %d radio %d.%02d%% / %d.%02d%%)\n",
           str, clock_time(), seqno,
           s->channel,
           s->num_input, s->input_txtime, s->input_rxtime,
           s->input_txtime - s->last_input_txtime,
           s->input_rxtime - s->last_input_rxtime,
           s->num_output, s->output_txtime, s->output_rxtime,
           s->output_txtime - s->last_output_txtime,
           s->output_rxtime - s->last_output_rxtime,
           s->channel,
           (int)((100L * (s->input_rxtime + s->input_txtime + s->output_rxtime + s->output_txtime)) / all_radio),
           (int)((10000L * (s->input_rxtime + s->input_txtime + s->output_rxtime + s->output_txtime)) / all_radio),
           (int)((100L * (s->input_rxtime + s->input_txtime +
                          s->output_rxtime + s->output_txtime -
                          (s->last_input_rxtime + s->last_input_txtime +
                           s->last_output_rxtime + s->last_output_txtime))) /
                 radio),
           (int)((10000L * (s->input_rxtime + s->input_txtime +
                          s->output_rxtime + s->output_txtime -
                          (s->last_input_rxtime + s->last_input_txtime +
                           s->last_output_rxtime + s->last_output_txtime))) /
                 radio));
#else
    printf("%s %lu SP %lu %u %u %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu (proto %u(%u) radio %d.%02d%% / %d.%02d%%)\n",
           str, clock_time(), seqno,
           s->proto, s->channel,
           s->num_input, s->input_txtime, s->input_rxtime,
           s->input_txtime - s->last_input_txtime,
           s->input_rxtime - s->last_input_rxtime,
           s->num_output, s->output_txtime, s->output_rxtime,
           s->output_txtime - s->last_output_txtime,
           s->output_rxtime - s->last_output_rxtime,
           s->proto, s->channel,
           (int)((100L * (s->input_rxtime + s->input_txtime + s->output_rxtime + s->output_txtime)) / all_radio),
           (int)((10000L * (s->input_rxtime + s->input_txtime + s->output_rxtime + s->output_txtime)) / all_radio),
           (int)((100L * (s->input_rxtime + s->input_txtime +
                          s->output_rxtime + s->output_txtime -
                          (s->last_input_rxtime + s->last_input_txtime +
                           s->last_output_rxtime + s->last_output_txtime))) /
                 radio),
           (int)((10000L * (s->input_rxtime + s->input_txtime +
                          s->output_rxtime + s->output_txtime -
                          (s->last_input_rxtime + s->last_input_txtime +
                           s->last_output_rxtime + s->last_output_txtime))) /
                 radio));
#endif
    s->last_input_txtime = s->input_txtime;
    s->last_input_rxtime = s->input_rxtime;
    s->last_output_txtime = s->output_txtime;
    s->last_output_rxtime = s->output_rxtime;
    
  }
  seqno++;
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(powertrace_process, ev, data)
{
  static struct etimer periodic;
  clock_time_t *period;
  PROCESS_BEGIN();

  period = data;

  if(period == NULL) {
    PROCESS_EXIT();
  }
  etimer_set(&periodic, *period);

  while(1) {
    PROCESS_WAIT_UNTIL(etimer_expired(&periodic));
    etimer_reset(&periodic);
    powertrace_print("PT");
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
void
powertrace_start(clock_time_t period)
{
  process_start(&powertrace_process, (void *)&period);
}
/*---------------------------------------------------------------------------*/
void
powertrace_stop(void)
{
  process_exit(&powertrace_process);
}
/*---------------------------------------------------------------------------*/
