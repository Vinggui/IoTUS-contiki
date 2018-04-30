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
 *         Implementation of the ContikiMAC power-saving radio duty cycling protocol
 * \author
 *         Adam Dunkels <adam@sics.se>
 *         Niclas Finne <nfi@sics.se>
 *         Joakim Eriksson <joakime@sics.se>
 */
#include "addresses.h"
#include "contiki-conf.h"
#include "contikiMAC.h"
#include "contikimac-framer.h"
#include "dev/leds.h"
#include "dev/watchdog.h"
#include "iotus-netstack.h"
#include "iotus-frame802154.h"
#include "lib/random.h"
#include "sys/pt.h"
#include "sys/rtimer.h"
#include "sys/ctimer.h"



#include <string.h>

#define DEBUG IOTUS_PRINT_IMMEDIATELY
#define THIS_LOG_FILE_NAME_DESCRITOR "contikiMAC"
#include "safe-printer.h"

#define PRINTF(...)         SAFE_PRINTF_LOG_ERROR(__VA_ARGS__)
#define PRINTDEBUG(...)     SAFE_PRINTF_LOG_ERROR(__VA_ARGS__)


/* TX/RX cycles are synchronized with neighbor wake periods */
#ifdef CONTIKIMAC_CONF_WITH_PHASE_OPTIMIZATION
#define WITH_PHASE_OPTIMIZATION      CONTIKIMAC_CONF_WITH_PHASE_OPTIMIZATION
#else /* CONTIKIMAC_CONF_WITH_PHASE_OPTIMIZATION */
#define WITH_PHASE_OPTIMIZATION      0
#endif /* CONTIKIMAC_CONF_WITH_PHASE_OPTIMIZATION */
/* More aggressive radio sleeping when channel is busy with other traffic */
#ifndef WITH_FAST_SLEEP
#define WITH_FAST_SLEEP              1
#endif
/* Radio does CSMA and autobackoff */
#ifndef RDC_CONF_HARDWARE_CSMA
#define RDC_CONF_HARDWARE_CSMA       0
#endif
/* Radio returns TX_OK/TX_NOACK after autoack wait */
#ifndef RDC_CONF_HARDWARE_ACK
#define RDC_CONF_HARDWARE_ACK        0
#endif
/* MCU can sleep during radio off */
#ifndef RDC_CONF_MCU_SLEEP
#define RDC_CONF_MCU_SLEEP           0
#endif

#if NETSTACK_RDC_CHANNEL_CHECK_RATE >= 64
#undef WITH_PHASE_OPTIMIZATION
#define WITH_PHASE_OPTIMIZATION 0
#endif

/* CYCLE_TIME for channel cca checks, in rtimer ticks. */
#ifdef CONTIKIMAC_CONF_CYCLE_TIME
#define CYCLE_TIME (CONTIKIMAC_CONF_CYCLE_TIME)
#else
#define CYCLE_TIME (RTIMER_ARCH_SECOND / NETSTACK_RDC_CHANNEL_CHECK_RATE)
#endif

/* CHANNEL_CHECK_RATE is enforced to be a power of two.
 * If RTIMER_ARCH_SECOND is not also a power of two, there will be an inexact
 * number of channel checks per second due to the truncation of CYCLE_TIME.
 * This will degrade the effectiveness of phase optimization with neighbors that
 * do not have the same truncation error.
 * Define SYNC_CYCLE_STARTS to ensure an integral number of checks per second.
 */
#if RTIMER_ARCH_SECOND & (RTIMER_ARCH_SECOND - 1)
#define SYNC_CYCLE_STARTS                    1
#endif

/* Are we currently receiving a burst? */
static int we_are_receiving_burst = 0;

/* INTER_PACKET_DEADLINE is the maximum time a receiver waits for the
   next packet of a burst when FRAME_PENDING is set. */
#ifdef CONTIKIMAC_CONF_INTER_PACKET_DEADLINE
#define INTER_PACKET_DEADLINE               CONTIKIMAC_CONF_INTER_PACKET_DEADLINE
#else
#define INTER_PACKET_DEADLINE               CLOCK_SECOND / 32
#endif

/* ContikiMAC performs periodic channel checks. Each channel check
   consists of two or more CCA checks. CCA_COUNT_MAX is the number of
   CCAs to be done for each periodic channel check. The default is
   two.*/
#ifdef CONTIKIMAC_CONF_CCA_COUNT_MAX
#define CCA_COUNT_MAX                      (CONTIKIMAC_CONF_CCA_COUNT_MAX)
#else
#define CCA_COUNT_MAX                      2
#endif

/* Before starting a transmission, Contikimac checks the availability
   of the channel with CCA_COUNT_MAX_TX consecutive CCAs */
#ifdef CONTIKIMAC_CONF_CCA_COUNT_MAX_TX
#define CCA_COUNT_MAX_TX                   (CONTIKIMAC_CONF_CCA_COUNT_MAX_TX)
#else
#define CCA_COUNT_MAX_TX                   6
#endif

