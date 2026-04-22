//
// TinyConsole.cc
// Minimal OPEN-R TCP console with application-layer XOR.
//
// ANT TCP lifecycle:
//  1. DoStart()     -> Listen()         allocate endpoint, post ListenMsg
//  2. ListenCont()  <- scheduler        client connected
//                   -> send BANNER (plain text, no XOR)
//  3. SendCont()    <- scheduler        send complete
//                   -> Receive()        arm for next incoming data
//  4. ReceiveCont() <- scheduler        data arrived
//                   -> XOR decode, dispatch, XOR encode reply, Send()
//  5. SendCont()    <- scheduler        reply sent
//                   -> Receive() again, or Close() if QUIT was received
//  6. CloseCont()   <- scheduler        connection closed
//                   -> Listen() again   wait for next client
//

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
    : xorTxOffset_(0), xorRxOffset_(0), pendingClose_(false)
{
    conn_.state = CONNECTION_CLOSED;
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

    ipstackRef_ = antStackRef("IPStack");

    // --- Allocate shared send buffer (ANT requires shared memory) ---
    antEnvCreateSharedBufferMsg sendBufMsg(CONSOLE_BUFSIZE);
    sendBufMsg.Call(ipstackRef_, sizeof(sendBufMsg));
    if (sendBufMsg.error != ANT_SUCCESS) {
        OSYSLOG1((osyslogERROR, "TinyConsole: can't alloc send buffer %d",
                  sendBufMsg.error));
        return oFAIL;
    }
    conn_.sendBuffer = sendBufMsg.buffer;
    conn_.sendBuffer.Map();
    conn_.sendData = (byte*)conn_.sendBuffer.GetAddress();

    // --- Allocate shared receive buffer ---
    antEnvCreateSharedBufferMsg recvBufMsg(CONSOLE_BUFSIZE);
    recvBufMsg.Call(ipstackRef_, sizeof(recvBufMsg));
    if (recvBufMsg.error != ANT_SUCCESS) {
        OSYSLOG1((osyslogERROR, "TinyConsole: can't alloc recv buffer %d",
                  recvBufMsg.error));
        return oFAIL;
    }
    conn_.recvBuffer = recvBufMsg.buffer;
    conn_.recvBuffer.Map();
    conn_.recvData = (byte*)conn_.recvBuffer.GetAddress();

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

    conn_.state   = CONNECTION_CONNECTED;
    pendingClose_ = false;

    // Reset XOR stream offsets for this new session.
    xorTxOffset_ = 0;
    xorRxOffset_ = 0;

    // Send the plaintext banner — NOT XOR'd so client knows when XOR begins.
    const char* banner    = CONSOLE_BANNER;
    int         bannerLen = strlen(banner);
    memcpy(conn_.sendData, banner, bannerLen);
    conn_.sendSize = bannerLen;

    TCPEndpointSendMsg sendMsg(conn_.endpoint,
                               conn_.sendData, conn_.sendSize);
    sendMsg.continuation = (void*)0;
    sendMsg.Send(ipstackRef_, myOID_,
                 Extra_Entry[entrySendCont], sizeof(sendMsg));
    conn_.state    = CONNECTION_SENDING;
    conn_.sendSize = 0;
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

    conn_.state = CONNECTION_CONNECTED;

    // Bug 3 fix: if QUIT was processed in the last ReceiveCont,
    // close the connection now that BYE has been delivered.
    if (pendingClose_) {
        pendingClose_ = false;
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
    conn_.state = CONNECTION_CONNECTED;

    int n = recvMsg->sizeMin;
    conn_.recvSize = n;

    // XOR decode incoming data.
    XorBuffer(conn_.recvData, n, xorRxOffset_);

    // Null-terminate for safe string ops.
    if (n < CONSOLE_BUFSIZE) conn_.recvData[n] = '\0';

    // -------------------------------------------------------
    //  Command dispatch.
    //
    //  Bug 1 fix: do NOT allocate cmd[CONSOLE_BUFSIZE] on the
    //  stack — 4096 bytes would overflow the OCF stack budget.
    //  Work directly in recvData which is already in shared
    //  heap memory, stripping CR/LF in place.
    // -------------------------------------------------------
    char* cmd    = (char*)conn_.recvData;
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
        pendingClose_ = true;   // Close() called from SendCont after BYE sent
    }

    // XOR encode the response and send.
    int respLen = strlen(response);
    memcpy(conn_.sendData, response, respLen);
    XorBuffer(conn_.sendData, respLen, xorTxOffset_);
    conn_.sendSize = respLen;

    Send(conn_.sendData, conn_.sendSize);
}

