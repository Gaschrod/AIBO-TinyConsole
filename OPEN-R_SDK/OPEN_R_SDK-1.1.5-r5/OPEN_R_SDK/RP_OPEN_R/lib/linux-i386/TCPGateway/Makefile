#
# Copyright 2002 Sony Corporation 
#

TARGET=tcpGateway
OBJS=TCPGatewayStub.o TCPGateway.o
CXXFLAGS=
LIBS=../libObjectComm.a ../libOPENR.a -lpthread

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -f $(TARGET)
