PKGDIR	?= .
L4DIR	?= $(PKGDIR)/../..

# the default is to build the listed directories, provided that they
# contain a Makefile. If you need to change this, uncomment the following
# line and adapt it.
# TARGET = include src lib server examples doc

TARGET = keyboard
SRC_CC_keyboard = driver.cc

REQUIRES_LIBS += libsigma0 libio-direct

include $(L4DIR)/mk/prog.mk
