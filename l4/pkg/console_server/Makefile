PKGDIR	?= .
L4DIR	?= $(PKGDIR)/../..

# the default is to build the listed directories, provided that they
# contain a Makefile. If you need to change this, uncomment the following
# line and adapt it.
# TARGET = include src lib server examples doc

L4_MULTITHREADED=y
REQUIRES_LIBS += libstdc++ libgfxbitmap

TARGET = hserver
SRC_CC_hserver = server.cc

include $(L4DIR)/mk/prog.mk
