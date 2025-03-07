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
#include "libc/intrin/asan.internal.h"
#include "libc/sysv/consts/at.h"
#include "libc/sysv/errfuns.h"
#include "libc/zipos/zipos.internal.h"

/**
 * Checks if effective user can access path in particular ways.
 *
 * @param dirfd is normally AT_FDCWD but if it's an open directory and
 *     file is a relative path, then file is opened relative to dirfd
 * @param path is a filename or directory
 * @param mode can be R_OK, W_OK, X_OK, F_OK
 * @param flags should be 0
 * @return 0 if ok, or -1 and sets errno
 * @asyncsignalsafe
 */
int faccessat(int dirfd, const char *path, int mode, uint32_t flags) {
  if (IsAsan() && !__asan_is_valid(path, 1)) return efault();
  if (weaken(__zipos_notat) && weaken(__zipos_notat)(dirfd, path) == -1) {
    return -1; /* TODO(jart): implement me */
  }
  if (!IsWindows()) {
    return sys_faccessat(dirfd, path, mode, flags);
  } else {
    return sys_faccessat_nt(dirfd, path, mode, flags);
  }
}
