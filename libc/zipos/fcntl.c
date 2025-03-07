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
#include "libc/calls/internal.h"
#include "libc/sysv/consts/f.h"
#include "libc/sysv/consts/fd.h"
#include "libc/sysv/consts/o.h"
#include "libc/sysv/errfuns.h"
#include "libc/zip.h"
#include "libc/zipos/zipos.internal.h"

#define ZIPOS  __zipos_get()
#define HANDLE ((struct ZiposHandle *)(intptr_t)g_fds.p[fd].handle)

int __zipos_fcntl(int fd, int cmd, uintptr_t arg) {
  if (cmd == F_GETFD) {
    ZTRACE("__zipos_fcntl(%`'.*s, %s)",
           ZIP_CFILE_NAMESIZE(ZIPOS->map + HANDLE->cfile),
           ZIP_CFILE_NAME(ZIPOS->map + HANDLE->cfile), "F_GETFD");
    if (g_fds.p[fd].flags & O_CLOEXEC) {
      return FD_CLOEXEC;
    } else {
      return 0;
    }
  } else if (cmd == F_SETFD) {
    ZTRACE("__zipos_fcntl(%`'.*s, %s, 0x%016lx)",
           ZIP_CFILE_NAMESIZE(ZIPOS->map + HANDLE->cfile),
           ZIP_CFILE_NAME(ZIPOS->map + HANDLE->cfile), "F_SETFD", arg);
    if (arg & FD_CLOEXEC) {
      g_fds.p[fd].flags |= O_CLOEXEC;
      return FD_CLOEXEC;
    } else {
      g_fds.p[fd].flags &= ~O_CLOEXEC;
      return 0;
    }
  } else {
    ZTRACE("__zipos_fcntl(%`'.*s, %d, 0x%016lx) → EINVAL",
           ZIP_CFILE_NAMESIZE(ZIPOS->map + HANDLE->cfile),
           ZIP_CFILE_NAME(ZIPOS->map + HANDLE->cfile), cmd, arg);
    return einval();
  }
}
