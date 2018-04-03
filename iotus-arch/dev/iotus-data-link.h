/**
 * \defgroup description...
 *
 * This...
 *
 * @{
 */

/*
 * iotus-data-link.h
 *
 *  Created on: Oct 23, 2017
 *      Author: vinicius
 */



/* The following stuff ends the \defgroup block at the beginning of
   the file: */

/** @} */
#ifndef IOTUS_DEV_DATA_LINK_H_
#define IOTUS_DEV_DATA_LINK_H_


#include "packet.h"
#include "platform-conf.h"

extern const struct iotus_radio_driver_struct native_radio_radio_driver;
struct iotus_data_link_protocol_struct {
  void (* start)(void);
  void (* post_start)(void);
  void (* run)(void);
  void (* close)(void);
  void (* send)(iotus_packet_t *packet);
  void (* sent_cb)(iotus_packet_t *packet);
  void (* receive)(iotus_packet_t *packet);
};

#endif /* IOTUS_DEV_DATA_LINK_H_ */

/* The following stuff ends the \defgroup block at the beginning of
   the file: */

/** @} */
