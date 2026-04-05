//
// Copyright 2002,2003 Sony Corporation 
//
// Permission to use, copy, modify, and redistribute this software for
// non-commercial use is hereby granted.
//
// This software is provided "as is" without warranty of any kind,
// either expressed or implied, including but not limited to the
// implied warranties of fitness for a particular purpose.
//

#include <string.h>
#include <OPENR/OSyslog.h>
#include <OPENR/OPENRAPI.h>
#include <ant.h>
#include <EndpointTypes.h>
#include <TCPEndpointMsg.h>
#include "EchoServer.h"
#include "entry.h"

EchoServer::EchoServer ()
{
}

OStatus
EchoServer::DoInit(const OSystemEvent& event)
{
    OSYSDEBUG(("EchoServer::DoInit()\n"));
    return oSUCCESS;
}

OStatus
EchoServer::DoStart(const OSystemEvent& event)
{
    OSYSDEBUG(("EchoServer::DoStart()\n"));

    ipstackRef = antStackRef("IPStack");

    for (int index = 0; index < ECHOSERVER_CONNECTION_MAX; index++) {
        OStatus result = InitTCPConnection(index);
        if (result != oSUCCESS) return oFAIL;
    }

    Listen(0);

    return oSUCCESS;
}    

OStatus
EchoServer::DoStop(const OSystemEvent& event)
{
    OSYSDEBUG(("EchoServer::DoStop()\n"));
    return oSUCCESS;
}

OStatus
EchoServer::DoDestroy(const OSystemEvent& event)
{
    return oSUCCESS;
}

OStatus
EchoServer::Listen(int index)
{
    OSYSDEBUG(("EchoServer::Listen()\n"));

    if (connection[index].state != CONNECTION_CLOSED) return oFAIL;

    //
    // Create endpoint
    //
    antEnvCreateEndpointMsg tcpCreateMsg(EndpointType_TCP,
                                         ECHOSERVER_BUFFER_SIZE * 2);
    tcpCreateMsg.Call(ipstackRef, sizeof(tcpCreateMsg));
    if (tcpCreateMsg.error != ANT_SUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s[%d] antError %d",
                  "EchoServer::Listen()",
                  "Can't create endpoint",
                  index, tcpCreateMsg.error));
        return oFAIL;
    }
    connection[index].endpoint = tcpCreateMsg.moduleRef;

    //
    // Listen
    //
    TCPEndpointListenMsg listenMsg(connection[index].endpoint,
                                   IP_ADDR_ANY, ECHOSERVER_PORT);
    listenMsg.continuation = (void*)index;

    listenMsg.Send(ipstackRef, myOID_,
                   Extra_Entry[entryListenCont], sizeof(listenMsg));
    
    connection[index].state = CONNECTION_LISTENING;

    return oSUCCESS;
}

void
EchoServer::ListenCont(ANTENVMSG msg)
{
    OSYSDEBUG(("EchoServer::ListenCont()\n"));

    TCPEndpointListenMsg* listenMsg
        = (TCPEndpointListenMsg*)antEnvMsg::Receive(msg);
    int index = (int)listenMsg->continuation;

    if (listenMsg->error != TCP_SUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s %d",
                  "EchoServer::ListenCont()",
                  "FAILED. listenMsg->error", listenMsg->error));
        Close(index);
        return;
    }

    connection[index].state = CONNECTION_CONNECTED;

    SetReceiveData(index);
    Receive(index);
}

OStatus
EchoServer::Send(int index)
{
    OSYSDEBUG(("EchoServer::Send()\n"));

    if (connection[index].sendSize == 0 ||
        connection[index].state != CONNECTION_CONNECTED) return oFAIL;

    TCPEndpointSendMsg sendMsg(connection[index].endpoint,
                               connection[index].sendData,
                               connection[index].sendSize);
    sendMsg.continuation = (void*)index;

    sendMsg.Send(ipstackRef, myOID_,
                 Extra_Entry[entrySendCont],
                 sizeof(TCPEndpointSendMsg));

    connection[index].state = CONNECTION_SENDING;
    connection[index].sendSize = 0;
    return oSUCCESS;
}

void
EchoServer::SendCont(ANTENVMSG msg)
{
    OSYSDEBUG(("EchoServer::SendCont()\n"));

    TCPEndpointSendMsg* sendMsg = (TCPEndpointSendMsg*)antEnvMsg::Receive(msg);
    int index = (int)(sendMsg->continuation);

    if (sendMsg->error != TCP_SUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s %d",
                  "EchoServer::SendCont()",
                  "FAILED. sendMsg->error", sendMsg->error));
        Close(index);
        return;
    }

    OSYSPRINT(("sendData : %s", connection[index].sendData));
    
    connection[index].state = CONNECTION_CONNECTED;

    SetReceiveData(index);
    Receive(index);
}

