#
# Copyright 2002 Sony Corporation 
#

TARGET=registryManager
OBJS=RegistryManagerStub.o RegistryManager.o
CXXFLAGS=
LIBS=../libOPENR.a

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -f $(TARGET)
