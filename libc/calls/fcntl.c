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
#include "libc/bits/weaken.h"
#include "libc/calls/calls.h"
#include "libc/calls/internal.h"
#include "libc/dce.h"
#include "libc/sysv/errfuns.h"
#include "libc/zipos/zipos.internal.h"

/**
 * Does things with file descriptor, via re-imagined hourglass api, e.g.
 *
 *     CHECK_NE(-1, fcntl(fd, F_SETFD, FD_CLOEXEC));
 *
 * This function implements POSIX Advisory Locks, e.g.
 *
 *     CHECK_NE(-1, fcntl(zfd, F_SETLKW, &(struct flock){F_WRLCK}));
 *     // ...
 *     CHECK_NE(-1, fcntl(zfd, F_SETLK, &(struct flock){F_UNLCK}));
 *
 * Please be warned that locks currently do nothing on Windows since
 * figuring out how to polyfill them correctly is a work in progress.
 *
 * @param cmd can be F_{GET,SET}{FD,FL}, etc.
 * @param arg can be FD_CLOEXEC, etc. depending
 * @return 0 on success, or -1 w/ errno
 * @asyncsignalsafe
 */
int fcntl(int fd, int cmd, ...) {
  va_list va;
  uintptr_t arg;
  va_start(va, cmd);
  arg = va_arg(va, uintptr_t);
  va_end(va);
  if (fd >= 0) {
    if (fd < g_fds.n && g_fds.p[fd].kind == kFdZip) {
      return weaken(__zipos_fcntl)(fd, cmd, arg);
    } else if (!IsWindows()) {
      return sys_fcntl(fd, cmd, arg);
    } else {
      return sys_fcntl_nt(fd, cmd, arg);
    }
  } else {
    return einval();
  }
}
