#include <string.h>
#include <OPENR/ODebug.h>
#include "ConsolePI.h"
#include "def.h"
#include "entry.h"

ConsolePI::ConsolePI() {
    conn.state = CONNECTION_CLOSED;
}

OStatus ConsolePI::Initialize(const OID& myoid, const antStackRef& ipstack) {
    myOID = myoid;
    ipstackRef = ipstack;
    
    // Alloc Send Buffer
    antEnvCreateSharedBufferMsg sendBufMsg(CONSOLE_BUFSIZE);
    sendBufMsg.Call(ipstackRef, sizeof(sendBufMsg));
    if (sendBufMsg.error != ANT_SUCCESS) {
        OSYSLOG1((osyslogERROR, "ConsolePI: send buffer alloc error %d", sendBufMsg.error));
        return oFAIL;
    }
    conn.sendBuffer = sendBufMsg.buffer;
    conn.sendBuffer.Map();
    conn.sendData = (byte*)(conn.sendBuffer.GetAddress());

    // Alloc Recv Buffer
    antEnvCreateSharedBufferMsg recvBufMsg(CONSOLE_BUFSIZE);
    recvBufMsg.Call(ipstackRef, sizeof(recvBufMsg));
    if (recvBufMsg.error != ANT_SUCCESS) {
        OSYSLOG1((osyslogERROR, "ConsolePI: recv buffer alloc error %d", recvBufMsg.error));
        return oFAIL;
    }
    conn.recvBuffer = recvBufMsg.buffer;
    conn.recvBuffer.Map();
    conn.recvData = (byte*)(conn.recvBuffer.GetAddress());

    Listen();
    return oSUCCESS;
}

OStatus ConsolePI::Listen() {
    if (conn.state != CONNECTION_CLOSED) return oFAIL;

    antEnvCreateEndpointMsg tcpCreateMsg(EndpointType_TCP, CONSOLE_BUFSIZE * 2);
    tcpCreateMsg.Call(ipstackRef, sizeof(tcpCreateMsg));
    if (tcpCreateMsg.error != ANT_SUCCESS) {
        OSYSLOG1((osyslogERROR, "ConsolePI: endpoint error %d", tcpCreateMsg.error));
        return oFAIL;
    }
    conn.endpoint = tcpCreateMsg.moduleRef;

    TCPEndpointListenMsg listenMsg(conn.endpoint, IP_ADDR_ANY, CONSOLE_PORT);
    listenMsg.continuation = (void*)0;
    listenMsg.Send(ipstackRef, myOID, Extra_Entry[entryListenCont], sizeof(listenMsg));
    
    conn.state = CONNECTION_LISTENING;
    return oSUCCESS;
}

void ConsolePI::ListenCont(TCPEndpointListenMsg* msg) {
    if (msg->error != TCP_SUCCESS) {
        OSYSLOG1((osyslogERROR, "ConsolePI: ListenCont error %d", msg->error));
        Close();
        return;
    }
    
    OSYSPRINT(("ConsolePI: Client connected!\n"));
    conn.state = CONNECTION_CONNECTED;
    
    // Reset crypto offsets for new session
    xorRxOffset = 0;
    xorTxOffset = 0;
    
    Receive();
}

OStatus ConsolePI::Send(const char* data) {
    if (conn.state != CONNECTION_CONNECTED) return oFAIL;

    int len = strlen(data);
    if (len >= CONSOLE_BUFSIZE) len = CONSOLE_BUFSIZE - 1;
    
    memcpy(conn.sendData, data, len);
    ApplyXOR(conn.sendData, len, xorTxOffset); // Encrypt
    
    TCPEndpointSendMsg sendMsg(conn.endpoint, conn.sendData, len);
    sendMsg.continuation = (void*)0;
    sendMsg.Send(ipstackRef, myOID, Extra_Entry[entrySendCont], sizeof(sendMsg));

    conn.state = CONNECTION_SENDING;
    return oSUCCESS;
}

void ConsolePI::SendCont(TCPEndpointSendMsg* msg) {
    if (msg->error != TCP_SUCCESS) {
        OSYSLOG1((osyslogERROR, "ConsolePI: SendCont error %d", msg->error));
        Close();
        return;
    }
    conn.state = CONNECTION_CONNECTED;
    Receive(); // Wait for next command
}

OStatus ConsolePI::Receive() {
    if (conn.state != CONNECTION_CONNECTED && conn.state != CONNECTION_SENDING) return oFAIL;

    TCPEndpointReceiveMsg recvMsg(conn.endpoint, conn.recvData, 1, CONSOLE_BUFSIZE - 1);
    recvMsg.continuation = (void*)0;
    recvMsg.Send(ipstackRef, myOID, Extra_Entry[entryReceiveCont], sizeof(recvMsg));

    conn.state = CONNECTION_RECEIVING;
    return oSUCCESS;
}

void ConsolePI::ReceiveCont(TCPEndpointReceiveMsg* msg) {
    if (msg->error == TCP_CONNECTION_CLOSED) {
        OSYSPRINT(("ConsolePI: Client disconnected.\n"));
        Close();
        return;
    }
    if (msg->error != TCP_SUCCESS) {
        OSYSLOG1((osyslogERROR, "ConsolePI: ReceiveCont error %d", msg->error));
        Close();
        return;
    }

    int len = msg->sizeMin;
    ApplyXOR(conn.recvData, len, xorRxOffset); // Decrypt
    conn.recvData[len] = '\0'; // Safe termination
    
    // Limit print length to avoid OSYSPRINT overflow
    OSYSPRINT(("ConsolePI RX: %.100s\n", (char*)conn.recvData));

    // Simple response logic
    if (strncmp((char*)conn.recvData, "PING", 4) == 0) {
        Send("PONG\n");
    } else if (strncmp((char*)conn.recvData, "QUIT", 4) == 0) {
        Send("BYE\n");
        // Force close after sending BYE (will close cleanly in SendCont->Receive->Close)
    } else {
        Send("OK\n");
    }
}

OStatus ConsolePI::Close() {
    if (conn.state == CONNECTION_CLOSED || conn.state == CONNECTION_CLOSING) return oFAIL;

    TCPEndpointCloseMsg closeMsg(conn.endpoint);
    closeMsg.continuation = (void*)0;
    closeMsg.Send(ipstackRef, myOID, Extra_Entry[entryCloseCont], sizeof(closeMsg));

    conn.state = CONNECTION_CLOSING;
    return oSUCCESS;
}

void ConsolePI::CloseCont(TCPEndpointCloseMsg* msg) {
    conn.state = CONNECTION_CLOSED;
    Listen(); // Restart listening for a new client
}

void ConsolePI::ApplyXOR(byte* buf, int len, int& offset) {
    for (int i = 0; i < len; ++i) {
        buf[i] ^= XOR_KEY[offset % XOR_KEYLEN];
        offset++;
    }
}
