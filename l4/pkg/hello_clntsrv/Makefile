PKGDIR	?= .
L4DIR	?= $(PKGDIR)/../..

# the default is to build the listed directories, provided that they
# contain a Makefile. If you need to change this, uncomment the following
# line and adapt it.
# TARGET = include src lib server examples doc

REQUIRES_LIBS += libstdc++ libgfxbitmap

TARGET = hserver hclient1 hclient2
SRC_CC_hserver = server.cc
SRC_CC_hclient1 = client.cc
SRC_CC_hclient2 = client.cc

include $(L4DIR)/mk/prog.mk