/* CCA_CHECK_TIME is the time it takes to perform a CCA check. */
/* Note this may be zero. AVRs have 7612 ticks/sec, but block until cca is done */
#ifdef CONTIKIMAC_CONF_CCA_CHECK_TIME
#define CCA_CHECK_TIME                     (CONTIKIMAC_CONF_CCA_CHECK_TIME)
#else
#define CCA_CHECK_TIME                     RTIMER_ARCH_SECOND / 8192
#endif

/* CCA_SLEEP_TIME is the time between two successive CCA checks. */
/* Add 1 when rtimer ticks are coarse */
#ifdef CONTIKIMAC_CONF_CCA_SLEEP_TIME
#define CCA_SLEEP_TIME CONTIKIMAC_CONF_CCA_SLEEP_TIME
#else
#if RTIMER_ARCH_SECOND > 8000
#define CCA_SLEEP_TIME                     RTIMER_ARCH_SECOND / 2000
#else
#define CCA_SLEEP_TIME                     (RTIMER_ARCH_SECOND / 2000) + 1
#endif /* RTIMER_ARCH_SECOND > 8000 */
#endif /* CONTIKIMAC_CONF_CCA_SLEEP_TIME */

/* CHECK_TIME is the total time it takes to perform CCA_COUNT_MAX
   CCAs. */
#define CHECK_TIME                         (CCA_COUNT_MAX * (CCA_CHECK_TIME + CCA_SLEEP_TIME))

/* CHECK_TIME_TX is the total time it takes to perform CCA_COUNT_MAX_TX
   CCAs. */
#define CHECK_TIME_TX                      (CCA_COUNT_MAX_TX * (CCA_CHECK_TIME + CCA_SLEEP_TIME))

/* LISTEN_TIME_AFTER_PACKET_DETECTED is the time that we keep checking
   for activity after a potential packet has been detected by a CCA
   check. */
#ifdef CONTIKIMAC_CONF_LISTEN_TIME_AFTER_PACKET_DETECTED
#define LISTEN_TIME_AFTER_PACKET_DETECTED  CONTIKIMAC_CONF_LISTEN_TIME_AFTER_PACKET_DETECTED
#else
#define LISTEN_TIME_AFTER_PACKET_DETECTED  RTIMER_ARCH_SECOND / 80
#endif

/* MAX_SILENCE_PERIODS is the maximum amount of periods (a period is
   CCA_CHECK_TIME + CCA_SLEEP_TIME) that we allow to be silent before
   we turn of the radio. */
#ifdef CONTIKIMAC_CONF_MAX_SILENCE_PERIODS
#define MAX_SILENCE_PERIODS                CONTIKIMAC_CONF_MAX_SILENCE_PERIODS
#else
#define MAX_SILENCE_PERIODS                5
#endif

/* MAX_NONACTIVITY_PERIODS is the maximum number of periods we allow
   the radio to be turned on without any packet being received, when
   WITH_FAST_SLEEP is enabled. */
#ifdef CONTIKIMAC_CONF_MAX_NONACTIVITY_PERIODS
#define MAX_NONACTIVITY_PERIODS            CONTIKIMAC_CONF_MAX_NONACTIVITY_PERIODS
#else
#define MAX_NONACTIVITY_PERIODS            10
#endif




/* STROBE_TIME is the maximum amount of time a transmitted packet
   should be repeatedly transmitted as part of a transmission. */
#define STROBE_TIME                        (CYCLE_TIME + 2 * CHECK_TIME)

/* GUARD_TIME is the time before the expected phase of a neighbor that
   a transmitted should begin transmitting packets. */
#ifdef CONTIKIMAC_CONF_GUARD_TIME
#define GUARD_TIME                         CONTIKIMAC_CONF_GUARD_TIME
#else
#define GUARD_TIME                         10 * CHECK_TIME + CHECK_TIME_TX
#endif

/* INTER_PACKET_INTERVAL is the interval between two successive packet transmissions */
#ifdef CONTIKIMAC_CONF_INTER_PACKET_INTERVAL
#define INTER_PACKET_INTERVAL              CONTIKIMAC_CONF_INTER_PACKET_INTERVAL
#else
#define INTER_PACKET_INTERVAL              RTIMER_ARCH_SECOND / 2500
#endif

/* AFTER_ACK_DETECTED_WAIT_TIME is the time to wait after a potential
   ACK packet has been detected until we can read it out from the
   radio. */
#ifdef CONTIKIMAC_CONF_AFTER_ACK_DETECTED_WAIT_TIME
#define AFTER_ACK_DETECTED_WAIT_TIME      CONTIKIMAC_CONF_AFTER_ACK_DETECTED_WAIT_TIME
#else
#define AFTER_ACK_DETECTED_WAIT_TIME      RTIMER_ARCH_SECOND / 1500
#endif

/* MAX_PHASE_STROBE_TIME is the time that we transmit repeated packets
   to a neighbor for which we have a phase lock. */
