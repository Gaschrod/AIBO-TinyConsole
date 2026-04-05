#
# Copyright 2002 Sony Corporation 
#

TARGET=oobjectManager
OBJS=OObjectManagerStub.o OObjectManager.o
CXXFLAGS=
LIBS=../libOPENR.a

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -f $(TARGET)
