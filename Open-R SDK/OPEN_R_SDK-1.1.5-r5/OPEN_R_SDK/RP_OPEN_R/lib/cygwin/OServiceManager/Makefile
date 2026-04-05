#
# Copyright 2002 Sony Corporation 
#

TARGET=oserviceManager.exe
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
LIBS=-L/usr/local/lib ../libOPENR.a libOFSusr.a libmcoopz.a -lcygipc

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
		$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -f $(TARGET)
