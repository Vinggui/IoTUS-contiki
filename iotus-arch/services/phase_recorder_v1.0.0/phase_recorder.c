
/**
 * \defgroup description...
 *
 * This...
 *
 * @{
 */

/*
 * seqnum.c
 *
 *  Created on: Feb 27, 2018
 *      Author: vinicius
 */

#include <stdio.h>
#include <stdlib.h>
#include "iotus-core.h"
#include "iotus-netstack.h"
#include "nodes.h"
#include "phase_recorder.h"
#include "sys/clock.h"
#include "sys/ctimer.h"
#include "sys/rtimer.h"


#define DEBUG IOTUS_DONT_PRINT//IOTUS_PRINT_IMMEDIATELY
#define THIS_LOG_FILE_NAME_DESCRITOR "PhasR"
#include "safe-printer.h"

#if PHASE_CONF_DRIFT_CORRECT
#define PHASE_DRIFT_CORRECT PHASE_CONF_DRIFT_CORRECT
#else
#define PHASE_DRIFT_CORRECT 0
#endif

#define PHASE_DEFER_THRESHOLD 1

#define MAX_NOACKS            16

#define MAX_NOACKS_TIME       CLOCK_SECOND * 30

#define PHASE_QUEUESIZE       8


struct phase_queueitem {
  struct ctimer timer;
  iotus_packet_t *packetPhased;
};

/**
 * This struct is necessary to work with additionalInfo system
 */
typedef struct __attribute__ ((__packed__)) phase_recorder_t {
  rtimer_clock_t ptr;
} phase_recorder_t;

MEMB(phased_packets_memb, struct phase_queueitem, PHASE_QUEUESIZE);

/*---------------------------------------------------------------------------*/
void
phase_recorder_update(const iotus_node_t *neighbor, rtimer_clock_t time,
             int mac_status)
{
  if(neighbor == NULL) {
    return;
  }

  /* If we have an entry for this neighbor already, we renew it. */
  //e = nbr_table_get_from_lladdr(nbr_phase, neighbor);
  phase_recorder_t *phasePointer = pieces_get_additional_info_var(
                                  neighbor->additionalInfoList,
                                  IOTUS_NODES_ADD_INFO_TYPE_WAKEUP_PHASE);

  if(phasePointer != NULL) {
    if(mac_status == MAC_TX_OK) {
#if PHASE_DRIFT_CORRECT
      //e->drift = time - e->time;
#endif
      phasePointer->ptr = time;
      SAFE_PRINTF_LOG_INFO("Saved time again %u\n", *phasePointer);
    }
    /**
     * The ack update and timing update to remove a neighbor is
     * now done by the node service, not here.
     */
    /* If the neighbor didn't reply to us, it may have switched
       phase (rebooted). We try a number of transmissions to it
       before we drop it from the phase list. */
    // if(mac_status == MAC_TX_NOACK) {
    //   SAFE_PRINTF_LOG_ERROR("noacks %d\n", e->noacks);
    //   e->noacks++;
    //   if(e->noacks == 1) {
    //     timer_set(&e->noacks_timer, MAX_NOACKS_TIME);
    //   }
    //   if(e->noacks >= MAX_NOACKS || timer_expired(&e->noacks_timer)) {
    //     PRINTF("drop %d\n", neighbor->u8[0]);
    //     nbr_table_remove(nbr_phase, e);
    //     return;
    //   }
    // } else if(mac_status == MAC_TX_OK) {
    //   e->noacks = 0;
    // }
  } else {
    /* No matching phase was found, so we allocate a new one. */
    if(mac_status == MAC_TX_OK && phasePointer == NULL) {

      phasePointer = pieces_modify_additional_info_var(
                          neighbor->additionalInfoList,
                          IOTUS_NODES_ADD_INFO_TYPE_WAKEUP_PHASE,
                          sizeof(rtimer_clock_t),
                          TRUE);
      if(NULL == phasePointer) {
        SAFE_PRINTF_LOG_ERROR("Set");
        return;
      }

      phasePointer->ptr = time;
      
      SAFE_PRINTF_LOG_INFO("Saved time %u\n", phasePointer->ptr);
      //e = nbr_table_add_lladdr(nbr_phase, neighbor, NBR_TABLE_REASON_MAC, NULL);
    }
  }
}
/*---------------------------------------------------------------------------*/
static void
send_packet(void *ptr)
{
  struct phase_queueitem *p = ptr;
  SAFE_PRINTF_LOG_INFO("sending phased pkt %u\n", packet_get_sequence_number(p->packetPhased));
  
  // packet_continue_deferred_packet(p->packetPhased);
  iotus_netstack_return returnAns;
  returnAns = active_data_link_protocol->send(p->packetPhased);
  packet_confirm_transmission(p->packetPhased, returnAns);

  memb_free(&phased_packets_memb, p);
}

