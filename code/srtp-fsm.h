#ifndef __srtp_fsm_h__
#define __srtp_fsm_h__

/* states */
#define SRTP_STATE_closed    0  // connection closed
#define SRTP_STATE_listen    1  // server : listening for incoming connections
#define SRTP_STATE_open_req  2  // client : sent request to start connection
#define SRTP_STATE_open_ack  3  // server : sent response to confirm start of connection
#define SRTP_STATE_connected 4  // connected : connection established
#define SRTP_STATE_close_req 5  // sent request to close connection
#define SRTP_STATE_close_ack 6  // sent response to confirm connection is closed

#endif /* __srtp_fsm_h__ */