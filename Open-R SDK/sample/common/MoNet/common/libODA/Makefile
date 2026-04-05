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
CXX=$(OPENRSDK_ROOT)/bin/mipsel-linux-g++
AR=$(OPENRSDK_ROOT)/bin/mipsel-linux-ar
RANLIB=$(OPENRSDK_ROOT)/bin/mipsel-linux-ranlib
CXXFLAGS= \
	-O2 \
	-g \
	-I. \
	-I$(OPENRSDK_ROOT)/OPEN_R/include/R4000 \
	-I$(OPENRSDK_ROOT)/OPEN_R/include \
	-I../include

#
# When OPENR_DEBUG is defined, OSYSDEBUG() is available.
#
#CXXFLAGS+= -DOPENR_DEBUG

OBJS		= \
	ODA.o \

TARGET=libODA.a

.PHONY: all clean

all: $(TARGET)

%.o: %.cc
	$(CXX) $(CXXFLAGS) -o $@ -c $^

$(TARGET): $(OBJS)
	rm -f $@
	$(AR) r $@ $(OBJS)
	$(RANLIB) $@

clean:
	rm -f $(TARGET) *.o 
