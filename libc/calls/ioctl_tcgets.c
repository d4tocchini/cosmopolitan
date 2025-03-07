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
#include "libc/calls/internal.h"
#include "libc/calls/ioctl.h"
#include "libc/calls/struct/metatermios.internal.h"
#include "libc/calls/termios.internal.h"
#include "libc/dce.h"
#include "libc/intrin/asan.internal.h"
#include "libc/sysv/consts/termios.h"
#include "libc/sysv/errfuns.h"

int ioctl_tcgets_nt(int, struct termios *) hidden;

static int ioctl_tcgets_sysv(int fd, struct termios *tio) {
  int rc;
  union metatermios t;
  if ((rc = sys_ioctl(fd, TCGETS, &t)) != -1) {
    __termios2linux(tio, &t);
  }
  return rc;
}

/**
 * Returns information about terminal.
 *
 * @see tcgetattr(fd, tio) dispatches here
 * @see ioctl(fd, TCGETS, tio) dispatches here
 * @see ioctl(fd, TIOCGETA, tio) dispatches here
 */
int ioctl_tcgets(int fd, ...) {
  va_list va;
  struct termios *tio;
  va_start(va, fd);
  tio = va_arg(va, struct termios *);
  va_end(va);
  if (fd >= 0) {
    if (!tio) return efault();
    if (IsAsan() && !__asan_is_valid(tio, sizeof(*tio))) return efault();
    if (fd < g_fds.n && g_fds.p[fd].kind == kFdZip) {
      return enotty();
    } else if (!IsWindows()) {
      return ioctl_tcgets_sysv(fd, tio);
    } else {
      return ioctl_tcgets_nt(fd, tio);
    }
  } else {
    return einval();
  }
}
