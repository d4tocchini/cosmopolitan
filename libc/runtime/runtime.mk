#-*-mode:makefile-gmake;indent-tabs-mode:t;tab-width:8;coding:utf-8-*-┐
#───vi: set et ft=make ts=8 tw=8 fenc=utf-8 :vi───────────────────────┘
#
# SYNOPSIS
#
#   Cosmopolitan Runtime
#
# DESCRIPTION
#
#   This package exports essential routines for userspace process
#   initialization.

PKGS += LIBC_RUNTIME

LIBC_RUNTIME = $(LIBC_RUNTIME_A_DEPS) $(LIBC_RUNTIME_A)
LIBC_RUNTIME_ARTIFACTS += LIBC_RUNTIME_A
LIBC_RUNTIME_A = o/$(MODE)/libc/runtime/runtime.a
LIBC_RUNTIME_A_FILES := $(wildcard libc/runtime/*)
LIBC_RUNTIME_A_HDRS = $(filter %.h,$(LIBC_RUNTIME_A_FILES))
LIBC_RUNTIME_A_SRCS_S = $(filter %.S,$(LIBC_RUNTIME_A_FILES))
LIBC_RUNTIME_A_SRCS_C = $(filter %.c,$(LIBC_RUNTIME_A_FILES))

LIBC_RUNTIME_A_SRCS =					\
	$(LIBC_RUNTIME_A_SRCS_S)			\
	$(LIBC_RUNTIME_A_SRCS_C)

LIBC_RUNTIME_A_OBJS =					\
	$(LIBC_RUNTIME_A_SRCS_S:%.S=o/$(MODE)/%.o)	\
	$(LIBC_RUNTIME_A_SRCS_C:%.c=o/$(MODE)/%.o)

LIBC_RUNTIME_A_CHECKS =					\
	$(LIBC_RUNTIME_A).pkg				\
	$(LIBC_RUNTIME_A_HDRS:%=o/$(MODE)/%.ok)

LIBC_RUNTIME_A_DIRECTDEPS =				\
	LIBC_BITS					\
	LIBC_CALLS					\
	LIBC_ELF					\
	LIBC_FMT					\
	LIBC_INTRIN					\
	LIBC_NEXGEN32E					\
	LIBC_NT_KERNEL32				\
	LIBC_STR					\
	LIBC_STUBS					\
	LIBC_SYSV					\
	LIBC_SYSV_CALLS

LIBC_RUNTIME_A_DEPS :=					\
	$(call uniq,$(foreach x,$(LIBC_RUNTIME_A_DIRECTDEPS),$($(x))))

$(LIBC_RUNTIME_A):					\
		libc/runtime/				\
		$(LIBC_RUNTIME_A).pkg			\
		$(LIBC_RUNTIME_A_OBJS)

$(LIBC_RUNTIME_A).pkg:					\
		$(LIBC_RUNTIME_A_OBJS)			\
		$(foreach x,$(LIBC_RUNTIME_A_DIRECTDEPS),$($(x)_A).pkg)

o/$(MODE)/libc/runtime/abort-nt.o			\
o/$(MODE)/libc/runtime/arememoryintervalsok.o		\
o/$(MODE)/libc/runtime/assertfail.o			\
o/$(MODE)/libc/runtime/directmap.o			\
o/$(MODE)/libc/runtime/directmapnt.o			\
o/$(MODE)/libc/runtime/findmemoryinterval.o		\
o/$(MODE)/libc/runtime/ftrace.greg.o			\
o/$(MODE)/libc/runtime/getdosargv.o			\
o/$(MODE)/libc/runtime/getdosenviron.o			\
o/$(MODE)/libc/runtime/hook.greg.o			\
o/$(MODE)/libc/runtime/isheap.o				\
o/$(MODE)/libc/runtime/memtrack.o			\
o/$(MODE)/libc/runtime/memtracknt.o			\
o/$(MODE)/libc/runtime/mman.greg.o			\
o/$(MODE)/libc/runtime/print.greg.o			\
o/$(MODE)/libc/runtime/stackchkfail.o			\
o/$(MODE)/libc/runtime/stackchkfaillocal.o		\
o/$(MODE)/libc/runtime/winmain.greg.o:			\
		OVERRIDE_CFLAGS +=			\
			$(NO_MAGIC)

o/$(MODE)/libc/runtime/mman.greg.o:			\
		OVERRIDE_CFLAGS +=			\
			-ffreestanding

o/$(MODE)/libc/runtime/ftrace.greg.o:			\
		OVERRIDE_CFLAGS +=			\
			-mgeneral-regs-only

LIBC_RUNTIME_LIBS = $(foreach x,$(LIBC_RUNTIME_ARTIFACTS),$($(x)))
LIBC_RUNTIME_SRCS = $(foreach x,$(LIBC_RUNTIME_ARTIFACTS),$($(x)_SRCS))
LIBC_RUNTIME_HDRS = $(foreach x,$(LIBC_RUNTIME_ARTIFACTS),$($(x)_HDRS))
LIBC_RUNTIME_BINS = $(foreach x,$(LIBC_RUNTIME_ARTIFACTS),$($(x)_BINS))
LIBC_RUNTIME_CHECKS = $(foreach x,$(LIBC_RUNTIME_ARTIFACTS),$($(x)_CHECKS))
LIBC_RUNTIME_OBJS = $(foreach x,$(LIBC_RUNTIME_ARTIFACTS),$($(x)_OBJS))
LIBC_RUNTIME_TESTS = $(foreach x,$(LIBC_RUNTIME_ARTIFACTS),$($(x)_TESTS))
$(LIBC_RUNTIME_OBJS): $(BUILD_FILES) libc/runtime/runtime.mk

.PHONY: o/$(MODE)/libc/runtime
o/$(MODE)/libc/runtime: $(LIBC_RUNTIME_CHECKS)