void
TinyConsole::CloseCont(ANTENVMSG msg)
{
    TCPEndpointCloseMsg* closeMsg =
        (TCPEndpointCloseMsg*)antEnvMsg::Receive(msg);

    OSYSDEBUG(("TinyConsole::CloseCont()\n"));

    conn_.state = CONNECTION_CLOSED;
    Listen();  // immediately wait for next client
}

// ================================================================
//  Private TCP helpers
// ================================================================

OStatus
TinyConsole::Listen()
{
    OSYSDEBUG(("TinyConsole::Listen()\n"));

    if (conn_.state != CONNECTION_CLOSED) return oFAIL;

    antEnvCreateEndpointMsg tcpCreateMsg(EndpointType_TCP,
                                         CONSOLE_BUFSIZE * 2);
    tcpCreateMsg.Call(ipstackRef_, sizeof(tcpCreateMsg));
    if (tcpCreateMsg.error != ANT_SUCCESS) {
        OSYSLOG1((osyslogERROR, "TinyConsole::Listen endpoint FAIL %d",
                  tcpCreateMsg.error));
        return oFAIL;
    }
    conn_.endpoint = tcpCreateMsg.moduleRef;

    TCPEndpointListenMsg listenMsg(conn_.endpoint,
                                   IP_ADDR_ANY, CONSOLE_PORT);
    listenMsg.continuation = (void*)0;
    listenMsg.Send(ipstackRef_, myOID_,
                   Extra_Entry[entryListenCont], sizeof(listenMsg));

    conn_.state = CONNECTION_LISTENING;
    OSYSPRINT(("TinyConsole: listening on port %d\n", CONSOLE_PORT));
    return oSUCCESS;
}

OStatus
TinyConsole::Send(const byte* data, int size)
{
    if (conn_.state != CONNECTION_CONNECTED) {
        OSYSLOG1((osyslogERROR, "TinyConsole::Send called in wrong state %d",
                  conn_.state));
        return oFAIL;
    }

    TCPEndpointSendMsg sendMsg(conn_.endpoint,
                               conn_.sendData, size);
    sendMsg.continuation = (void*)0;
    sendMsg.Send(ipstackRef_, myOID_,
                 Extra_Entry[entrySendCont],
                 sizeof(TCPEndpointSendMsg));

    conn_.state    = CONNECTION_SENDING;
    conn_.sendSize = 0;
    return oSUCCESS;
}

OStatus
TinyConsole::Receive()
{
    if (conn_.state != CONNECTION_CONNECTED) {
        OSYSLOG1((osyslogERROR, "TinyConsole::Receive called in wrong state %d",
                  conn_.state));
        return oFAIL;
    }

    TCPEndpointReceiveMsg recvMsg(conn_.endpoint,
                                  conn_.recvData,
                                  1, CONSOLE_BUFSIZE);
    recvMsg.continuation = (void*)0;
    recvMsg.Send(ipstackRef_, myOID_,
                 Extra_Entry[entryReceiveCont], sizeof(recvMsg));

    conn_.state    = CONNECTION_RECEIVING;
    conn_.recvSize = 0;
    return oSUCCESS;
}

OStatus
TinyConsole::Close()
{
    if (conn_.state == CONNECTION_CLOSED ||
        conn_.state == CONNECTION_CLOSING) return oFAIL;

    TCPEndpointCloseMsg closeMsg(conn_.endpoint);
    closeMsg.continuation = (void*)0;
    closeMsg.Send(ipstackRef_, myOID_,
                  Extra_Entry[entryCloseCont], sizeof(closeMsg));

    conn_.state = CONNECTION_CLOSING;
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

