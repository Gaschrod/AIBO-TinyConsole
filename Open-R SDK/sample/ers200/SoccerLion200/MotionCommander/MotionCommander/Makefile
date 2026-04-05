#
# Copyright 2002,2003 Sony Corporation 
#
# Permission to use, copy, modify, and redistribute this software for
# non-commercial use is hereby granted.
#
# This software is provided "as is" without warranty of any kind,
# either expressed or implied, including but not limited to the
# implied warranties of fitness for a particular purpose.
#

OPENRSDK_ROOT?=/usr/local/OPEN_R_SDK
INSTALLDIR=../MS
CXX=$(OPENRSDK_ROOT)/bin/mipsel-linux-g++
STRIP=$(OPENRSDK_ROOT)/bin/mipsel-linux-strip
MKBIN=$(OPENRSDK_ROOT)/OPEN_R/bin/mkbin
STUBGEN=$(OPENRSDK_ROOT)/OPEN_R/bin/stubgen2
MKBINFLAGS=-p $(OPENRSDK_ROOT)
OMWARES_DIR=../../OMWares
OMWARES_OBJ_DIR=$(OMWARES_DIR)/MS/nomemprot/OPEN-R/MW/OBJS
LIBS=-L$(OMWARES_DIR)/lib -lOMWares -lObjectComm -lOPENR
CXXFLAGS= \
	-O2 \
	-g \
	-I. \
	-I$(OMWARES_DIR)/include \
	-I$(OPENRSDK_ROOT)/OPEN_R/include/R4000 \
	-I$(OPENRSDK_ROOT)/OPEN_R/include

.PHONY: all install clean

all: motionCommander.bin

OBJS = \
	MotionCommanderStub.o \
	MotionCommander.o

%.o: %.cc
	$(CXX) $(CXXFLAGS) -o $@ -c $^

MotionCommanderStub.cc: stub.cfg
	$(STUBGEN) $^

motionCommander.bin: $(OBJS) motionCommander.ocf
	$(MKBIN) $(MKBINFLAGS) -o $@ $^ $(LIBS)
	$(STRIP) $@

install: motionCommander.bin
	cp $(OMWARES_OBJ_DIR)/*.BIN $(INSTALLDIR)/OPEN-R/MW/OBJS
	gzip -c $^ > $(INSTALLDIR)/OPEN-R/MW/OBJS/MTNCMDER.BIN

clean:
	rm -f *.o *.bin *.elf *.snap.cc
	rm -f MotionCommanderStub.h MotionCommanderStub.cc def.h entry.h
	rm -f $(INSTALLDIR)/OPEN-R/MW/OBJS/*.BIN
