#ifndef __byteorder64_h__
#define __byteorder64_h__

/*
  Byte order for 64-bit integers in memory.

  Saleem Bhatti <https://saleem.host.cs.st-andrews.ac.uk/>
  Feb 2023 (minimal)
*/

/*
  There does not appear to be a standard endian check mechanism,
  or a standard host-to-network / network-to-host API for dealing
  with 64-bit integer values.

  Linux does have a set of functions in:

    #include <endian.h>
    man 3 endian

  but these are not POSIX compliant.

  So, some examples of how to check/convert 64-bit value are below.

  This has been tested on AMD64, Intel64, and Raspberry-Pi/ARM-Cortex-72,
  all of which are little-endian.

  (I did not have a real big-endian machine to test this with, but
  have checked with ppc64 cross-compile and emulation on linux.)

*/

#include <stdint.h>

int isBigEndian();    /* returns 1 for true, 0 for false */
int isLittleEndian(); /* returns 1 for true, 0 for false */

uint64_t hton64(uint64_t v64); /* host    to network byte order */
uint64_t ntoh64(uint64_t v64); /* network to host    byte order */

#endif /* __byteorder64_h__ */
