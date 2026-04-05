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
#include "EchoClient.h"
#include "entry.h"

EchoClient::EchoClient ()
{
}

OStatus
EchoClient::DoInit(const OSystemEvent& event)
{
    OSYSDEBUG(("EchoClient::DoInit()\n"));
    return oSUCCESS;
}

OStatus
EchoClient::DoStart(const OSystemEvent& event)
{
    OSYSDEBUG(("EchoClient::DoStart()\n"));

    ipstackRef = antStackRef("IPStack");

    for (int index = 0; index < ECHOCLIENT_CONNECTION_MAX; index++) {
        OStatus result = InitTCPConnection(index);
        if (result != oSUCCESS) return oFAIL;
    }

    Connect(0);

    return oSUCCESS;
}    

OStatus
EchoClient::DoStop(const OSystemEvent& event)
{
    OSYSDEBUG(("EchoClient::DoStop()\n"));
    return oSUCCESS;
}

OStatus
EchoClient::DoDestroy(const OSystemEvent& event)
{
    return oSUCCESS;
}

OStatus
EchoClient::Connect(int index)
{
    OSYSDEBUG(("EchoClient::Connect()\n"));

    if (connection[index].state != CONNECTION_CLOSED) return oFAIL;

    OSYSPRINT(("ECHOSERVER_IP is %s (EchoClientConfig.h)\n", ECHOSERVER_IP));
    TCPEndpointConnectMsg connectMsg(connection[index].endpoint,
                                     IP_ADDR_ANY, IP_PORT_ANY,
                                     ECHOSERVER_IP, ECHOSERVER_PORT);
    connectMsg.continuation = (void*)index;

    connectMsg.Send(ipstackRef, myOID_,
                    Extra_Entry[entryConnectCont], sizeof(connectMsg));
    
    connection[index].state = CONNECTION_CONNECTING;

    return oSUCCESS;
}

void
EchoClient::ConnectCont(ANTENVMSG msg)
{
    OSYSDEBUG(("EchoClient::ConnectCont()\n"));

    TCPEndpointConnectMsg* connectMsg
        = (TCPEndpointConnectMsg*)antEnvMsg::Receive(msg);
    int index = (int)connectMsg->continuation;

    if (connectMsg->error != TCP_SUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s %d",
                  "EchoClient::ConnectCont()",
                  "FAILED. connectMsg->error", connectMsg->error));
        connection[index].state = CONNECTION_CLOSED;
        return;
    }

    connection[index].state = CONNECTION_CONNECTED;

    SetSendData(index);
    Send(index);
}

OStatus
EchoClient::Send(int index)
{
    OSYSDEBUG(("EchoClient::Send()\n"));

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
EchoClient::SendCont(ANTENVMSG msg)
{
    OSYSDEBUG(("EchoClient::SendCont()\n"));

    TCPEndpointSendMsg* sendMsg = (TCPEndpointSendMsg*)antEnvMsg::Receive(msg);
    int index = (int)(sendMsg->continuation);

    if (sendMsg->error != TCP_SUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s %d",
                  "EchoClient::SendCont()",
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
EchoClient::Receive(int index)
{
    OSYSDEBUG(("EchoClient::Receive()\n"));

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
EchoClient::ReceiveCont(ANTENVMSG msg)
{
    OSYSDEBUG(("EchoClient::ReceiveCont()\n"));

    TCPEndpointReceiveMsg* receiveMsg
        = (TCPEndpointReceiveMsg*)antEnvMsg::Receive(msg);
    int index = (int)(receiveMsg->continuation);

    if (receiveMsg->error != TCP_SUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s %d",
                  "EchoClient::ReceiveCont()",
                  "FAILED. receiveMsg->error", receiveMsg->error));
        Close(index);
        return;
    }

    OSYSPRINT(("recvData : %s", connection[index].recvData));

    SetSendData(index);
    Send(index);
}

OStatus
EchoClient::Close(int index)
{
    OSYSDEBUG(("EchoClient::Close()\n"));

    if (connection[index].state == CONNECTION_CLOSED ||
        connection[index].state == CONNECTION_CLOSING) return oSUCCESS;

    TCPEndpointCloseMsg closeMsg(connection[index].endpoint);
    closeMsg.continuation = (void*)index;

    closeMsg.Send(ipstackRef, myOID_,
                  Extra_Entry[entryCloseCont], sizeof(closeMsg));

    connection[index].state = CONNECTION_CLOSING;

    return oSUCCESS;
}

void
EchoClient::CloseCont(ANTENVMSG msg)
{
    OSYSDEBUG(("EchoClient::CloseCont()\n"));

    TCPEndpointCloseMsg* closeMsg
        = (TCPEndpointCloseMsg*)antEnvMsg::Receive(msg);
    int index = (int)(closeMsg->continuation);

    connection[index].state = CONNECTION_CLOSED;
}

OStatus
EchoClient::InitTCPConnection(int index)
{
    OSYSDEBUG(("EchoClient::InitTCPConnection()\n"));

    //
    // Create endpoint
    //
    antEnvCreateEndpointMsg tcpCreateMsg(EndpointType_TCP,
                                         ECHOCLIENT_BUFFER_SIZE * 2);

    tcpCreateMsg.Call(ipstackRef, sizeof(tcpCreateMsg));
    if (tcpCreateMsg.error != ANT_SUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s[%d] antError %d",
                  "EchoClient::Connect()",
                  "Can't create endpoint",
                  index, tcpCreateMsg.error));
        return oFAIL;
    }

    connection[index].endpoint = tcpCreateMsg.moduleRef;
    connection[index].state = CONNECTION_CLOSED;

    // 
    // Allocate send buffer
    //
    antEnvCreateSharedBufferMsg sendBufferMsg(ECHOCLIENT_BUFFER_SIZE);

    sendBufferMsg.Call(ipstackRef, sizeof(sendBufferMsg));
    if (sendBufferMsg.error != ANT_SUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s[%d] antError %d",
                  "EchoClient::InitTCPConnection()",
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
    antEnvCreateSharedBufferMsg recvBufferMsg(ECHOCLIENT_BUFFER_SIZE);

    recvBufferMsg.Call(ipstackRef, sizeof(recvBufferMsg));
    if (recvBufferMsg.error != ANT_SUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s[%d] antError %d",
                  "EchoClient::InitTCPConnection()",
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
EchoClient::SetSendData(int index)
{
    strcpy((char*)connection[index].sendData, ECHOCLIENT_MESSAGE);
    connection[index].sendSize = strlen((char*)connection[index].sendData) + 1;
}

void
EchoClient::SetReceiveData(int index)
{
    connection[index].recvSize = ECHOCLIENT_BUFFER_SIZE;    
}
