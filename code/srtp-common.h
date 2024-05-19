#ifndef __srtp_common_h__
#define __srtp_common_h__

#include <stdint.h>

/* 1280 to fit comfortably within 1500 MTU. This avoids
   fragmentation/loss issues with UDP, even with a SRTP
   header added. */
#define SRTP_MAX_DATA_SIZE 1280

/*
  Return values from API.
*/
#define SRTP_SUCCESS 0

/* error values below can be used as bit-mask, if needed */

/* SRTP_ERROR: general problem, e.g. a system call failure */
#define SRTP_ERROR (-1)

/* SRTP_ERROR_api: problem with API calls, e.g. API calls in the wrong order */
#define SRTP_ERROR_api (-2) 

/* SRTP_ERROR_fsm: problem with FSM state change, e.g. wrong packet incoming, 
   or API call in wrong order */
#define SRTP_ERROR_fsm (-4) 

/* SRTP_ERROR_closed: connection is closed */
#define SRTP_ERROR_closed (-8)

/* SRTP_ERROR_protocol: problem with protocol exchange, e.g. unexpected packet type */
#define SRTP_ERROR_protocol (-16) 

uint64_t srtp_timestamp(); // in microseconds
#define SRTP_TIMESTAMP_SECONDS(t_)      ((uint64_t) t_ / 1000000)
#define SRTP_TIMESTAMP_MICROSECONDS(t_) ((uint64_t) t_ % 1000000)
#define SRTP_TIMESTAMP_MILLISECONDS(t_) (((uint64_t) t_ % 1000000) / 1000)

/*
  A human-friendly version of t is provided in t_str, using localtime(3).
  
  Arguments:
  : t - a timestamp value as returned by srtp_timestamp().
  : t_str - a string buffer of at least 64 bytes.

  Returns:
  : number of characters in the string (excluding the terminating '\0').
    t_str is updated, upon success, with a string, such as
      "2023/03/05-16:35:36  (1678034136.992466s)"
  : < 0 on error
  
*/
int srtp_time_str(uint64_t t, char *t_str);

#endif /* __srtp_common_h__ */