OStatus
EchoServer::Receive(int index)
{
    OSYSDEBUG(("EchoServer::Receive()\n"));

    if (connection[index].state != CONNECTION_CONNECTED &&
        connection[index].state != CONNECTION_SENDING) return oFAIL;

    TCPEndpointReceiveMsg receiveMsg(connection[index].endpoint,
                                     connection[index].recvData,
                                     1, connection[index].recvSize);
    receiveMsg.continuation = (void*)index;

    receiveMsg.Send(ipstackRef, myOID_,
                    Extra_Entry[entryReceiveCont], sizeof(receiveMsg));

    return oSUCCESS;
}

void
EchoServer::ReceiveCont(ANTENVMSG msg)
{
    OSYSDEBUG(("EchoServer::ReceiveCont()\n"));

    TCPEndpointReceiveMsg* receiveMsg
        = (TCPEndpointReceiveMsg*)antEnvMsg::Receive(msg);
    int index = (int)(receiveMsg->continuation);

    if (receiveMsg->error != TCP_SUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s %d",
                  "EchoServer::ReceiveCont()",
                  "FAILED. receiveMsg->error", receiveMsg->error));
        Close(index);
        return;
    }

    OSYSPRINT(("recvData : %s", connection[index].recvData));
               
    connection[index].sendSize = receiveMsg->sizeMin;
    SetSendData(index);
    Send(index);
}

OStatus
EchoServer::Close(int index)
{
    OSYSDEBUG(("EchoServer::Close()\n"));

    if (connection[index].state == CONNECTION_CLOSED ||
        connection[index].state == CONNECTION_CLOSING) return oFAIL;

    TCPEndpointCloseMsg closeMsg(connection[index].endpoint);
    closeMsg.continuation = (void*)index;

    closeMsg.Send(ipstackRef, myOID_,
                  Extra_Entry[entryCloseCont], sizeof(closeMsg));

    connection[index].state = CONNECTION_CLOSING;

    return oSUCCESS;
}

void
EchoServer::CloseCont(ANTENVMSG msg)
{
    OSYSDEBUG(("EchoServer::CloseCont()\n"));
    
    TCPEndpointCloseMsg* closeMsg
        = (TCPEndpointCloseMsg*)antEnvMsg::Receive(msg);
    int index = (int)(closeMsg->continuation);

    connection[index].state = CONNECTION_CLOSED;
    Listen(index);
}

OStatus
EchoServer::InitTCPConnection(int index)
{
    OSYSDEBUG(("EchoServer::InitTCPConnection()\n"));

    connection[index].state = CONNECTION_CLOSED;

    // 
    // Allocate send buffer
    //
    antEnvCreateSharedBufferMsg sendBufferMsg(ECHOSERVER_BUFFER_SIZE);

    sendBufferMsg.Call(ipstackRef, sizeof(sendBufferMsg));
    if (sendBufferMsg.error != ANT_SUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s[%d] antError %d",
                  "EchoServer::InitTCPConnection()",
                  "Can't allocate send buffer",
                  index, sendBufferMsg.error));
        return oFAIL;
    }

    connection[index].sendBuffer = sendBufferMsg.buffer;
    connection[index].sendBuffer.Map();
    connection[index].sendData
        = (byte*)(connection[index].sendBuffer.GetAddress());

    //
    // Allocate receive buffer
    //
    antEnvCreateSharedBufferMsg recvBufferMsg(ECHOSERVER_BUFFER_SIZE);

    recvBufferMsg.Call(ipstackRef, sizeof(recvBufferMsg));
    if (recvBufferMsg.error != ANT_SUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s[%d] antError %d",
                  "EchoServer::InitTCPConnection()",
                  "Can't allocate receive buffer",
                  index, recvBufferMsg.error));
        return oFAIL;
    }

    connection[index].recvBuffer = recvBufferMsg.buffer;
    connection[index].recvBuffer.Map();
    connection[index].recvData
        = (byte*)(connection[index].recvBuffer.GetAddress());

    return oSUCCESS;
}

void
EchoServer::SetSendData(int index)
{
    memcpy(connection[index].sendData,
           connection[index].recvData, connection[index].sendSize);
}

void
EchoServer::SetReceiveData(int index)
{
    connection[index].recvSize = ECHOSERVER_BUFFER_SIZE;
}
