/*

Simplified (2-Clause) BSD License
taken from
<http://www.opensource.org/licenses/bsd-license.php>

Copyright (c) 2023, 2022, 2021, 2008 Saleem N. Bhatti <https://saleem.host.cs.st-andrews.ac.uk/>

All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

  - Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.

  - Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in
    the documentation and/or other materials provided with the
    distribution.

  - Neither the name of 'Saleem N. Bhatti' or 'Saleem Bhatti', nor the
    names of any other contributors may be used to endorse or promote
    products derived from this software without specific prior written
    permission.

DISCLAIMER
----------
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

27 November 2008

*/

/******************************************************************************
Some debugging stuff

Saleem Bhatti <http://saleem.host.cs.st-andrews.ac.uk/>

Jan 2023, Jan 2022, Jan 2021, Nov 2008, Nov 2001, Mar 1993, Jan 1991
******************************************************************************/

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>

#include "d_print.h"

void
d_advise(FILE *file, const char *format, ...)
{
  FILE *fp;
  char *fmt;
  va_list args;

  va_start(args, (char *) format);
  fp = file;
  fmt = (char *) format;

  if (fp == (FILE *) 0)
    fp = stderr;

  if (fmt == (char *) 0) {
#if defined(DEBUG)
    fprintf(stderr, "d_advise(): no format string\n");
#endif
    va_end(args);
    return ;
  }

  vfprintf(fp, fmt, args);
  va_end(args);

  return;
}

void
d_error(FILE *file, const char *format, ...)
{
  FILE *fp;
  char *fmt;
  va_list args;

  va_start(args, (char *) format);
  fp = file;
  fmt = (char *) format;

  if (fp == (FILE *) 0)
    fp = stderr;

  if (fmt == (char *) 0) {
#if defined(DEBUG)
    fprintf(stderr, "d_error(): no format string\n");
#endif
    va_end(args);
    return ;
  }

  fprintf(fp, "error %d: %s :", errno, strerror(errno));
  vfprintf(fp, fmt, args);
  va_end(args);

  return;
}


void
d_stderr(const char *format, ...)
{
  char *fmt;
  va_list args;

  va_start(args, (char *) format);
  fmt = (char *) format;

  if (fmt == (char *) 0) {
#if defined(DEBUG)
    fprintf(stderr, "d_stderr(): no format string\n");
#endif
    va_end(args);
    return ;
  }

  d_advise(stderr, fmt, args);
  va_end(args);

  return;
}


void
d_hexdump(FILE *fp,
          const void *ptr_mem,
          const uint32_t length,
          const uint8_t what)
{
  uint8_t *ptr = (uint8_t *) ptr_mem; /* useful for printing! */
  const char spc[] = "  ";
  uint32_t l, t, i;

  t = 0;
  for(l = 0; l < length; ++l) {

    /* memory address at start of line */
    if (l % 16 == 0) {
      t = l;
      if ((what & D_HEXDUMP_addr) == D_HEXDUMP_addr)
        fprintf(fp, "%14lx%s", ((uintptr_t) ptr + (uintptr_t) l), spc);
    }

    /* data */
    if ((what & D_HEXDUMP_data) == D_HEXDUMP_data) {

      /* space out the hexdumped bytes, 2 at a time */
      if (l % 2 == 0) fprintf(fp, " ");

      /* print the value of a byte */
      fprintf(fp, "%02x", (unsigned char) ptr[l]);
    }

    /* print the text version of the bytes and end the line */
    if (l % 16 == 15 || l == length - 1) {

      /* end of hexdump, but not 16 byte aligned */
      if (l == length - 1 && l % 16 != 15 &&
          (what & D_HEXDUMP_data) == D_HEXDUMP_data) {
        int ll;
        int lll = l + 15 - (l % 16);
        for(ll = l; ll < lll; ++ll) {
          fprintf(fp, "  ");
          if (ll % 2 == 0) fprintf(fp, " ");
        }
      }

      /* print text */
      if ((what & D_HEXDUMP_text) == D_HEXDUMP_text) {
        fprintf(fp, spc);
        for(i = t; i <= l; ++i) {
          uint8_t c = (uint8_t) ptr[i];
          fprintf(fp, "%c", (c > 31 && c < 128) ? c : '.');
        }
      }

      /* end the line */
      fprintf(fp, "\n");
    }
  }
}
