OPENRSDK_ROOT?=/usr/local/OPEN_R_SDK
INSTALLDIR=../MS
CXX=$(OPENRSDK_ROOT)/bin/mipsel-linux-g++
CC=$(OPENRSDK_ROOT)/bin/mipsel-linux-gcc
STRIP=$(OPENRSDK_ROOT)/bin/mipsel-linux-strip
MKBIN=$(OPENRSDK_ROOT)/OPEN_R/bin/mkbin
STUBGEN=$(OPENRSDK_ROOT)/OPEN_R/bin/stubgen2
MKBINFLAGS=-p $(OPENRSDK_ROOT)
LIBS=-L$(OPENRSDK_ROOT)/OPEN_R/lib -lObjectComm -lOPENR -lInternet -lantMCOOP
CXXFLAGS= \
	-O2 \
	-g \
	-I. \
	-I$(OPENRSDK_ROOT)/OPEN_R/include/R4000 \
	-I$(OPENRSDK_ROOT)/OPEN_R/include/MCOOP \
	-I$(OPENRSDK_ROOT)/OPEN_R/include

CXXFLAGS+= -DOPENR_DEBUG
CFLAGS= -O2 -g -I. -DPOLY1305_16BIT

.PHONY: all install clean

all: tinyConsole.bin

%.o: %.cc
	$(CXX) $(CXXFLAGS) -o $@ -c $^

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $^

TinyConsoleStub.cc: stub.cfg
	$(STUBGEN) stub.cfg

CRYPTO_OBJS = rfc7539.o chacha20poly1305.o poly1305-donna.o chacha_merged.o tweetnacl.o

tinyConsole.bin: TinyConsoleStub.o TinyConsole.o $(CRYPTO_OBJS) tinyConsole.ocf
	$(MKBIN) $(MKBINFLAGS) -o $@ $^ $(LIBS)
	$(STRIP) $@

install: tinyConsole.bin
	gzip -c tinyConsole.bin > $(INSTALLDIR)/OPEN-R/MW/OBJS/TINYCONS.BIN

clean:
	rm -f *.o *.bin *.elf *.snap.cc
	rm -f TinyConsoleStub.h TinyConsoleStub.cc def.h entry.h
	rm -f $(INSTALLDIR)/OPEN-R/MW/OBJS/TINYCONS.BIN

