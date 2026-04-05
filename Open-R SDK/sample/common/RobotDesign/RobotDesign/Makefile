#
# Copyright 2002,2003,2004 Sony Corporation 
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
LIBS=-L$(OPENRSDK_ROOT)/OPEN_R/lib -lObjectComm -lOPENR
CXXFLAGS= \
	-O2 \
	-g \
	-I. \
	-I$(OPENRSDK_ROOT)/OPEN_R/include/R4000 \
	-I$(OPENRSDK_ROOT)/OPEN_R/include \
	-DOPENR_DEBUG

.PHONY: all install clean

all: robotDesign.bin

%.o: %.cc
	$(CXX) $(CXXFLAGS) -o $@ -c $^

RobotDesignStub.cc: stub.cfg
	$(STUBGEN) stub.cfg

robotDesign.bin: RobotDesignStub.o RobotDesign.o robotDesign.ocf
	$(MKBIN) $(MKBINFLAGS) -o $@ $^ $(LIBS)
	$(STRIP) $@

install: robotDesign.bin
	gzip -c robotDesign.bin > $(INSTALLDIR)/OPEN-R/MW/OBJS/ERS-210.BIN
	gzip -c robotDesign.bin > $(INSTALLDIR)/OPEN-R/MW/OBJS/ERS-220.BIN
	gzip -c robotDesign.bin > $(INSTALLDIR)/OPEN-R/MW/OBJS/ERS-7.BIN

clean:
	rm -f *.o *.bin *.elf *.snap.cc
	rm -f RobotDesignStub.h RobotDesignStub.cc def.h entry.h
	rm -f $(INSTALLDIR)/OPEN-R/MW/OBJS/ERS-210.BIN
	rm -f $(INSTALLDIR)/OPEN-R/MW/OBJS/ERS-220.BIN
	rm -f $(INSTALLDIR)/OPEN-R/MW/OBJS/ERS-7.BIN
