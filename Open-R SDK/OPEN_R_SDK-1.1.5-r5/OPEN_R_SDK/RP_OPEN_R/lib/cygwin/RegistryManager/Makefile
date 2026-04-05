#
# Copyright 2002 Sony Corporation 
#

TARGET=registryManager.exe
OBJS=RegistryManagerStub.o RegistryManager.o
CXXFLAGS=
LIBS=-L/usr/local/lib ../libOPENR.a -lcygipc

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -f $(TARGET)
