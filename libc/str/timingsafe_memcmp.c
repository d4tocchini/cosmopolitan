/*-*- mode:c;indent-tabs-mode:nil;c-basic-offset:2;tab-width:8;coding:utf-8 -*-│
│vi: set net ft=c ts=2 sts=2 sw=2 fenc=utf-8                                :vi│
╞══════════════════════════════════════════════════════════════════════════════╡
│ Copyright 2014 Google Inc.                                                   │
│                                                                              │
│ Permission to use, copy, modify, and/or distribute this software for         │
│ any purpose with or without fee is hereby granted, provided that the         │
│ above copyright notice and this permission notice appear in all copies.      │
│                                                                              │
│ THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL                │
│ WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED                │
│ WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE             │
│ AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL         │
│ DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR        │
│ PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER               │
│ TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR             │
│ PERFORMANCE OF THIS SOFTWARE.                                                │
╚─────────────────────────────────────────────────────────────────────────────*/
#include "libc/str/str.h"

asm(".ident\t\"\\n\\n\
timingsafe_memcmp (ISC License)\\n\
Copyright 2014 Google Inc.\"");
asm(".include \"libc/disclaimer.inc\"");

/**
 * Lexicographically compares the first len bytes in b1 and b2.
 *
 * Running time is independent of the byte sequences compared, making
 * this safe to use for comparing secret values such as cryptographic
 * MACs. In contrast, memcmp() may short-circuit after finding the first
 * differing byte.
 *
 * @note each byte is interpreted as unsigned char
 */
int timingsafe_memcmp(const void *b1, const void *b2, size_t len) {
  const unsigned char *p1 = b1, *p2 = b2;
  size_t i;
  int res = 0, done = 0;
  for (i = 0; i < len; i++) {
    /* lt is -1 if p1[i] < p2[i]; else 0. */
    int lt = (p1[i] - p2[i]) >> CHAR_BIT;
    /* gt is -1 if p1[i] > p2[i]; else 0. */
    int gt = (p2[i] - p1[i]) >> CHAR_BIT;
    /* cmp is 1 if p1[i] > p2[i]; -1 if p1[i] < p2[i]; else 0. */
    int cmp = lt - gt;
    /* set res = cmp if !done. */
    res |= cmp & ~done;
    /* set done if p1[i] != p2[i]. */
    done |= lt | gt;
  }
  return res;
}
