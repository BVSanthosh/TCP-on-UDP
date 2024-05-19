#ifndef __d_print_h__
#define __d_print_h__

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

27 Nov 2008

*/

/******************************************************************************
Some debugging stuff

Saleem Bhatti <http://saleem.host.cs.st-andrews.ac.uk/>

Jan 2023, Jan 2022, Jan 2021, Nov 2008, Nov 2001, Mar 1993, Jan 1991
******************************************************************************/

#include <stdio.h>
#include <stdint.h>

void d_advise(FILE *fp, const char *format, ...);
void d_error(FILE *fp, const char *format, ...);
void d_stderr(const char *format, ...); /* == d_advise(stderr, ...) */

#define D_HEXDUMP_addr ((uint8_t) 0x01)
#define D_HEXDUMP_data ((uint8_t) 0x02)
#define D_HEXDUMP_text ((uint8_t) 0x04)
#define D_HEXDUMP_all  ((uint8_t) 0xff)

void d_hexdump(FILE *fp,
               const void *ptr,
               const uint32_t length,
               const uint8_t what);

/* Handy macros */
#define D_ERROR(s_) \
    d_error(stdout, (char *) (s_) == (char *) 0 ? "" : (s_))

#define D_TRACER(s_) \
    d_stderr("file=%s line=%d %s\n", __FILE__, __LINE__, \
             (char *) (s_) == (char *) 0 ? "" : (s_))

#define D_HEXDUMP(p_, l_) \
    d_hexdump(stdout, (void *) p_, l_, D_HEXDUMP_all)

#endif /* __d_print_h__ */
