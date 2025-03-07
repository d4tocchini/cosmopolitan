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
#include "libc/calls/struct/dirent.h"
#include "libc/rand/rand.h"
#include "libc/runtime/gc.internal.h"
#include "libc/runtime/runtime.h"
#include "libc/str/str.h"
#include "libc/testlib/testlib.h"
#include "libc/x/x.h"

STATIC_YOINK("zip_uri_support");

TEST(dirstream, test) {
  DIR *dir;
  struct dirent *ent;
  char *dpath, *file1, *file2;
  dpath = gc(xasprintf("%s%s%lu", kTmpPath, "dirstream", rand64()));
  file1 = gc(xasprintf("%s/%s", dpath, "foo"));
  file2 = gc(xasprintf("%s/%s", dpath, "bar"));
  EXPECT_NE(-1, mkdir(dpath, 0755));
  EXPECT_NE(-1, touch(file1, 0644));
  EXPECT_NE(-1, touch(file2, 0644));
  EXPECT_TRUE(NULL != (dir = opendir(dpath)));
  bool hasfoo = false;
  bool hasbar = false;
  while ((ent = readdir(dir))) {
    if (strcmp(ent->d_name, "foo")) hasfoo = true;
    if (strcmp(ent->d_name, "bar")) hasbar = true;
  }
  EXPECT_TRUE(hasfoo);
  EXPECT_TRUE(hasbar);
  EXPECT_NE(-1, closedir(dir));
  EXPECT_NE(-1, unlink(file2));
  EXPECT_NE(-1, unlink(file1));
  EXPECT_NE(-1, rmdir(dpath));
}

TEST(dirstream, zipTest) {
  bool foundNewYork = false;
  DIR *d;
  struct dirent *e;
  const char *path = "/zip/usr/share/zoneinfo/";
  ASSERT_NE(0, _gc(xiso8601ts(NULL)));
  ASSERT_NE(NULL, (d = opendir(path)));
  while ((e = readdir(d))) {
    foundNewYork |= !strcmp(e->d_name, "New_York");
  }
  closedir(d);
  EXPECT_TRUE(foundNewYork);
}

TEST(rewinddir, test) {
  DIR *dir;
  struct dirent *ent;
  char *dpath, *file1, *file2;
  dpath = gc(xasprintf("%s%s%lu", kTmpPath, "dirstream", rand64()));
  file1 = gc(xasprintf("%s/%s", dpath, "foo"));
  file2 = gc(xasprintf("%s/%s", dpath, "bar"));
  EXPECT_NE(-1, mkdir(dpath, 0755));
  EXPECT_NE(-1, touch(file1, 0644));
  EXPECT_NE(-1, touch(file2, 0644));
  EXPECT_TRUE(NULL != (dir = opendir(dpath)));
  readdir(dir);
  readdir(dir);
  readdir(dir);
  rewinddir(dir);
  bool hasfoo = false;
  bool hasbar = false;
  while ((ent = readdir(dir))) {
    if (strcmp(ent->d_name, "foo")) hasfoo = true;
    if (strcmp(ent->d_name, "bar")) hasbar = true;
  }
  EXPECT_TRUE(hasfoo);
  EXPECT_TRUE(hasbar);
  EXPECT_NE(-1, closedir(dir));
  EXPECT_NE(-1, unlink(file2));
  EXPECT_NE(-1, unlink(file1));
  EXPECT_NE(-1, rmdir(dpath));
}
