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

typedef struct timestamp {
  unsigned long seconds;
  clock_time_t fineTime;
} timestamp;

#define TIMESTAMP_GRANULARITY       1000 //Milliseconds

void
timestamp_mark(timestamp *time);

uint8_t
timestamp_greater_then(timestamp *time_1,timestamp *time_2);

unsigned long
timestamp_diference(timestamp *time_1, timestamp *time_2);

unsigned long
timestamp_elapsed(timestamp *time);

#endif /* IOTUS_ARCH_SHARED_UTILS_TIMESTAMP_H_ */

/* The following stuff ends the \defgroup block at the beginning of
   the file: */

/** @} */
