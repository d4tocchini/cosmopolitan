/*-*- mode:c;indent-tabs-mode:nil;c-basic-offset:2;tab-width:8;coding:utf-8 -*-│
│vi: set net ft=c ts=2 sts=2 sw=2 fenc=utf-8                                :vi│
╞══════════════════════════════════════════════════════════════════════════════╡
│ This is free and unencumbered software released into the public domain.      │
│                                                                              │
│ Anyone is free to copy, modify, publish, use, compile, sell, or              │
│ distribute this software, either in source code form or as a compiled        │
│ binary, for any purpose, commercial or non-commercial, and by any            │
│ means.                                                                       │
│                                                                              │
│ In jurisdictions that recognize copyright laws, the author or authors        │
│ of this software dedicate any and all copyright interest in the              │
│ software to the public domain. We make this dedication for the benefit       │
│ of the public at large and to the detriment of our heirs and                 │
│ successors. We intend this dedication to be an overt act of                  │
│ relinquishment in perpetuity of all present and future rights to this        │
│ software under copyright law.                                                │
│                                                                              │
│ THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,              │
│ EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF           │
│ MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.       │
│ IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR            │
│ OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,        │
│ ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR        │
│ OTHER DEALINGS IN THE SOFTWARE.                                              │
╚─────────────────────────────────────────────────────────────────────────────*/
#include "libc/bits/safemacros.internal.h"
#include "libc/dns/servicestxt.h"
#include "libc/errno.h"
#include "libc/fmt/conv.h"
#include "libc/macros.internal.h"

/**
 * Opens and searches /etc/services to find port for a given name.
 *
 * @param servname is a NULL-terminated string
 * @param servproto is a NULL-terminated string (eg "tcp", "udp")
 *          (if servproto is an empty string,
 *           if is filled with the first matching
 *           protocol)
 * @param servprotolen the size of servproto
 * @param buf is a buffer to store the official name of the service
 *          (if NULL, the official name is not stored)
 * @param bufsize is the size of buf
 * @param filepath is the location of services file
 *          (if NULL, uses /etc/services)
 * @return -1 on error, or positive port number
 * @note aliases are read from file for comparison, but not returned.
 * @see LookupServicesByPort
 */
int LookupServicesByName(const char *servname, char *servproto,
                         size_t servprotolen, char *buf, size_t bufsize,
                         const char *filepath) {
  FILE *f;
  char *line;
  char pathbuf[PATH_MAX];
  const char *path;
  size_t linesize;
  int found, result;
  char *name, *port, *proto, *alias, *comment, *tok;
  if (!(path = filepath)) {
    path = "/etc/services";
    if (IsWindows()) {
      path =
          firstnonnull(GetNtServicesTxtPath(pathbuf, ARRAYLEN(pathbuf)), path);
    }
  }
  if (servprotolen == 0 || !(f = fopen(path, "r"))) {
    return -1;
  }
  line = NULL;
  linesize = 0;
  found = 0;
  result = -1;
  if (buf && bufsize != 0) strcpy(buf, "");
  while (found == 0 && (getline(&line, &linesize, f)) != -1) {
    if ((comment = strchr(line, '#'))) *comment = '\0';
    name = strtok_r(line, " \t\r\n\v", &tok);
    port = strtok_r(NULL, "/ \t\r\n\v", &tok);
    proto = strtok_r(NULL, " \t\r\n\v", &tok);
    if (name && port && proto) {
      alias = name;
      while (alias && strcasecmp(alias, servname) != 0)
        alias = strtok_r(NULL, " \t\r\n\v", &tok);
      if (alias) /* alias matched with servname */
      {
        if (!servproto[0] || strncasecmp(proto, servproto, servprotolen) == 0) {
          if (!servproto[0] && !memccpy(servproto, proto, '\0', servprotolen)) {
            strcpy(servproto, "");
            break;
          }
          if (buf && bufsize != 0 && !memccpy(buf, name, '\0', bufsize)) {
            strcpy(buf, "");
            break;
          }
          result = atoi(port);
          found = 1;
        }
      }
    }
  }
  free(line);
  if (ferror(f)) {
    errno = ferror(f);
    return -1;
  }
  fclose(f);
  if (!found) return -1;
  return result;
}
