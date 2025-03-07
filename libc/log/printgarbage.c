/*-*- mode:c;indent-tabs-mode:nil;c-basic-offset:2;tab-width:8;coding:utf-8 -*-│
│vi: set net ft=c ts=2 sts=2 sw=2 fenc=utf-8                                :vi│
╞══════════════════════════════════════════════════════════════════════════════╡
│ Copyright 2021 Justine Alexandra Roberts Tunney                              │
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
#include "libc/log/log.h"
#include "libc/nexgen32e/gc.internal.h"
#include "libc/stdio/stdio.h"
/* clang-format off */

/**
 * Prints list of deferred operations on shadow stack.
 */
void PrintGarbage(FILE *f) {
  size_t i;
  f = stderr;
  fprintf(f, "\n");
  fprintf(f, "                          SHADOW STACK @ 0x%016lx\n", __builtin_frame_address(0));
  fprintf(f, " garbage entry  parent frame     original ret        callback              arg        \n");
  fprintf(f, "-------------- -------------- ------------------ ------------------ ------------------\n");
  for (i = __garbage.i; i--;) {
    fprintf(f, "0x%012lx 0x%012lx %-18s %-18s 0x%016lx\n", 
            __garbage.p + i,
            __garbage.p[i].frame,
            GetSymbolByAddr(__garbage.p[i].ret-1),
            GetSymbolByAddr(__garbage.p[i].fn),
            __garbage.p[i].arg);
  }
}
