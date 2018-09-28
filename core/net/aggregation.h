
/**
 * \addtogroup nullNet
 * @{
 */

#ifndef AGGREGATION_H_
#define AGGREGATION_H_

#include "lib/mmem.h"


struct aggregation_queueitem {
  struct aggregation_queueitem *next;
  struct ctimer timeout;
  mac_callback_t mac_callback;
  struct mmem data;
  linkaddr_t node_addr;
};

void
aggregation_init(void);


uint8_t
create_aggregation_frame(const char *msg, uint8_t size, linkaddr_t *node_addr, mac_callback_t mac_callback, uint16_t timeout);


void
destroy_aggregation_frame(struct aggregation_queueitem *p);

#endif /* AGGREGATION_H_ */

/** @} */
/** @} */
