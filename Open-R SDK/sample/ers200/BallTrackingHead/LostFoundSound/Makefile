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
LIBS=-L$(OPENRSDK_ROOT)/OPEN_R/lib -lObjectComm -lOPENR
CXXFLAGS= \
	-O2 \
	-g \
	-I. \
	-I$(OPENRSDK_ROOT)/OPEN_R/include/R4000 \
	-I$(OPENRSDK_ROOT)/OPEN_R/include \
	-I../common/include

#
# When OPENR_DEBUG is defined, OSYSDEBUG() is available.
#
#CXXFLAGS+= -DOPENR_DEBUG

.PHONY: all install clean

all: lostFoundSound.bin

%.o: %.cc
	$(CXX) $(CXXFLAGS) -o $@ -c $^

LostFoundSoundStub.cc: stub.cfg
	$(STUBGEN) stub.cfg	

lostFoundSound.bin: LostFoundSoundStub.o LostFoundSound.o WAV.o lostFoundSound.ocf
	$(MKBIN) $(MKBINFLAGS) -o $@ $^ $(LIBS)
	$(STRIP) $@

install: lostFoundSound.bin
	gzip -c lostFoundSound.bin > $(INSTALLDIR)/OPEN-R/MW/OBJS/LFSOUND.BIN

clean:
	rm -f *.o *.bin *.elf *.snap.cc
	rm -f LostFoundSoundStub.h LostFoundSoundStub.cc def.h entry.h
	rm -f $(INSTALLDIR)/OPEN-R/MW/OBJS/LFSOUND.BIN
