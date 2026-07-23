//
// TinyConsole.cc
// Minimal OPEN-R TCP console with application-layer XOR.


#include <string.h>
#include <ctype.h>
#include <OPENR/OSyslog.h>
#include <OPENR/ODebug.h>
#include <EndpointTypes.h>
#include <TCPEndpointMsg.h>
#include "TinyConsole.h"

// ================================================================
//  Constructor
// ================================================================
TinyConsole::TinyConsole()
    : xorTxOffset(0), xorRxOffset(0), pendingClose(false)
{
    conn.state = CONNECTION_CLOSED;
}

// ================================================================
//  OPEN-R lifecycle
// ================================================================

OStatus
TinyConsole::DoInit(const OSystemEvent& event)
{
    OSYSDEBUG(("TinyConsole::DoInit()\n"));
    return oSUCCESS;
}

OStatus
TinyConsole::DoStart(const OSystemEvent& event)
{
    OSYSDEBUG(("TinyConsole::DoStart()\n"));

    ipstackRef = antStackRef("IPStack");

    // --- Allocate shared send buffer (ANT requires shared memory) ---
    antEnvCreateSharedBufferMsg sendBufMsg(CONSOLE_BUFSIZE);
    sendBufMsg.Call(ipstackRef, sizeof(sendBufMsg));
    if (sendBufMsg.error != ANT_SUCCESS) {
        OSYSLOG1((osyslogERROR, "TinyConsole: can't alloc send buffer %d",
                  sendBufMsg.error));
        return oFAIL;
    }
    conn.sendBuffer = sendBufMsg.buffer;
    conn.sendBuffer.Map();
    conn.sendData = (byte*)conn.sendBuffer.GetAddress();

    // --- Allocate shared receive buffer ---
    antEnvCreateSharedBufferMsg recvBufMsg(CONSOLE_BUFSIZE);
    recvBufMsg.Call(ipstackRef, sizeof(recvBufMsg));
    if (recvBufMsg.error != ANT_SUCCESS) {
        OSYSLOG1((osyslogERROR, "TinyConsole: can't alloc recv buffer %d",
                  recvBufMsg.error));
        return oFAIL;
    }
    conn.recvBuffer = recvBufMsg.buffer;
    conn.recvBuffer.Map();
    conn.recvData = (byte*)conn.recvBuffer.GetAddress();

    OSYSPRINT(("TinyConsole: starting on port %d\n", CONSOLE_PORT));
    return Listen();
}

OStatus
TinyConsole::DoStop(const OSystemEvent& event)
{
    OSYSDEBUG(("TinyConsole::DoStop()\n"));
    return oSUCCESS;
}

OStatus
TinyConsole::DoDestroy(const OSystemEvent& event)
{
    return oSUCCESS;
}

// ================================================================
//  ANT Extra-entry callbacks
// ================================================================

void
TinyConsole::ListenCont(ANTENVMSG msg)
{
    TCPEndpointListenMsg* listenMsg =
        (TCPEndpointListenMsg*)antEnvMsg::Receive(msg);

    OSYSDEBUG(("TinyConsole::ListenCont() lAddr=%x lPort=%d fAddr=%x fPort=%d\n",
               listenMsg->lAddress.Address(), listenMsg->lPort,
               listenMsg->fAddress.Address(), listenMsg->fPort));

    if (listenMsg->error != TCP_SUCCESS) {
        OSYSLOG1((osyslogERROR, "TinyConsole::ListenCont FAILED %d",
                  listenMsg->error));
        Close();
        return;
    }

    conn.state   = CONNECTION_CONNECTED;
    pendingClose = false;

    // Reset XOR stream offsets for this new session.
    xorTxOffset = 0;
    xorRxOffset = 0;

    // Send the plaintext banner — NOT XOR'd so client knows when XOR begins.
    const char* banner    = CONSOLE_BANNER;
    int         bannerLen = strlen(banner);
    memcpy(conn.sendData, banner, bannerLen);
    conn.sendSize = bannerLen;

    TCPEndpointSendMsg sendMsg(conn.endpoint,
                               conn.sendData, conn.sendSize);
    sendMsg.continuation = (void*)0;
    sendMsg.Send(ipstackRef, myOID,
                 Extra_Entry[entrySendCont], sizeof(sendMsg));
    conn.state    = CONNECTION_SENDING;
    conn.sendSize = 0;
    // SendCont() will call Receive() once banner delivery is confirmed.
}

void
TinyConsole::SendCont(ANTENVMSG msg)
{
    TCPEndpointSendMsg* sendMsg =
        (TCPEndpointSendMsg*)antEnvMsg::Receive(msg);

    OSYSDEBUG(("TinyConsole::SendCont()\n"));

    if (sendMsg->error != TCP_SUCCESS) {
        OSYSLOG1((osyslogERROR, "TinyConsole::SendCont FAILED %d",
                  sendMsg->error));
        Close();
        return;
    }

    conn.state = CONNECTION_CONNECTED;

    if (pendingClose) {
        pendingClose = false;
        Close();
        return;
    }

    Receive();
}

