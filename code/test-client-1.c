/*
  THIS FILE MUST NOT BE MODIFIED.

  CS3102 Coursework P2 : Simple, Reliable Transport Protocol (SRTP)
  saleem, 20 Feb 2023

  Test client 1 : client download.  
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
main(int argc, char *argv[])
{
  uid_t port = getuid(); // same as test-server-1.c
  int sd;
  uint8_t *data;
  int b;
  int32_t v1, *v1_p, v2, *v2_p;
  char remote[22];
  int n = 100000; // same as test-server-1.c
  int rx_pkts = 0, rx_fails = 0;

  if (argc != 2) {
    printf("usage: test-client-1 <fqdn>\n");
    exit(0);
  }
  printf("test-client-1\n");

  if (port < 1024)
    port += 1024; // hack, avoid reserved ports, should not happen in labs

  srtp_initialise();

  sd = srtp_open(argv[1], port);
  if (sd < 0) {
    printf("srtp_open() failed for %s\n", argv[1]);
    exit(0);
  }
  Srtp_Pcb_remote(remote);
  printf("srtp_open(): %s\n", remote);

  data = malloc(SRTP_MAX_DATA_SIZE); /* SRTP_MAX_DATA_SIZE = 1280 */
  v1_p = (int32_t *) data; // first word
  v2_p = (int32_t *) &data[SRTP_MAX_DATA_SIZE - sizeof(int32_t)]; // last word

  /* receive n packets */
  for (int i = 0; i < n; ++i) {

    memset(data, 0, SRTP_MAX_DATA_SIZE); // clear
    b = srtp_rx(sd, data, SRTP_MAX_DATA_SIZE);

    if (b < 0) {
      printf("srtp_rx() failed (n=%d)\n", n);
      ++rx_fails;
      continue;
    }

    /* Is size as expected? */
    if (b == SRTP_MAX_DATA_SIZE)
      { v1 = ntohl(*(v1_p)); v2 = ntohl(*(v2_p)); }
    else { v1 = v2 = -1; }

    /* Is content as expected? */
#define CHECK(v_, c_) (v_ == c_ ? "(good)" : "(bad)")
    int c = i + 1;
    printf("srtp_rx(): %d/%d pkts, %d bytes %s, v1=%d %s, v2=%d %s.\n",
            c, n,
            b, CHECK(b, SRTP_MAX_DATA_SIZE),
            v1, CHECK(v1, c),
            v2, CHECK(v2, c));
    ++rx_pkts;

  } // for (int i = 0; i < n; ++i)
 
  /*
    finish
  */
  printf(" %d/%d pkts from %s, %d fails.\n", rx_pkts, n, remote, rx_fails);
  srtp_close(sd);
  Srtp_Pcb_report();
  free(data);

  return 0;
}
