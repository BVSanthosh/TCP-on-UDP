/*
  THIS FILE MUST NOT BE MODIFIED.

  CS3102 Coursework P2 : Simple, Reliable Transport Protocol (SRTP)
  saleem, 20 Feb 2023

  Test server 3 : client-server exchange.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include "srtp.h"
#include "srtp-common.h"
#include "srtp-pcb.h"


int
main(int argc, char **argv)
{
  uid_t port = getuid(); // same as test-server-3.c
  int start_sd, sd;
  uint8_t *data;
  int c, b;
  int32_t v1, *v1_p, v2, *v2_p;
  char remote[22];
  int n = 100000; // same as test-server-3.c
  int rx_pkts = 0, tx_pkts = 0, rx_fails = 0, tx_fails = 0;

  if (argc != 1) {
    printf("usage: test-server-3\n");
    exit(0);
  }
  printf("test-server-3\n");

  if (port < 1024)
    port += 1024; // hack, avoid reserved ports, should not happen in labs

  data = malloc(SRTP_MAX_DATA_SIZE); /* SRTP_MAX_DATA_SIZE = 1280 */
  v1_p = (int32_t *) data; // first word
  v2_p = (int32_t *) &data[SRTP_MAX_DATA_SIZE - sizeof(int32_t)]; // last word

  srtp_initialise();

  start_sd = srtp_start(port);
  if (start_sd < 0) {
    printf("start() failed\n");
    exit(0);
  }
  printf("start() OK\n");

  sd = srtp_accept(start_sd);
  if (sd < 0) {
    printf("accept() failed\n");
    exit(0);
  }
  Srtp_Pcb_remote(remote);
  printf("accept(): connected to %s\n", remote);

  /*
    tx and rx n packets
  */

  for (int i = 0; i < n; ++i) {

    /*
      tx : transmit a packet
    */

    memset(data, 0, SRTP_MAX_DATA_SIZE); // clear
    c = i + 1;
    *v1_p = htonl(c); // first word
    *v2_p = htonl(c); // last word
    b = srtp_tx(sd, data, SRTP_MAX_DATA_SIZE);
    if (b < 0) {
      printf("srtp_tx() failed (n=%d)\n", n);
      ++tx_fails;
    }

    else {  
      printf("srtp_tx(): %d/%d pkts, %d/%d bytes.\n",
              c, n, b, SRTP_MAX_DATA_SIZE);
      ++tx_pkts;
    }


    /*
      rx : receive a packet
    */
    memset(data, 0, SRTP_MAX_DATA_SIZE); // clear
    b = srtp_rx(sd, data, SRTP_MAX_DATA_SIZE);

    if (b < 0) {
      printf("srtp_rx() failed (n=%d)\n", n);
      ++rx_fails;
    }

    else {
      /* Is size as expected? */
      if (b == SRTP_MAX_DATA_SIZE)
        { v1 = ntohl(*(v1_p)); v2 = ntohl(*(v2_p)); }
      else { v1 = v2 = -1; }

      /* Is content as expected? */
  #define CHECK(v_, c_) (v_ == c_ ? "(good)" : "(bad)")
      c = i + 1;
      printf("srtp_rx(): %d/%d pkts, %d bytes %s, v1=%d %s, v2=%d %s.\n",
              c, n,
              b,  CHECK(b, SRTP_MAX_DATA_SIZE),
              v1, CHECK(v1, c),
              v2, CHECK(v2, c));
      ++rx_pkts;
    }

  }

  /*
    finish
  */
  printf(" tx: %d/%d pkts from %s, %d fails.\n", tx_pkts, n, remote, tx_fails);
  printf(" rx: %d/%d pkts from %s, %d fails.\n", rx_pkts, n, remote, rx_fails);
  srtp_close(sd);
  Srtp_Pcb_report();
  free(data);

  return 0;
}
