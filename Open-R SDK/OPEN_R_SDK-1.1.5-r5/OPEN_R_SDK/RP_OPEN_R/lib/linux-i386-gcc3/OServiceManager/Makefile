#
# Copyright 2002 Sony Corporation 
#

TARGET=oserviceManager
OBJS=\
	OServiceManagerStub.o \
	OServiceManager.o \
	OServiceBindingRegistry.o \
	OConnectedServiceRegistry.o \
	ODesignDB.o \
	OLineReader.o \
	OLineTokenizer.o \
	OServiceNameParser.o \
	OSystemFile.o \
	OSystemFileOnNativeFS.o
CXXFLAGS=
LIBS=../libOPENR.a libOFSusr.a libmcoopz.a

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
		$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -f $(TARGET)
