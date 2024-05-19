#include <inttypes.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>

#include "srtp-common.h"

/* generate timestamp in microseconds, us */

uint64_t
srtp_timestamp()
{
  struct timeval tv;

  if (gettimeofday(&tv, (struct timezone *) 0) != 0) {
    perror("timestamp() : gettimeofday");
    return 0;
  }

  return ((uint64_t) tv.tv_sec * 1000000) + ((uint64_t) tv.tv_usec);
}


int
srtp_time_str(uint64_t t, char *t_str)
{
  if (t_str == (char *) 0)
    return -1;

#define TIME_STR_SIZE 22  // should be enough!
#define SRTP_TIME_STR_SIZE 64  // should be enough!
#define TIME_FORMAT "%Y-%m-%d_%H:%M:%S"

  char time_str[TIME_STR_SIZE];
  time_t t_s = (time_t) SRTP_TIMESTAMP_SECONDS(t);
  if (strftime(time_str, TIME_STR_SIZE, TIME_FORMAT, localtime(&t_s)) == 0)
    return -1;

  int s = snprintf(t_str, SRTP_TIME_STR_SIZE, "%s (%"PRIu64".%06"PRIu64"s)",
                   time_str, SRTP_TIMESTAMP_SECONDS(t), SRTP_TIMESTAMP_MICROSECONDS(t));
  return s;
}
