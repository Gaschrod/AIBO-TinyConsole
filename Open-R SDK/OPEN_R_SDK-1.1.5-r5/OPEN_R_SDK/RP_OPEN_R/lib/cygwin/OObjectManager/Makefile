#
# Copyright 2002 Sony Corporation 
#

TARGET=oobjectManager.exe
OBJS=OObjectManagerStub.o OObjectManager.o
CXXFLAGS=
LIBS=-L/usr/local/lib ../libOPENR.a -lcygipc

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -f $(TARGET)
