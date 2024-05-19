/*
  THIS FILE MUST NOT BE MODIFIED.

  CS3102 Coursework P2 : Simple, Reliable Transport Protocol (SRTP)
  saleem, 20 Feb 2023

  Test server 1 : server download. 
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
  uid_t port = getuid(); // same as test-client-1.c
  int start_sd, sd;
  uint8_t *data;
  int b;
  uint32_t *v1_p, *v2_p;
  char remote[22];
  int n = 100000; // same as test-client-1.c
  int tx_pkts = 0, tx_fails = 0;

  if (argc != 1) {
    printf("usage: test-server-1\n");
    exit(0);
  }
  printf("test-server-1\n");

  if (port < 1024)
    port += 1024; // hack, avoid reserved ports, should not happen in labs

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
  printf("accept(): %s\n", remote);

  data = calloc(1, SRTP_MAX_DATA_SIZE); /* SRTP_MAX_DATA_SIZE = 1280 */
  v1_p = (uint32_t *) data;
  v2_p = (uint32_t *) &data[SRTP_MAX_DATA_SIZE - sizeof(uint32_t)];

  /* transmit n packets */
  memset(data, 0, SRTP_MAX_DATA_SIZE); // clear
  for (int i = 0; i < n; ++i) {

    int c = i + 1;
    *v1_p = htonl(c); // first word
    *v2_p = htonl(c); // last word
    b = srtp_tx(sd, data, SRTP_MAX_DATA_SIZE);
    if (b < 0) {
      printf("srtp_tx() failed (n=%d)\n", n);
      ++tx_fails;
      continue;      
    }

    printf("srtp_tx(): %d/%d pkts, %d/%d bytes.\n",
           c, n, b, SRTP_MAX_DATA_SIZE);
    ++tx_pkts;
  }


  /*
    finish
  */
  printf(" %d/%d pkts from %s, %d fails.\n", tx_pkts, n, remote, tx_fails);
  srtp_close(sd);
  Srtp_Pcb_report();
  free(data);

  return 0;
}
