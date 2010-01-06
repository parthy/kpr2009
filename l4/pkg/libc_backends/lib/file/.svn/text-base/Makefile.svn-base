PKGDIR  ?= ../..
L4DIR   ?= $(PKGDIR)/../..

TARGET         = libc_be_file.a libc_be_file.so
REQUIRES_LIBS  = l4re libsupc++
PC_FILENAME    = libc_be_file
SRC_CC         = file.cc mmap.cc

include $(L4DIR)/mk/lib.mk