#ifdef CONTIKIMAC_CONF_MAX_PHASE_STROBE_TIME
#define MAX_PHASE_STROBE_TIME              CONTIKIMAC_CONF_MAX_PHASE_STROBE_TIME
#else
#define MAX_PHASE_STROBE_TIME              RTIMER_ARCH_SECOND / 60
#endif

#ifdef CONTIKIMAC_CONF_SEND_SW_ACK
#define CONTIKIMAC_SEND_SW_ACK CONTIKIMAC_CONF_SEND_SW_ACK
#else
#define CONTIKIMAC_SEND_SW_ACK 0
#endif

#define ACK_LEN 3

#include <stdio.h>
static struct rtimer rt;
static struct pt pt;

static volatile uint8_t contikimac_is_on = 0;
static volatile uint8_t contikimac_keep_radio_on = 0;

static volatile unsigned char we_are_sending = 0;
static volatile unsigned char radio_is_on = 0;

static uint8_t is_receiver_awake = 0;


#if CONTIKIMAC_CONF_COMPOWER
static struct compower_activity current_packet;
#endif /* CONTIKIMAC_CONF_COMPOWER */

#if WITH_PHASE_OPTIMIZATION

#include "net/mac/phase.h"

#endif /* WITH_PHASE_OPTIMIZATION */

#define DEFAULT_STREAM_TIME (4 * CYCLE_TIME)

#if CONTIKIMAC_CONF_BROADCAST_RATE_LIMIT
static struct timer broadcast_rate_timer;
static int broadcast_rate_counter;
#endif /* CONTIKIMAC_CONF_BROADCAST_RATE_LIMIT */

