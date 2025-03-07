/*-*- mode:c;indent-tabs-mode:nil;c-basic-offset:2;tab-width:8;coding:utf-8 -*-│
│vi: set net ft=c ts=2 sts=2 sw=2 fenc=utf-8                                :vi│
╞══════════════════════════════════════════════════════════════════════════════╡
│ Copyright 2020 Justine Alexandra Roberts Tunney                              │
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
#include "libc/assert.h"
#include "libc/calls/internal.h"
#include "libc/mem/mem.h"
#include "libc/nt/runtime.h"
#include "libc/zip.h"
#include "libc/zipos/zipos.internal.h"

/**
 * Closes compressed object.
 *
 * @param fd is vetted by close()
 */
int __zipos_close(int fd) {
  struct ZiposHandle *h;
  h = (struct ZiposHandle *)(intptr_t)g_fds.p[fd].handle;
  ZTRACE("__zipos_close(%`'.*s)",
         ZIP_CFILE_NAMESIZE(__zipos_get()->map + h->cfile),
         ZIP_CFILE_NAME(__zipos_get()->map + h->cfile));
  if (!IsWindows()) {
    sys_close(fd);
  } else {
    CloseHandle(h->handle);
  }
  if (!__vforked) {
    free(h->freeme);
    free(h);
  }
  return 0;
}