void
TinyConsole::ReceiveCont(ANTENVMSG msg)
{
    TCPEndpointReceiveMsg* recvMsg =
        (TCPEndpointReceiveMsg*)antEnvMsg::Receive(msg);

    OSYSDEBUG(("TinyConsole::ReceiveCont() n=%d\n", recvMsg->sizeMin));

    if (recvMsg->error == TCP_CONNECTION_CLOSED) {
        OSYSPRINT(("TinyConsole: client disconnected\n"));
        Close();
        return;
    }

    if (recvMsg->error != TCP_SUCCESS) {
        OSYSLOG1((osyslogERROR, "TinyConsole::ReceiveCont FAILED %d",
                  recvMsg->error));
        Close();
        return;
    }

    // Bug 2 fix: restore state to CONNECTED so Send() will accept the call.
    conn.state = CONNECTION_CONNECTED;

    int n = recvMsg->sizeMin;
    conn.recvSize = n;

    // XOR decode incoming data.
    XorBuffer(conn.recvData, n, xorRxOffset);

    // Null-terminate for safe string ops.
    if (n < CONSOLE_BUFSIZE) conn.recvData[n] = '\0';

    // -------------------------------------------------------
    //  Command dispatch.
    //
    //  Bug 1 fix: do NOT allocate cmd[CONSOLE_BUFSIZE] on the
    //  stack — 4096 bytes would overflow the OCF stack budget.
    //  Work directly in recvData which is already in shared
    //  heap memory, stripping CR/LF in place.
    // -------------------------------------------------------
    char* cmd    = (char*)conn.recvData;
    int   cmdLen = n;

    while (cmdLen > 0 &&
           (cmd[cmdLen-1] == '\r' || cmd[cmdLen-1] == '\n'))
        cmd[--cmdLen] = '\0';

    const char* response = "OK\n";

    if (cmdLen == 0) {
        response = "> ";
    } else if (!strncmp(cmd, "PING", 4)) {
        response = "PONG\n";
    } else if (!strncmp(cmd, "QUIT", 4)) {
        response  = "BYE\n";
        pendingClose = true;   // Close() called from SendCont after BYE sent
    }

    // XOR encode the response and send.
    int respLen = strlen(response);
    memcpy(conn.sendData, response, respLen);
    XorBuffer(conn.sendData, respLen, xorTxOffset);
    conn.sendSize = respLen;

    Send(conn.sendData, conn.sendSize);
}

void
TinyConsole::CloseCont(ANTENVMSG msg)
{
    TCPEndpointCloseMsg* closeMsg =
        (TCPEndpointCloseMsg*)antEnvMsg::Receive(msg);

    OSYSDEBUG(("TinyConsole::CloseCont()\n"));

    conn.state = CONNECTION_CLOSED;
    Listen();  // immediately wait for next client
}

// ================================================================
//  Private TCP helpers
// ================================================================

OStatus
TinyConsole::Listen()
{
    OSYSDEBUG(("TinyConsole::Listen()\n"));

    if (conn.state != CONNECTION_CLOSED) return oFAIL;

    antEnvCreateEndpointMsg tcpCreateMsg(EndpointType_TCP,
                                         CONSOLE_BUFSIZE * 2);
    tcpCreateMsg.Call(ipstackRef, sizeof(tcpCreateMsg));
    if (tcpCreateMsg.error != ANT_SUCCESS) {
        OSYSLOG1((osyslogERROR, "TinyConsole::Listen endpoint FAIL %d",
                  tcpCreateMsg.error));
        return oFAIL;
    }
    conn.endpoint = tcpCreateMsg.moduleRef;

    TCPEndpointListenMsg listenMsg(conn.endpoint,
                                   IP_ADDR_ANY, CONSOLE_PORT);
    listenMsg.continuation = (void*)0;
    listenMsg.Send(ipstackRef, myOID,
                   Extra_Entry[entryListenCont], sizeof(listenMsg));

    conn.state = CONNECTION_LISTENING;
    OSYSPRINT(("TinyConsole: listening on port %d\n", CONSOLE_PORT));
    return oSUCCESS;
}

OStatus
TinyConsole::Send(const byte* data, int size)
{
    if (conn.state != CONNECTION_CONNECTED) {
        OSYSLOG1((osyslogERROR, "TinyConsole::Send called in wrong state %d",
                  conn.state));
        return oFAIL;
    }

    TCPEndpointSendMsg sendMsg(conn.endpoint,
                               conn.sendData, size);
    sendMsg.continuation = (void*)0;
    sendMsg.Send(ipstackRef, myOID,
                 Extra_Entry[entrySendCont],
                 sizeof(TCPEndpointSendMsg));

    conn.state    = CONNECTION_SENDING;
    conn.sendSize = 0;
    return oSUCCESS;
}

OStatus
TinyConsole::Receive()
{
    if (conn.state != CONNECTION_CONNECTED) {
        OSYSLOG1((osyslogERROR, "TinyConsole::Receive called in wrong state %d",
                  conn.state));
        return oFAIL;
    }

    TCPEndpointReceiveMsg recvMsg(conn.endpoint,
                                  conn.recvData,
                                  1, CONSOLE_BUFSIZE);
    recvMsg.continuation = (void*)0;
    recvMsg.Send(ipstackRef, myOID,
                 Extra_Entry[entryReceiveCont], sizeof(recvMsg));

    conn.state    = CONNECTION_RECEIVING;
    conn.recvSize = 0;
    return oSUCCESS;
}

OStatus
TinyConsole::Close()
{
    if (conn.state == CONNECTION_CLOSED ||
        conn.state == CONNECTION_CLOSING) return oFAIL;

    TCPEndpointCloseMsg closeMsg(conn.endpoint);
    closeMsg.continuation = (void*)0;
    closeMsg.Send(ipstackRef, myOID,
                  Extra_Entry[entryCloseCont], sizeof(closeMsg));

    conn.state = CONNECTION_CLOSING;
    return oSUCCESS;
}

// ================================================================
//  XOR stream helper
// ================================================================
void
TinyConsole::XorBuffer(byte* buf, int len, int& offset)
{
    for (int i = 0; i < len; ++i) {
        buf[i] ^= XOR_KEY[offset % XOR_KEYLEN];
        ++offset;
    }
}

