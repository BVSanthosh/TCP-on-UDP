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

#include "byteorder64.h"

static uint16_t S_endian_u16_v = 0xaabb;
static uint8_t  *S_endian_u8_p = (uint8_t *) &S_endian_u16_v;

#define IS_BIG_ENDIAN    (S_endian_u8_p[0] == 0xaa)
#define IS_LITTLE_ENDIAN (S_endian_u8_p[0] == 0xbb)

int isBigEndian()    { return IS_BIG_ENDIAN    ? 1 : 0; }
int isLittleEndian() { return IS_LITTLE_ENDIAN ? 1 : 0; }

/*
  The bit-shift operations can all be done in registers, so this should be
  pretty efficient and have low overhead.
*/

#define REVERSE_BYTE_ORDER_64(v64_) ( \
  (v64_ & 0xff00000000000000) >> 56 | \
  (v64_ & 0x00ff000000000000) >> 40 | \
  (v64_ & 0x0000ff0000000000) >> 24 | \
  (v64_ & 0x000000ff00000000) >>  8 | \
  (v64_ & 0x00000000ff000000) <<  8 | \
  (v64_ & 0x0000000000ff0000) << 24 | \
  (v64_ & 0x000000000000ff00) << 40 | \
  (v64_ & 0x00000000000000ff) << 56   \
)

uint64_t
hton64(uint64_t v64) { return IS_BIG_ENDIAN ? v64 : REVERSE_BYTE_ORDER_64(v64); }

uint64_t
ntoh64(uint64_t v64) { return IS_BIG_ENDIAN ? v64 : REVERSE_BYTE_ORDER_64(v64); }
