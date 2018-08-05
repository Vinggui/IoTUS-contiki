/**
 * \defgroup decription...
 *
 * This...
 *
 * @{
 */

/*
 * timestamp.h
 *
 *  Created on: Nov 15, 2017
 *      Author: vinicius
 */

#ifndef IOTUS_ARCH_SHARED_UTILS_TIMESTAMP_H_
#define IOTUS_ARCH_SHARED_UTILS_TIMESTAMP_H_

#include "clock.h"


typedef struct __attribute__ ((__packed__)) timestamp {
  unsigned long seconds;
  clock_time_t fineTime;
} timestamp_t;

#define TIMESTAMP_GRANULARITY       1000U //Milliseconds

void
timestamp_mark(timestamp_t *time, int16_t delta);

uint8_t
timestamp_greater_then(timestamp_t *time_1,timestamp_t *time_2);

uint16_t
timestamp_diference(timestamp_t *time_1, timestamp_t *time_2);

uint16_t
timestamp_elapsed(timestamp_t *time);

uint16_t
timestamp_remainder(timestamp_t *time);


#endif /* IOTUS_ARCH_SHARED_UTILS_TIMESTAMP_H_ */

/* The following stuff ends the \defgroup block at the beginning of
   the file: */

/** @} */
