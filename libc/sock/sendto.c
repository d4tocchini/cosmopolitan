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
#include "libc/calls/struct/iovec.h"
#include "libc/dce.h"
#include "libc/intrin/asan.internal.h"
#include "libc/sock/internal.h"
#include "libc/sock/sock.h"
#include "libc/str/str.h"
#include "libc/sysv/errfuns.h"

/**
 * Sends data over network.
 *
 * This function blocks unless MSG_DONTWAIT is passed. In that case, the
 * non-error EWOULDBLOCK might be returned. It basically means we didn't
 * wait around to learn an amount of bytes were written that we know in
 * advance are guaranteed to be atomic.
 *
 * @param fd is the file descriptor returned by socket()
 * @param buf is the data to send, which we'll copy if necessary
 * @param size is the byte-length of buf
 * @param flags MSG_OOB, MSG_DONTROUTE, MSG_PARTIAL, MSG_NOSIGNAL, etc.
 * @param opt_addr is a binary ip:port destination override, which is
 *     mandatory for UDP if connect() wasn't called
 * @param addrsize is the byte-length of addr's true polymorphic form
 * @return number of bytes transmitted, or -1 w/ errno
 * @error EINTR, EHOSTUNREACH, ECONNRESET (UDP ICMP Port Unreachable),
 *     EPIPE (if MSG_NOSIGNAL), EMSGSIZE, ENOTSOCK, EFAULT, etc.
 * @asyncsignalsafe
 */
ssize_t sendto(int fd, const void *buf, size_t size, uint32_t flags,
               const void *opt_addr, uint32_t addrsize) {
  if (IsAsan() && (!__asan_is_valid(buf, size) ||
                   (opt_addr && !__asan_is_valid(opt_addr, addrsize)))) {
    return efault();
  }
  _firewall(opt_addr, addrsize);
  if (!IsWindows()) {
    if (!IsBsd() || !opt_addr) {
      return sys_sendto(fd, buf, size, flags, opt_addr, addrsize);
    } else {
      char addr2[sizeof(
          struct sockaddr_un_bsd)]; /* sockaddr_un_bsd is the largest */
      if (addrsize > sizeof(addr2)) return einval();
      memcpy(&addr2, opt_addr, addrsize);
      sockaddr2bsd(&addr2[0]);
      return sys_sendto(fd, buf, size, flags, &addr2[0], addrsize);
    }
  } else {
    if (__isfdopen(fd)) {
      if (__isfdkind(fd, kFdSocket)) {
        return sys_sendto_nt(&g_fds.p[fd], (struct iovec[]){{buf, size}}, 1,
                             flags, opt_addr, addrsize);
      } else if (__isfdkind(fd, kFdFile)) { /* e.g. socketpair() */
        if (flags) return einval();
        if (opt_addr) return eisconn();
        return sys_write_nt(&g_fds.p[fd], (struct iovec[]){{buf, size}}, 1, -1);
      } else {
        return enotsock();
      }
    } else {
      return ebadf();
    }
  }
}
