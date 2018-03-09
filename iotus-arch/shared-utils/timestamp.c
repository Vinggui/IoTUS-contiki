
/**
 * \defgroup decription...
 *
 * This...
 *
 * @{
 */

/*
 * timestamp.c
 *
 *  Created on: Feb 27, 2018
 *      Author: vinicius
 */

#include <stdio.h>
#include <stdlib.h>
#include "iotus-core.h"
#include "platform-conf.h"
#include "clock.h"
#include "timestamp.h"

#define DEBUG 1
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else /* DEBUG */
#define PRINTF(...)
#endif /* DEBUG */

/*---------------------------------------------------------------------*/
/*
 * \brief Get the elapsed time between the provided values and now.
 * \param time    The pointer of the timestamp to be set.
 */
void
timestamp_mark(timestamp *time)
{
  time->fineTime = (clock_time() % CLOCK_CONF_SECOND);
  time->seconds = clock_seconds();
}
/*---------------------------------------------------------------------*/

/*
 * \brief Gets which timestamp is bigger then the other.
 * \param time_1    The pointer of the timestamp 1 to be calculated.
 * \param time_2    The pointer of the timestamp 2 to be calculated.
 * \retval 0           Both inputs 1 and 2 are equal.
 * \retval 1           Inputs 1 > 2.
 * \retval 2           Inputs 1 < 2.
 */
uint8_t
timestamp_greater_then(timestamp *time_1,timestamp *time_2)
{
  if(time_1->seconds == time_2->seconds) {
    if(time_1->fineTime == time_2->fineTime) {
      return 0;
    } else if(time_1->fineTime > time_2->fineTime) {
      return 1;
    } else {
      return 2;
    }
  }

  if(time_1->seconds > time_2->seconds) {
    return 1;
  } else {
    return 2;
  }

}
/*---------------------------------------------------------------------*/
/*
 * \brief Get the time difference between the provided values 1 and 2.
 * \param time_1    The pointer of the timestamp 1 to be calculated.
 * \param time_2    The pointer of the timestamp 2 to be calculated.
 * \return             The elasped time in milliseconds between time 1 and 2 (input has to be 1>2).
 */
unsigned long
timestamp_diference(timestamp *time_1, timestamp *time_2)
{
  /* We have to consider every kind of clock implementation.
   * It means that clock_time() can return epoch values or easily wraped 2 bytes counter
   */
  uint8_t greaterValue = timestamp_greater_then(time_1,time_2);
  unsigned long difference = 0;//This must be zero!
  if(greaterValue == 1) {
    difference = time_1->seconds - time_2->seconds;
    difference *= TIMESTAMP_GRANULARITY;

    if(time_1->fineTime > time_2->fineTime) {
      difference += ((time_1->fineTime - time_2->fineTime)*TIMESTAMP_GRANULARITY)/CLOCK_CONF_SECOND;
    } else {
      difference -= ((time_2->fineTime - time_1->fineTime)*TIMESTAMP_GRANULARITY)/CLOCK_CONF_SECOND;
    }
  } else if(greaterValue == 2) {
    difference = time_2->seconds - time_1->seconds;
    difference *= TIMESTAMP_GRANULARITY;
    
    if(time_2->fineTime > time_1->fineTime) {
      difference += ((time_2->fineTime - time_1->fineTime)*TIMESTAMP_GRANULARITY)/CLOCK_CONF_SECOND;
    } else {
      difference -= ((time_1->fineTime - time_2->fineTime)*TIMESTAMP_GRANULARITY)/CLOCK_CONF_SECOND;
    }
  }
  return difference;
}
/*---------------------------------------------------------------------*/

/*
 * \brief Get the elapsed time between the provided values and now.
 * \param seconds    The pointer of the timestamp.
 * \return           The elasped time in milliseconds until now.
 */
unsigned long
timestamp_elapsed(timestamp *time)
{
  timestamp now;
  timestamp_mark(&now);
  return timestamp_diference(&now,time);
}
/*---------------------------------------------------------------------*/

/* The following stuff ends the \defgroup block at the beginning of
   the file: */

/** @} */