/*---------------------------------------------------------------------------*/
static void
on(void)
{
  if(contikimac_is_on && radio_is_on == 0) {
    radio_is_on = 1;
    active_radio_driver->on();
  }
}
/*---------------------------------------------------------------------------*/
static void
off(void)
{
  if(contikimac_is_on && radio_is_on != 0 &&
     contikimac_keep_radio_on == 0) {
    radio_is_on = 0;
    active_radio_driver->off();
  }
}
/*---------------------------------------------------------------------------*/
static void powercycle_wrapper(struct rtimer *t, void *ptr);
static char powercycle(struct rtimer *t, void *ptr);
/*---------------------------------------------------------------------------*/
static volatile rtimer_clock_t cycle_start;
#if SYNC_CYCLE_STARTS
static volatile rtimer_clock_t sync_cycle_start;
static volatile uint8_t sync_cycle_phase;
#endif
/*---------------------------------------------------------------------------*/
static void
schedule_powercycle(struct rtimer *t, rtimer_clock_t time)
{
  int r;
  rtimer_clock_t now;

  if(contikimac_is_on) {

    time += RTIMER_TIME(t);
    now = RTIMER_NOW();
    if(RTIMER_CLOCK_LT(time, now + RTIMER_GUARD_TIME)) {
      time = now + RTIMER_GUARD_TIME;
    }

    r = rtimer_set(t, time, 1, powercycle_wrapper, NULL);

    if(r != RTIMER_OK) {
      PRINTF("schedule_powercycle: could not set rtimer\n");
    }
  }
}
/*---------------------------------------------------------------------------*/
static void
schedule_powercycle_fixed(struct rtimer *t, rtimer_clock_t fixed_time)
{
  int r;
  rtimer_clock_t now;

  if(contikimac_is_on) {

    now = RTIMER_NOW();
    if(RTIMER_CLOCK_LT(fixed_time, now + RTIMER_GUARD_TIME)) {
      fixed_time = now + RTIMER_GUARD_TIME;
    }

    r = rtimer_set(t, fixed_time, 1, powercycle_wrapper, NULL);
    if(r != RTIMER_OK) {
      PRINTF("schedule_powercycle: could not set rtimer\n");
    }
  }
}
/*---------------------------------------------------------------------------*/
static void
powercycle_turn_radio_off(void)
{
#if CONTIKIMAC_CONF_COMPOWER
  uint8_t was_on = radio_is_on;
#endif /* CONTIKIMAC_CONF_COMPOWER */

  if(we_are_sending == 0 && we_are_receiving_burst == 0) {
    off();
#if CONTIKIMAC_CONF_COMPOWER
    if(was_on && !radio_is_on) {
      compower_accumulate(&compower_idle_activity);
    }
#endif /* CONTIKIMAC_CONF_COMPOWER */
  }
}
/*---------------------------------------------------------------------------*/
static void
powercycle_turn_radio_on(void)
{
  if(we_are_sending == 0 && we_are_receiving_burst == 0) {
    on();
  }
}
/*---------------------------------------------------------------------------*/
static void
powercycle_wrapper(struct rtimer *t, void *ptr)
{
  powercycle(t, ptr);
}
/*---------------------------------------------------------------------------*/
static void
advance_cycle_start(void)
{
  #if SYNC_CYCLE_STARTS

  /* Compute cycle start when RTIMER_ARCH_SECOND is not a multiple
  of CHANNEL_CHECK_RATE */
  if(sync_cycle_phase++ == NETSTACK_RDC_CHANNEL_CHECK_RATE) {
    sync_cycle_phase = 0;
    sync_cycle_start += RTIMER_ARCH_SECOND;
    cycle_start = sync_cycle_start;
  } else if( (RTIMER_ARCH_SECOND * NETSTACK_RDC_CHANNEL_CHECK_RATE) > 65535) {
    uint32_t phase_time = sync_cycle_phase*RTIMER_ARCH_SECOND;

    cycle_start = sync_cycle_start + phase_time/NETSTACK_RDC_CHANNEL_CHECK_RATE;
  } else {
    unsigned phase_time = sync_cycle_phase*RTIMER_ARCH_SECOND;

    cycle_start = sync_cycle_start + phase_time/NETSTACK_RDC_CHANNEL_CHECK_RATE;
  }
  #endif

  cycle_start += CYCLE_TIME;
}
/*---------------------------------------------------------------------------*/
static char
powercycle(struct rtimer *t, void *ptr)
{

  PT_BEGIN(&pt);

#if SYNC_CYCLE_STARTS
  sync_cycle_start = RTIMER_NOW();
#else
  cycle_start = RTIMER_NOW();
#endif

  while(1) {
    static uint8_t packet_seen;
    static uint8_t count;

    packet_seen = 0;

    for(count = 0; count < CCA_COUNT_MAX; ++count) {
      if(we_are_sending == 0 && we_are_receiving_burst == 0) {
        powercycle_turn_radio_on();
        /* Check if a packet is seen in the air. If so, we keep the
             radio on for a while (LISTEN_TIME_AFTER_PACKET_DETECTED) to
             be able to receive the packet. We also continuously check
             the radio medium to make sure that we wasn't woken up by a
             false positive: a spurious radio interference that was not
             caused by an incoming packet. */
        if(active_radio_driver->channel_clear() == 0) {
          packet_seen = 1;
          break;
        }
        powercycle_turn_radio_off();
      }
      schedule_powercycle_fixed(t, RTIMER_NOW() + CCA_SLEEP_TIME);
      PT_YIELD(&pt);
    }

    if(packet_seen) {
      static rtimer_clock_t start;
      static uint8_t silence_periods, periods;
      start = RTIMER_NOW();

      periods = silence_periods = 0;
      while(we_are_sending == 0 && radio_is_on &&
            RTIMER_CLOCK_LT(RTIMER_NOW(),
                            (start + LISTEN_TIME_AFTER_PACKET_DETECTED))) {

        /* Check for a number of consecutive periods of
             non-activity. If we see two such periods, we turn the
             radio off. Also, if a packet has been successfully
             received (as indicated by the
             active_radio_driver->pending_packet() function), we stop
             snooping. */
#if !RDC_CONF_HARDWARE_CSMA
       /* A cca cycle will disrupt rx on some radios, e.g. mc1322x, rf230 */
       /*TODO: Modify those drivers to just return the internal RSSI when already in rx mode */
        if(active_radio_driver->channel_clear()) {
          ++silence_periods;
        } else {
          silence_periods = 0;
        }
#endif

        ++periods;

        if(active_radio_driver->receiving_packet()) {
          silence_periods = 0;
        }
        if(silence_periods > MAX_SILENCE_PERIODS) {
          powercycle_turn_radio_off();
          break;
        }
        if(WITH_FAST_SLEEP &&
            periods > MAX_NONACTIVITY_PERIODS &&
            !(active_radio_driver->receiving_packet() ||
              active_radio_driver->pending_packet())) {
          powercycle_turn_radio_off();
          break;
        }
        if(active_radio_driver->pending_packet()) {
          break;
        }

        schedule_powercycle(t, CCA_CHECK_TIME + CCA_SLEEP_TIME);
        PT_YIELD(&pt);
      }
      if(radio_is_on) {
        if(!(active_radio_driver->receiving_packet() ||
             active_radio_driver->pending_packet()) ||
             !RTIMER_CLOCK_LT(RTIMER_NOW(),
                 (start + LISTEN_TIME_AFTER_PACKET_DETECTED))) {
          powercycle_turn_radio_off();
        }
      }
    }

    advance_cycle_start();

    if(RTIMER_CLOCK_LT(RTIMER_NOW() , cycle_start - CHECK_TIME * 4)) {
      /* Schedule the next powercycle interrupt, or sleep the mcu
      until then.  Sleeping will not exit from this interrupt, so
      ensure an occasional wake cycle or foreground processing will
      be blocked until a packet is detected */
#if RDC_CONF_MCU_SLEEP

      static uint8_t sleepcycle;
      if((sleepcycle++ < 16) && !we_are_sending && !radio_is_on) {
        rtimer_arch_sleep(RTIMER_NOW() - cycle_start);
      } else {
        sleepcycle = 0;
        schedule_powercycle_fixed(t, cycle_start);
        PT_YIELD(&pt);
      }
#else
      schedule_powercycle_fixed(t, cycle_start);
      PT_YIELD(&pt);
#endif
    }
  }

  PT_END(&pt);
}
/*---------------------------------------------------------------------------*/
static int
broadcast_rate_drop(void)
{
#if CONTIKIMAC_CONF_BROADCAST_RATE_LIMIT
  if(!timer_expired(&broadcast_rate_timer)) {
    broadcast_rate_counter++;
    if(broadcast_rate_counter < CONTIKIMAC_CONF_BROADCAST_RATE_LIMIT) {
      return 0;
    } else {
      return 1;
    }
  } else {
    timer_set(&broadcast_rate_timer, CLOCK_SECOND);
    broadcast_rate_counter = 0;
    return 0;
  }
#else /* CONTIKIMAC_CONF_BROADCAST_RATE_LIMIT */
  return 0;
#endif /* CONTIKIMAC_CONF_BROADCAST_RATE_LIMIT */
}
/*---------------------------------------------------------------------------*/
//send_packet(mac_callback_t mac_callback, void *mac_callback_ptr,
//	    struct rdc_buf_list *buf_list,
//            int is_receiver_awake)
static int8_t
send_packet(iotus_packet_t *packet)
{
  rtimer_clock_t t0;
#if WITH_PHASE_OPTIMIZATION
  rtimer_clock_t encounter_time = 0;
#endif
  int strobes;
  uint8_t got_strobe_ack = 0;
  uint8_t is_broadcast = 0;
  uint8_t is_known_receiver = 0;
  uint8_t collisions;
  int ret;
  uint8_t contikimac_was_on;
#if !RDC_CONF_HARDWARE_ACK
  uint8_t seqno;
#endif

  /* Exit if RDC and radio were explicitly turned off */
   if(!contikimac_is_on && !contikimac_keep_radio_on) {
    PRINTF("contikimac: radio is turned off\n");
    return MAC_TX_ERR_FATAL;
  }

  if(packet_get_size(packet) == 0) {
    PRINTF("contikimac: send_packet data len 0\n");
    return MAC_TX_ERR_FATAL;
  }

#if !NETSTACK_CONF_BRIDGE_MODE
  /* If NETSTACK_CONF_BRIDGE_MODE is set, assume PACKETBUF_ADDR_SENDER is already set. */
  //packetbuf_set_addr(PACKETBUF_ADDR_SENDER, &linkaddr_node_addr);
  //No action is necessary here, since this procedure is verified at the packet assymbly.
#endif
  //if(packetbuf_holds_broadcast()) {
  if(TRUE == packet_holds_broadcast(packet)) {
    is_broadcast = 1;
    PRINTDEBUG("contikimac: send broadcast\n");

    if(broadcast_rate_drop()) {
      return MAC_TX_COLLISION;
    }
  } else {
    uint8_t *recvAddr;
#if NETSTACK_CONF_WITH_IPV6
    recvAddr = nodes_get_address(IOTUS_ADDRESSES_TYPE_ADDR_IPV6,
                                          packet_get_next_destination(packet));
    if(NULL != recvAddr) {
      PRINTDEBUG("contikimac: send unicast to %02x%02x:%02x%02x:%02x%02x:%02x%02x\n",
               recvAddr[0],
               recvAddr[1],
               recvAddr[2],
               recvAddr[3],
               recvAddr[4],
               recvAddr[5],
               recvAddr[6],
               recvAddr[7]);
    }
#else /* NETSTACK_CONF_WITH_IPV6 */
    recvAddr = nodes_get_address(iotus_radio_selected_address_type,
                                 packet_get_next_destination(packet));
    if(NULL != recvAddr) {
      SAFE_PRINTF_LOG_INFO("contikimac: send unicast to %u.%u\n",
                 recvAddr[0],
                 recvAddr[1]);
    }
#endif /* NETSTACK_CONF_WITH_IPV6 */
  }
  /*
  if(!packetbuf_attr(PACKETBUF_ATTR_IS_CREATED_AND_SECURED)) {
    packetbuf_set_attr(PACKETBUF_ATTR_MAC_ACK, 1);
    if(NETSTACK_FRAMER.create() < 0) {
      PRINTF("contikimac: framer failed\n");
      return MAC_TX_ERR_FATAL;
    }
  } updated to \/\/\/\/\/     */
  packet_set_parameter(packet,PACKET_PARAMETERS_WAIT_FOR_ACK);
  if(contikimac_framer.create(packet) < 0) {
    PRINTF("contikimac: framer failed\n");
    return MAC_TX_ERR_FATAL;
  }

  active_radio_driver->prepare(packet);

#if WITH_PHASE_OPTIMIZATION
  if(!is_broadcast && !is_receiver_awake) {

    ret = phase_wait(packetbuf_addr(PACKETBUF_ADDR_RECEIVER),
                     CYCLE_TIME, GUARD_TIME,
                     mac_callback, mac_callback_ptr, buf_list);
    if(ret == PHASE_DEFERRED) {
      return MAC_TX_DEFERRED;
    }
    if(ret != PHASE_UNKNOWN) {
      is_known_receiver = 1;
    }
  }
#endif /* WITH_PHASE_OPTIMIZATION */



  /* By setting we_are_sending to one, we ensure that the rtimer
     powercycle interrupt do not interfere with us sending the packet. */
  we_are_sending = 1;

  /* If we have a pending packet in the radio, we should not send now,
     because we will trash the received packet. Instead, we signal
     that we have a collision, which lets the packet be received. This
     packet will be retransmitted later by the MAC protocol
     instread. */
  if(active_radio_driver->receiving_packet() || active_radio_driver->pending_packet()) {
    we_are_sending = 0;
    PRINTF("contikimac: collision receiving %d, pending %d\n",
           active_radio_driver->receiving_packet(), active_radio_driver->pending_packet());
    return MAC_TX_COLLISION;
  }

  /* Switch off the radio to ensure that we didn't start sending while
     the radio was doing a channel check. */
  off();


  strobes = 0;

  /* Send a train of strobes until the receiver answers with an ACK. */
  collisions = 0;

  got_strobe_ack = 0;

  /* Set contikimac_is_on to one to allow the on() and off() functions
     to control the radio. We restore the old value of
     contikimac_is_on when we are done. */
  contikimac_was_on = contikimac_is_on;
  contikimac_is_on = 1;

#if !RDC_CONF_HARDWARE_CSMA
    /* Check if there are any transmissions by others. */
    /* TODO: why does this give collisions before sending with the mc1322x? */
  if(is_receiver_awake == 0) {
    int i;
    for(i = 0; i < CCA_COUNT_MAX_TX; ++i) {
      t0 = RTIMER_NOW();
      on();
#if CCA_CHECK_TIME > 0
      while(RTIMER_CLOCK_LT(RTIMER_NOW(), t0 + CCA_CHECK_TIME)) { }
#endif
      if(active_radio_driver->channel_clear() == 0) {
        collisions++;
        off();
        break;
      }
      off();
      t0 = RTIMER_NOW();
      while(RTIMER_CLOCK_LT(RTIMER_NOW(), t0 + CCA_SLEEP_TIME)) { }
    }
  }

  if(collisions > 0) {
    we_are_sending = 0;
    off();
    PRINTF("contikimac: collisions before sending\n");
    contikimac_is_on = contikimac_was_on;
    return MAC_TX_COLLISION;
  }
#endif /* RDC_CONF_HARDWARE_CSMA */

#if !RDC_CONF_HARDWARE_ACK
  if(!is_broadcast) {
    /* Turn radio on to receive expected unicast ack.  Not necessary
       with hardware ack detection, and may trigger an unnecessary cca
       or rx cycle */
     on();
  }
  seqno = packet_get_sequence_number(packet);
#endif

  watchdog_periodic();
  t0 = RTIMER_NOW();
  for(strobes = 0, collisions = 0;
      got_strobe_ack == 0 && collisions == 0 &&
      RTIMER_CLOCK_LT(RTIMER_NOW(), t0 + STROBE_TIME); strobes++) {

    watchdog_periodic();

    if(!is_broadcast && (is_receiver_awake || is_known_receiver) &&
       !RTIMER_CLOCK_LT(RTIMER_NOW(), t0 + MAX_PHASE_STROBE_TIME)) {
      SAFE_PRINTF_LOG_WARNING("miss to %d\n", nodes_get_address(
                                iotus_radio_selected_address_type,
                                packet->nextDestinationNode));
      break;
    }

    {
      rtimer_clock_t wt;
#if WITH_PHASE_OPTIMIZATION
      rtimer_clock_t txtime = RTIMER_NOW();
#endif
#if RDC_CONF_HARDWARE_ACK
      int ret = active_radio_driver->transmit(packet);
#else
      active_radio_driver->transmit(packet);
#endif

#if RDC_CONF_HARDWARE_ACK
     /* For radios that block in the transmit routine and detect the
	ACK in hardware */
      if(ret == RADIO_TX_OK) {
        if(!is_broadcast) {
          got_strobe_ack = 1;
#if WITH_PHASE_OPTIMIZATION
          encounter_time = txtime;
#endif
          break;
        }
      } else if (ret == RADIO_TX_NOACK) {
      } else if (ret == RADIO_TX_COLLISION) {
          SAFE_PRINTF_LOG_ERROR("contikimac: collisions while sending\n");
          collisions++;
      }
      wt = RTIMER_NOW();
      while(RTIMER_CLOCK_LT(RTIMER_NOW(), wt + INTER_PACKET_INTERVAL)) { }
#else /* RDC_CONF_HARDWARE_ACK */
     /* Wait for the ACK packet */
      wt = RTIMER_NOW();
      while(RTIMER_CLOCK_LT(RTIMER_NOW(), wt + INTER_PACKET_INTERVAL)) { }

      if(!is_broadcast && (active_radio_driver->receiving_packet() ||
                           active_radio_driver->pending_packet() ||
                           active_radio_driver->channel_clear() == 0)) {
        wt = RTIMER_NOW();
        while(RTIMER_CLOCK_LT(RTIMER_NOW(), wt + AFTER_ACK_DETECTED_WAIT_TIME)) { }

        iotus_packet_t *ack = active_radio_driver->read();
        if(NULL == ack) {
          SAFE_PRINTF_LOG_ERROR("No ack received\n");
          iotus_parameters_radio_events.collisions++;
        } else {
          uint8_t ackLength = packet_get_payload_size(ack);
          uint8_t ackSeqno = pieces_get_data_pointer(ack)[ACK_LEN - 1];
          packet_destroy(ack);
          if(ackLength == ACK_LEN &&
             seqno == ackSeqno) {
            got_strobe_ack = 1;
  #if WITH_PHASE_OPTIMIZATION
            encounter_time = txtime;
  #endif
            break;
          } else {
            SAFE_PRINTF_LOG_ERROR("contikimac: collisions while sending\n");
            collisions++;
            iotus_parameters_radio_events.collisions++;
          }
        }
      }
#endif /* RDC_CONF_HARDWARE_ACK */
    }
  }

  off();

  SAFE_PRINTF_LOG_INFO("send (stbs=%u, len=%u, %s, %s), done\n", strobes,
         packet_get_size(packet),
         got_strobe_ack ? "ack" : "no ack",
         collisions ? "collision" : "no collision");

#if CONTIKIMAC_CONF_COMPOWER
  /* Accumulate the power consumption for the packet transmission. */
  compower_accumulate(&current_packet);

  /* Convert the accumulated power consumption for the transmitted
     packet to packet attributes so that the higher levels can keep
     track of the amount of energy spent on transmitting the
     packet. */
  compower_attrconv(&current_packet);

  /* Clear the accumulated power consumption so that it is ready for
     the next packet. */
  compower_clear(&current_packet);
#endif /* CONTIKIMAC_CONF_COMPOWER */

  contikimac_is_on = contikimac_was_on;
  we_are_sending = 0;

  /* Determine the return value that we will return from the
     function. We must pass this value to the phase module before we
     return from the function.  */
  if(collisions > 0) {
    ret = MAC_TX_COLLISION;
  } else if(!is_broadcast && !got_strobe_ack) {
    ret = MAC_TX_NOACK;
  } else {
    ret = MAC_TX_OK;
  }

#if WITH_PHASE_OPTIMIZATION
  if(is_known_receiver && got_strobe_ack) {
    PRINTF("no miss %d wake-ups %d\n",
	         nodes_get_address(
                iotus_radio_selected_address_type,
                packet->nextDestinationNode),
           strobes);
  }

  if(!is_broadcast) {
    if(collisions == 0 && is_receiver_awake == 0) {
      phase_update(packetbuf_addr(PACKETBUF_ADDR_RECEIVER),
		   encounter_time, ret);
    }
  }
#endif /* WITH_PHASE_OPTIMIZATION */

  return ret;
}
/*---------------------------------------------------------------------------*/
/* Timer callback triggered when receiving a burst, after having
   waited for a next packet for a too long time. Turns the radio off
   and leaves burst reception mode */
