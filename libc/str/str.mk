#-*-mode:makefile-gmake;indent-tabs-mode:t;tab-width:8;coding:utf-8-*-┐
#───vi: set et ft=make ts=8 tw=8 fenc=utf-8 :vi───────────────────────┘

PKGS += LIBC_STR

LIBC_STR_ARTIFACTS += LIBC_STR_A
LIBC_STR = $(LIBC_STR_A_DEPS) $(LIBC_STR_A)
LIBC_STR_A = o/$(MODE)/libc/str/str.a
LIBC_STR_A_FILES := $(wildcard libc/str/*)
LIBC_STR_A_HDRS = $(filter %.h,$(LIBC_STR_A_FILES))
LIBC_STR_A_INCS = $(filter %.inc,$(LIBC_STR_A_FILES))
LIBC_STR_A_SRCS_A = $(filter %.s,$(LIBC_STR_A_FILES))
LIBC_STR_A_SRCS_S = $(filter %.S,$(LIBC_STR_A_FILES))
LIBC_STR_A_SRCS_C = $(filter %.c,$(LIBC_STR_A_FILES))

LIBC_STR_A_SRCS =						\
	$(LIBC_STR_A_SRCS_A)					\
	$(LIBC_STR_A_SRCS_S)					\
	$(LIBC_STR_A_SRCS_C)

LIBC_STR_A_OBJS =						\
	$(LIBC_STR_A_SRCS_A:%.s=o/$(MODE)/%.o)			\
	$(LIBC_STR_A_SRCS_S:%.S=o/$(MODE)/%.o)			\
	$(LIBC_STR_A_SRCS_C:%.c=o/$(MODE)/%.o)

LIBC_STR_A_CHECKS =						\
	$(LIBC_STR_A).pkg					\
	$(LIBC_STR_A_HDRS:%=o/$(MODE)/%.ok)

LIBC_STR_A_DIRECTDEPS =						\
	LIBC_INTRIN						\
	LIBC_STUBS						\
	LIBC_SYSV						\
	LIBC_NEXGEN32E

LIBC_STR_A_DEPS :=						\
	$(call uniq,$(foreach x,$(LIBC_STR_A_DIRECTDEPS),$($(x))))

$(LIBC_STR_A):	libc/str/					\
		$(LIBC_STR_A).pkg				\
		$(LIBC_STR_A_OBJS)

$(LIBC_STR_A).pkg:						\
		$(LIBC_STR_A_OBJS)				\
		$(foreach x,$(LIBC_STR_A_DIRECTDEPS),$($(x)_A).pkg)

o/$(MODE)/libc/str/memmem.o:					\
		OVERRIDE_CPPFLAGS +=				\
			-DSTACK_FRAME_UNLIMITED

o//libc/str/bzero.o:						\
		OVERRIDE_CFLAGS +=				\
			-O2

o/$(MODE)/libc/str/dosdatetimetounix.o:				\
		OVERRIDE_CFLAGS +=				\
			-O3

o/$(MODE)/libc/str/getzipcdir.o					\
o/$(MODE)/libc/str/getzipcdircomment.o				\
o/$(MODE)/libc/str/getzipcdircommentsize.o			\
o/$(MODE)/libc/str/getzipcdiroffset.o				\
o/$(MODE)/libc/str/getzipcdirrecords.o				\
o/$(MODE)/libc/str/getzipcfilecompressedsize.o			\
o/$(MODE)/libc/str/getzipcfilemode.o				\
o/$(MODE)/libc/str/getzipcfileoffset.o				\
o/$(MODE)/libc/str/getzipcfileuncompressedsize.o		\
o/$(MODE)/libc/str/getziplfilecompressedsize.o			\
o/$(MODE)/libc/str/getziplfileuncompressedsize.o		\
o/$(MODE)/libc/str/getzipcfiletimestamps.o:			\
		OVERRIDE_CFLAGS +=				\
			-Os

o/$(MODE)/libc/str/iswpunct.o					\
o/$(MODE)/libc/str/iswupper.o					\
o/$(MODE)/libc/str/iswlower.o:					\
		OVERRIDE_CFLAGS +=				\
			-fno-jump-tables

o/$(MODE)/libc/str/windowsdurationtotimeval.o			\
o/$(MODE)/libc/str/windowsdurationtotimespec.o			\
o/$(MODE)/libc/str/timevaltowindowstime.o			\
o/$(MODE)/libc/str/timespectowindowstime.o			\
o/$(MODE)/libc/str/windowstimetotimeval.o			\
o/$(MODE)/libc/str/windowstimetotimespec.o:			\
		OVERRIDE_CFLAGS +=				\
			-O3

o/$(MODE)/libc/str/hey-gcc.asm					\
o/$(MODE)/libc/str/hey.o:					\
		OVERRIDE_CFLAGS +=				\
			-fsanitize=undefined

LIBC_STR_LIBS = $(foreach x,$(LIBC_STR_ARTIFACTS),$($(x)))
LIBC_STR_SRCS = $(foreach x,$(LIBC_STR_ARTIFACTS),$($(x)_SRCS))
LIBC_STR_HDRS = $(foreach x,$(LIBC_STR_ARTIFACTS),$($(x)_HDRS))
LIBC_STR_INCS = $(foreach x,$(LIBC_STR_ARTIFACTS),$($(x)_INCS))
LIBC_STR_BINS = $(foreach x,$(LIBC_STR_ARTIFACTS),$($(x)_BINS))
LIBC_STR_CHECKS = $(foreach x,$(LIBC_STR_ARTIFACTS),$($(x)_CHECKS))
LIBC_STR_OBJS = $(foreach x,$(LIBC_STR_ARTIFACTS),$($(x)_OBJS))
LIBC_STR_TESTS = $(foreach x,$(LIBC_STR_ARTIFACTS),$($(x)_TESTS))
$(LIBC_STR_OBJS): $(BUILD_FILES) libc/str/str.mk

.PHONY: o/$(MODE)/libc/str
o/$(MODE)/libc/str: $(LIBC_STR_CHECKS)
