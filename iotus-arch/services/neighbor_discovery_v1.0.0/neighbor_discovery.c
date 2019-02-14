
/**
 * \defgroup description...
 *
 * This...
 *
 * @{
 */

/*
 * null.c
 *
 *  Created on: Feb 27, 2018
 *      Author: vinicius
 */

#include "addresses.h"
#include "contiki.h"
#include "iotus-api.h"
#include "iotus-core.h"
#include "iotus-netstack.h"
#include "layer-packet-manager.h"
#include "lib/mmem.h"
#include "random.h"
#include "timestamp.h"
#include "tree_manager.h"


#define DEBUG IOTUS_PRINT_IMMEDIATELY//IOTUS_DONT_PRINT//
#define THIS_LOG_FILE_NAME_DESCRITOR "NDisc"
#include "safe-printer.h"



#if IOTUS_USING_MALLOC == 0
static struct mmem gPayload;
#endif /* IOTUS_USING_MALLOC == 0 */

uint8_t gMaintenancePeriod = 60;

static struct ctimer sendNDTimer;
static clock_time_t backOffDifference;
static uint8_t private_nd_control[12];


/*---------------------------------------------------------------------------*/
static void
send_cb(iotus_packet_t *packet, iotus_netstack_return returnAns)
{
  SAFE_PRINTF_LOG_INFO("Frame %p processed %u", packet, returnAns);
  // if(returnAns == MAC_TX_OK) {
    packet_destroy(packet);
  // }
}

/*---------------------------------------------------------------------------*/
static void
send_beacon(void *ptr)
{
  static uint8_t selfAddrValue;
  selfAddrValue = addresses_self_get_pointer(IOTUS_ADDRESSES_TYPE_ADDR_SHORT)[0];

  clock_time_t backoff = CLOCK_SECOND*KEEP_ALIVE_INTERVAL - backOffDifference;//ms
  backOffDifference = (CLOCK_SECOND*((random_rand()%BACKOFF_TIME)))/1000;

  backoff += backOffDifference;
  ctimer_set(&sendNDTimer, backoff, send_beacon, NULL);

  printf("nd maint beacon\n");
  iotus_packet_t *packet = iotus_initiate_packet(
                            12,
                            private_nd_control,
                            PACKET_PARAMETERS_WAIT_FOR_ACK,
                            IOTUS_PRIORITY_ROUTING,
                            5000,
                            NODES_BROADCAST,
                            send_cb);

  if(NULL == packet) {
    SAFE_PRINTF_LOG_INFO("Packet failed");
    return;
  }

  SAFE_PRINTF_LOG_INFO("Packet nd %u \n", packet->pktID);
  active_data_link_protocol->send(packet);
}
/*---------------------------------------------------------------------*/
/*
 * Default function required from IoTUS, to initialize, run and finish this service
 */
void iotus_signal_handler_neighbor_discovery(iotus_service_signal signal, void *data)
{
  if(IOTUS_START_SERVICE == signal) {
    SAFE_PRINT("\tService Neighbor D.\n");


    sprintf((char *)private_nd_control, "### tira ###");

    backOffDifference = (CLOCK_SECOND*((random_rand()%BACKOFF_TIME)))/1000;
    clock_time_t backoff = CLOCK_SECOND*KEEP_ALIVE_INTERVAL + backOffDifference;//ms
    ctimer_set(&sendNDTimer, backoff, send_beacon, NULL);

  }
  /* else if (IOTUS_RUN_SERVICE == signal){
  } else if (IOTUS_END_SERVICE == signal){

  }*/
}