#ifndef _ConsoleConfig_h_DEFINED
#define _ConsoleConfig_h_DEFINED

#include <ant.h>

#define CONSOLE_PORT    7777
#define CONSOLE_BUFSIZE 4096

const unsigned char XOR_KEY[] = { 0xA5, 0x3C, 0x7F, 0x11, 0xDE };
const int XOR_KEYLEN = 5;

enum ConnectionState {
    CONNECTION_CLOSED,
    CONNECTION_CONNECTING,
    CONNECTION_CONNECTED,
    CONNECTION_LISTENING,
    CONNECTION_SENDING,
    CONNECTION_RECEIVING,
    CONNECTION_CLOSING
};

struct TCPConnection {
    antModuleRef     endpoint;
    ConnectionState  state;
    antSharedBuffer  sendBuffer;
    byte* sendData;
    int              sendSize;
    antSharedBuffer  recvBuffer;
    byte* recvData;
    int              recvSize;
};

#endif