static void
recv_burst_off(void *ptr)
{
  off();
  we_are_receiving_burst = 0;
}
/*---------------------------------------------------------------------------*/
static void
input_packet(iotus_packet_t *packet)
{
  static struct ctimer ct;
  int duplicate = 0;

#if CONTIKIMAC_SEND_SW_ACK
  int original_datalen;
  uint8_t *original_dataptr;

  original_datalen = packet_get_payload_size(packet);//packetbuf_datalen();
  original_dataptr = pieces_get_data_pointer(packet);
#endif

  if(!we_are_receiving_burst) {
    off();
  }

  if(packet_get_payload_size(packet) == ACK_LEN) {
    /* Ignore ack packets */
    SAFE_PRINT("ContikiMAC: ignored ack\n");
    return;
  }

  /*  printf("cycle_start 0x%02x 0x%02x\n", cycle_start, cycle_start % CYCLE_TIME);*/

  if(contikimac_framer.parse(packet) >= 0) {
    if(packet_get_payload_size(packet) > 0 &&
       packet_get_size(packet) > 0 &&
       (packet->nextDestinationNode == NODES_SELF ||
        packet_holds_broadcast(packet))) {
      /* This is a regular packet that is destined to us or to the
         broadcast address. */

      /* If FRAME_PENDING is set, we are receiving a packets in a burst */
      we_are_receiving_burst = packet_get_parameter(packet, PACKET_PARAMETERS_PACKET_PENDING);
      if(we_are_receiving_burst) {
        on();
        /* Set a timer to turn the radio off in case we do not receive
     a next packet */
        ctimer_set(&ct, INTER_PACKET_DEADLINE, recv_burst_off, NULL);
      } else {
        off();
        ctimer_stop(&ct);
      }

#if RDC_WITH_DUPLICATE_DETECTION
      /* Check for duplicate packet. */
      //duplicate = mac_sequence_is_duplicate();
      if(packet_get_sequence_number(packet) == nodes_get_last_sequence_number(packet->prevSourceNode)) {
        duplicate = 1;
        /* Drop the packet. */
        PRINTF("contikimac: Drop duplicate\n");
      } else {
        //mac_sequence_register_seqno();
        nodes_register_sequence_number(packet->prevSourceNode, packet_get_sequence_number(packet));
      }
#endif /* RDC_WITH_DUPLICATE_DETECTION */

#if CONTIKIMAC_CONF_COMPOWER
      /* Accumulate the power consumption for the packet reception. */
      compower_accumulate(&current_packet);
      /* Convert the accumulated power consumption for the received
         packet to packet attributes so that the higher levels can
         keep track of the amount of energy spent on receiving the
         packet. */
      compower_attrconv(&current_packet);

      /* Clear the accumulated power consumption so that it is ready
         for the next packet. */
      compower_clear(&current_packet);
#endif /* CONTIKIMAC_CONF_COMPOWER */

      SAFE_PRINTF_LOG_INFO("contikimac: data (%u)\n", packet_get_payload_size(packet));

#if CONTIKIMAC_SEND_SW_ACK
      {
        frame802154_t info154;
        uint8_t iotusAddressType;
        frame802154_parse(original_dataptr, original_datalen, &info154);
        if(info154.fcf.dest_addr_mode == FRAME802154_SHORTADDRMODE) {
          iotusAddressType = IOTUS_ADDRESSES_TYPE_ADDR_SHORT;
        } else {
          iotusAddressType = IOTUS_ADDRESSES_TYPE_ADDR_LONG;
        }
        if(info154.fcf.frame_type == FRAME802154_DATAFRAME &&
            info154.fcf.ack_required != 0 &&
            addresses_compare((uint8_t *)&info154.dest_addr,
                              addresses_self_get_pointer(iotusAddressType),
                              ADDRESSES_GET_TYPE_SIZE(iotusAddressType))) {
            //linkaddr_cmp((linkaddr_t *)&info154.dest_addr,
            //    &linkaddr_node_addr)) {
          uint8_t ackdata[ACK_LEN] = {0, 0, 0};

          we_are_sending = 1;
          ackdata[0] = FRAME802154_ACKFRAME;
          ackdata[1] = 0;
          ackdata[2] = info154.seq;

          iotus_packet_t *ackPkt = packet_create_msg(
                                      3,
                                      ackdata,
                                      IOTUS_PRIORITY_DATA_LINK, 0,
                                      FALSE,
                                      NODES_BROADCAST);

          if(NULL != ackPkt) {
            packet_set_type(ackPkt, IOTUS_PACKET_TYPE_IEEE802154_ACK);
            active_radio_driver->send(ackPkt);
            printf("Ack sent\n");

            /* If packet has no callback function. Destroy it... */
            packet_destroy(ackPkt);
          } else {
            printf("No ack sent\n");
          }
          we_are_sending = 0;
        }
      }
#endif /* CONTIKIMAC_SEND_SW_ACK */

      if(!duplicate) {
        active_routing_protocol->receive(packet);
      }
      return;
    } else {
      PRINTDEBUG("contikimac: data not for us\n");
    }
  } else {
    SAFE_PRINTF_LOG_ERROR("Parse (%u)\n", packet_get_size(packet));
  }
}
/*---------------------------------------------------------------------------*/
static int
turn_on(void)
{
  if(contikimac_is_on == 0) {
    contikimac_is_on = 1;
    contikimac_keep_radio_on = 0;
    rtimer_set(&rt, RTIMER_NOW() + CYCLE_TIME, 1, powercycle_wrapper, NULL);
  }
  return 1;
}
/*---------------------------------------------------------------------------*/
static int
turn_off(int keep_radio_on)
{
  contikimac_is_on = 0;
  contikimac_keep_radio_on = keep_radio_on;
  if(keep_radio_on) {
    radio_is_on = 1;
    return active_radio_driver->on();
  } else {
    radio_is_on = 0;
    return active_radio_driver->off();
  }
}
/*---------------------------------------------------------------------------*/
static void
init(void)
{
  radio_is_on = 0;
  PT_INIT(&pt);

  rtimer_set(&rt, RTIMER_NOW() + CYCLE_TIME, 1, powercycle_wrapper, NULL);

  contikimac_is_on = 1;

#if WITH_PHASE_OPTIMIZATION
  phase_init();
#endif /* WITH_PHASE_OPTIMIZATION */
}
/*---------------------------------------------------------------------------*/
static unsigned short
duty_cycle(void)
{
  return (1ul * CLOCK_SECOND * CYCLE_TIME) / RTIMER_ARCH_SECOND;
}
/*---------------------------------------------------------------------------*/
static void
post_start(void)
{
}
/*---------------------------------------------------------------------------*/
static void
run(void)
{
}
const struct iotus_data_link_protocol_struct contikiMAC_protocol = {
  "ContikiMAC",
  init,
  post_start,
  run,
  NULL,
  send_packet,
  NULL,
  input_packet
};
/*---------------------------------------------------------------------------*/
