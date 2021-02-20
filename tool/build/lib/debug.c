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
#include "libc/calls/calls.h"
#include "libc/calls/struct/stat.h"
#include "libc/elf/elf.h"
#include "libc/log/check.h"
#include "libc/runtime/gc.h"
#include "libc/sysv/consts/map.h"
#include "libc/sysv/consts/o.h"
#include "libc/sysv/consts/prot.h"
#include "libc/x/x.h"
#include "tool/build/lib/loader.h"

void LoadDebugSymbols(struct Elf *elf) {
  int fd;
  size_t n;
  void *elfmap;
  struct stat st;
  const char *path;
  if (elf->ehdr && GetElfSymbolTable(elf->ehdr, elf->size, &n) && n) return;
  DCHECK_NOTNULL(elf->prog);
  fprintf(stderr, "HI %s\n", elf->prog);
  if ((fd = open(gc(xstrcat(elf->prog, ".dbg")), O_RDONLY)) != -1 ||
      (fd = open(elf->prog, O_RDONLY)) != -1) {
    if (fstat(fd, &st) != -1 &&
        (elfmap = mmap(NULL, st.st_size, PROT_READ, MAP_SHARED, fd, 0)) !=
            MAP_FAILED) {
      elf->ehdr = elfmap;
      elf->size = st.st_size;
    }
    close(fd);
  }
}