/*---------------------------------------------------------------------------*/
phase_recorder_status_t
phase_recorder_wait(const iotus_node_t *neighbor, rtimer_clock_t cycle_time,
           rtimer_clock_t guard_time, iotus_packet_t *packet)
{
  //  const linkaddr_t *neighbor = packetbuf_addr(PACKETBUF_ADDR_RECEIVER);
  /* We go through the list of phases to find if we have recorded a
     phase for this particular neighbor. If so, we can compute the
     time for the next expected phase and setup a ctimer to switch on
     the radio just before the phase. */
  //e = nbr_table_get_from_lladdr(nbr_phase, neighbor);
  phase_recorder_t *phasePointer = pieces_get_additional_info_var(
                                  neighbor->additionalInfoList,
                                  IOTUS_NODES_ADD_INFO_TYPE_WAKEUP_PHASE);


  if(phasePointer != NULL) {
    SAFE_PRINTF_LOG_INFO("recovered %u\n", phasePointer->ptr);
    rtimer_clock_t wait, now, expected, sync;
    clock_time_t ctimewait;
    
    /* We expect phases to happen every CYCLE_TIME time
       units. The next expected phase is at time e->time +
       CYCLE_TIME. To compute a relative offset, we subtract
       with clock_time(). Because we are only interested in turning
       on the radio within the CYCLE_TIME period, we compute the
       waiting time with modulo CYCLE_TIME. */
    
    /*      printf("neighbor phase 0x%02x (cycle 0x%02x)\n", e->time & (cycle_time - 1),
            cycle_time);*/

    /*      if(e->noacks > 0) {
            printf("additional wait %d\n", additional_wait);
            }*/
    
    now = RTIMER_NOW();

    sync = (phasePointer == NULL) ? now : phasePointer->ptr;

#if PHASE_DRIFT_CORRECT
    {
      int32_t s;
      if(e->drift > cycle_time) {
        s = e->drift % cycle_time / (e->drift / cycle_time);  /* drift per cycle */
        s = s * (now - sync) / cycle_time;                    /* estimated drift to now */
        sync += s;                                            /* add it in */
      }
    }
#endif

    /* Check if cycle_time is a power of two */
    if(!(cycle_time & (cycle_time - 1))) {
      /* Faster if cycle_time is a power of two */
      wait = (rtimer_clock_t)((sync - now) & (cycle_time - 1));
    } else {
      /* Works generally */
      wait = cycle_time - (rtimer_clock_t)((now - sync) % cycle_time);
    }

    if(wait < guard_time) {
      wait += cycle_time;
    }

    ctimewait = (CLOCK_SECOND * (wait - guard_time)) / RTIMER_ARCH_SECOND;

    if(ctimewait > PHASE_DEFER_THRESHOLD) {
      if(!packet_get_parameter(packet, PACKET_PARAMETERS_WAS_DEFFERED)) {
        // This paket was not yet deffered, update data
        struct phase_queueitem *p;
        
        p = memb_alloc(&phased_packets_memb);
        if(p != NULL) {
          p->packetPhased = packet;
          SAFE_PRINTF_LOG_INFO("saved for later!\n");
          ctimer_set(&p->timer, ctimewait, send_packet, p);
          packet_set_parameter(packet,PACKET_PARAMETERS_WAS_DEFFERED);
          packet_set_parameter(packet,PACKET_PARAMETERS_IS_READY_TO_TRANSMIT);
          return PHASE_DEFERRED;
        } else {
          SAFE_PRINTF_LOG_INFO("Error allocating!\n");
          return PHASE_UNKNOWN;
        }
      }
    }

    expected = now + wait - guard_time;
    if(!RTIMER_CLOCK_LT(expected, now)) {
      /* Wait until the receiver is expected to be awake */
      while(RTIMER_CLOCK_LT(RTIMER_NOW(), expected));
    }
    return PHASE_SEND_NOW;
  }
  return PHASE_UNKNOWN;
}

/*---------------------------------------------------------------------*/
/*
 * Default function required from IoTUS, to initialize, run and finish this service
 */
void iotus_signal_handler_phase_recorder(iotus_service_signal signal, void *data)
{
  if(IOTUS_START_SERVICE == signal) {
    SAFE_PRINT("\tService Phase recorder\n");
  }
  /* else if (IOTUS_RUN_SERVICE == signal){
  } else if (IOTUS_END_SERVICE == signal){

  }*/
}