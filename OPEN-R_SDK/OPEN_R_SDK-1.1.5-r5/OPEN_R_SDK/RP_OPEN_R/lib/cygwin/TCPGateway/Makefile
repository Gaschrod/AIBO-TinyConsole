#
# Copyright 2002 Sony Corporation 
#

TARGET=tcpGateway.exe
OBJS=TCPGatewayStub.o TCPGateway.o
CXXFLAGS=
LIBS=-L/usr/local/lib ../libObjectComm.a ../libOPENR.a -lcygipc

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -f $(TARGET)
