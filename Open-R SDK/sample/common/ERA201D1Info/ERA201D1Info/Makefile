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
LIBS= \
	-L$(OPENRSDK_ROOT)/OPEN_R/lib \
	-lObjectComm \
	-lOPENR \
	-lInternet \
	-lantMCOOP \
	-lERA201D1
CXXFLAGS= \
	-O2 \
	-g \
	-I. \
	-I$(OPENRSDK_ROOT)/OPEN_R/include/R4000 \
	-I$(OPENRSDK_ROOT)/OPEN_R/include/MCOOP \
	-I$(OPENRSDK_ROOT)/OPEN_R/include

.PHONY: all install clean

all: era201d1Info.bin

%.o: %.cc
	$(CXX) $(CXXFLAGS) -o $@ -c $^

ERA201D1InfoStub.cc: stub.cfg
	$(STUBGEN) stub.cfg

era201d1Info.bin: ERA201D1InfoStub.o ERA201D1Info.o era201d1Info.ocf
	$(MKBIN) $(MKBINFLAGS) -o $@ $^ $(LIBS)
	$(STRIP) $@

install: era201d1Info.bin
	gzip -c era201d1Info.bin > $(INSTALLDIR)/OPEN-R/MW/OBJS/ERA201D1.BIN

clean:
	rm -f *.o *.bin *.elf *.snap.cc
	rm -f ERA201D1InfoStub.h ERA201D1InfoStub.cc def.h entry.h
	rm -f $(INSTALLDIR)/OPEN-R/MW/OBJS/ERA201D1.BIN
