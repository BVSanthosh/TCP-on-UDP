#ifndef __srtp_packet_h__
#define __srtp_packet_h__

#include <stdint.h>
#include "srtp-common.h"

/* packet type values : bit field, but can be used as required */

#define SRTP_TYPE_req       ((uint8_t) 0x01)
#define SRTP_TYPE_ack       ((uint8_t) 0x02)

#define SRTP_TYPE_open      ((uint8_t) 0x10)
#define SRTP_TYPE_open_req  (SRTP_TYPE_open  | SRTP_TYPE_req)
#define SRTP_TYPE_open_ack  (SRTP_TYPE_open  | SRTP_TYPE_ack)

#define SRTP_TYPE_close     ((uint8_t) 0x20)
#define SRTP_TYPE_close_req (SRTP_TYPE_close | SRTP_TYPE_req)
#define SRTP_TYPE_close_ack (SRTP_TYPE_close | SRTP_TYPE_ack)

#define SRTP_TYPE_data      ((uint8_t) 0x40)
#define SRTP_TYPE_data_req  (SRTP_TYPE_data  | SRTP_TYPE_req)
#define SRTP_TYPE_data_ack  (SRTP_TYPE_data  | SRTP_TYPE_ack)

typedef struct Srtp_Header_s {
  uint8_t type; // use SRTP_TYPE_x values above : sensible to keep this field here
  uint8_t seq_num; //sequence number for ordered delivery and loss detection
  uint8_t ack_num; //acknowledgement number for reliable delivery
} Srtp_Header_t;

#define SRTP_MAX_PAYLOAD_SIZE SRTP_MAX_DATA_SIZE

typedef struct Srtp_Packet_s {
  Srtp_Header_t *header;
  uint16_t       payload_size; /* <= SRTP_MAX_PAYLOAD_SIZE */
  void *payload; /* buffer that is <= SRTP_MAX_PAYLOAD_SIZE */
} Srtp_Packet_t;

#endif /* __srtp_packet_h__ */