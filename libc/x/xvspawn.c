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
#include "libc/calls/calls.h"
#include "libc/calls/sigbits.h"
#include "libc/errno.h"
#include "libc/sysv/consts/sig.h"
#include "libc/x/x.h"

/**
 * Spawns process using vfork().
 *
 * @param f is child callback
 * @param r if provided gets accounting data
 * @return wstatus, or -1 w/ errno
 */
int xvspawn(void f(void *), void *ctx, struct rusage *r) {
  int pid, wstatus;
  sigset_t chldmask, savemask;
  struct sigaction saveint, savequit;
  xsigaction(SIGINT, SIG_IGN, 0, 0, &saveint);
  xsigaction(SIGQUIT, SIG_IGN, 0, 0, &savequit);
  sigemptyset(&chldmask);
  sigaddset(&chldmask, SIGCHLD);
  sigprocmask(SIG_BLOCK, &chldmask, &savemask);
  if ((pid = vfork()) != -1) {
    if (!pid) {
      xsigaction(SIGINT, SIG_DFL, 0, 0, 0);
      xsigaction(SIGQUIT, SIG_DFL, 0, 0, 0);
      sigprocmask(SIG_SETMASK, &savemask, 0);
      f(ctx);
      _exit(127);
    }
    while (wait4(pid, &wstatus, 0, r) == -1) {
      if (errno != EINTR) {
        wstatus = -1;
        break;
      }
    }
    sigaction(SIGINT, &saveint, 0);
    sigaction(SIGQUIT, &savequit, 0);
    sigprocmask(SIG_SETMASK, &savemask, 0);
    return wstatus;
  } else {
    return -1;
  }
}
