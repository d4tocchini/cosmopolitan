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
#include "libc/alg/alg.h"
#include "libc/mem/internal.h"
#include "libc/mem/mem.h"
#include "libc/runtime/runtime.h"
#include "libc/str/str.h"
#include "libc/sysv/errfuns.h"

#define MAX_VARS 512

static bool once;

static void PutEnvDestroy(void) {
  char **a;
  for (a = environ; *a; ++a) free(*a);
  free(environ);
}

static void PutEnvInit(void) {
  char **pin, **pout;
  pin = environ;
  pout = malloc(sizeof(char *) * MAX_VARS);
  environ = pout;
  while (*pin) *pout++ = strdup(*pin++);
  *pout = NULL;
  atexit(PutEnvDestroy);
}

int PutEnvImpl(char *s, bool overwrite) {
  char *p;
  unsigned i, namelen;
  if (!once) {
    PutEnvInit();
    once = true;
  }
  p = strchr(s, '=');
  if (!p) goto fail;
  namelen = p + 1 - s;
  for (i = 0; environ[i]; ++i) {
    if (!strncmp(environ[i], s, namelen)) {
      if (!overwrite) {
        free(s);
        return 0;
      }
      goto replace;
    }
  }
  if (i + 1 >= MAX_VARS) goto fail;
  environ[i + 1] = NULL;
replace:
  free(environ[i]);
  environ[i] = s;
  return 0;
fail:
  free(s);
  return einval();
}

/**
 * Emplaces environment key=value.
 * @see setenv(), getenv()
 */
int putenv(char *string) {
  return PutEnvImpl(strdup(string), true);
}
