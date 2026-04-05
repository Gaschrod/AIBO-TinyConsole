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
#include <UDPEndpointMsg.h>
#include "UDPEchoServer.h"
#include "entry.h"

UDPEchoServer::UDPEchoServer ()
{
}

OStatus
UDPEchoServer::DoInit(const OSystemEvent& event)
{
    OSYSDEBUG(("UDPEchoServer::DoInit()\n"));

    ipstackRef = antStackRef("IPStack");

    for (int index = 0; index < UDPECHOSERVER_CONNECTION_MAX; index++) {
        OStatus result = InitUDPBuffer(index);
        if (result != oSUCCESS) return oFAIL;
    }

    return oSUCCESS;
}

OStatus
UDPEchoServer::DoStart(const OSystemEvent& event)
{
    OStatus result;
    OSYSDEBUG(("UDPEchoServer::DoStart()\n"));

    for (int index = 0; index < UDPECHOSERVER_CONNECTION_MAX; index++) {
        // 
        // Bind and Start receive data
        //
        result = CreateUDPEndpoint(index);
        if (result != oSUCCESS) {
            OSYSLOG1((osyslogERROR, "%s : %s[%d]",
                  "UDPEchoServer::DoStart()",
                  "CreateUDPEndpoint() fail", index));
            continue;
        }
        result = Bind(index);
        if (result != oSUCCESS) {
            OSYSLOG1((osyslogERROR, "%s : %s[%d]",
                  "UDPEchoServer::DoStart()",
                  "Bind() fail", index));
            continue;
        }
    }

    return oSUCCESS;
}    

OStatus
UDPEchoServer::DoStop(const OSystemEvent& event)
{
    OSYSDEBUG(("UDPEchoServer::DoStop()\n"));

    for (int index = 0; index < UDPECHOSERVER_CONNECTION_MAX; index++) {
        if (connection[index].state != CONNECTION_CLOSED &&
            connection[index].state != CONNECTION_CLOSING) {

            // Connection close
            UDPEndpointCloseMsg closeMsg(connection[index].endpoint);
            closeMsg.Call(ipstackRef, sizeof(closeMsg));
            connection[index].state = CONNECTION_CLOSED;
        }
    }

    return oSUCCESS;
}

OStatus
UDPEchoServer::DoDestroy(const OSystemEvent& event)
{
    for (int index = 0; index < UDPECHOSERVER_CONNECTION_MAX; index++) {
        // UnMap and Destroy SendBuffer
        connection[index].sendBuffer.UnMap();
        antEnvDestroySharedBufferMsg destroySendBufferMsg(connection[index].sendBuffer);
        destroySendBufferMsg.Call(ipstackRef, sizeof(destroySendBufferMsg));

        // UnMap and Destroy RecvBuffer
        connection[index].recvBuffer.UnMap();
        antEnvDestroySharedBufferMsg destroyRecvBufferMsg(connection[index].recvBuffer);
        destroyRecvBufferMsg.Call(ipstackRef, sizeof(destroyRecvBufferMsg));
    }

    return oSUCCESS;
}

OStatus
UDPEchoServer::Send(int index)
{
    OSYSDEBUG(("UDPEchoServer::Send()\n"));

    if (connection[index].sendSize == 0 ||
        connection[index].state != CONNECTION_CONNECTED) return oFAIL;

    UDPEndpointSendMsg sendMsg(connection[index].endpoint,
                               connection[index].sendAddress,
                               connection[index].sendPort,
                               connection[index].sendData,
                               connection[index].sendSize);
    sendMsg.continuation = (void*)index;
    sendMsg.Send(ipstackRef, myOID_,
                 Extra_Entry[entrySendCont], sizeof(UDPEndpointSendMsg));
                 

    connection[index].state = CONNECTION_SENDING;
    return oSUCCESS;
}

void
UDPEchoServer::SendCont(ANTENVMSG msg)
{
    OSYSDEBUG(("UDPEchoServer::SendCont()\n"));

    UDPEndpointSendMsg* sendMsg = (UDPEndpointSendMsg*)antEnvMsg::Receive(msg);
    int index = (int)(sendMsg->continuation);
    if (connection[index].state == CONNECTION_CLOSED)
        return;

    if (sendMsg->error != UDP_SUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s %d",
                  "UDPEchoServer::SendCont()",
                  "FAILED. sendMsg->error", sendMsg->error));
        Close(index);
        return;
    }
    char in_buff[64];
    OSYSPRINT(("sendData : %s", connection[index].sendData));
    OSYSPRINT(("sendAddress : %s\n", connection[index].sendAddress.GetAsString(in_buff)));
    OSYSPRINT(("sendPort : %d\n\n", connection[index].sendPort));
    
    connection[index].state = CONNECTION_CONNECTED;
    connection[index].recvSize = UDPECHOSERVER_BUFFER_SIZE;
    Receive(index);
}

OStatus
UDPEchoServer::Receive(int index)
{
    OSYSDEBUG(("UDPEchoServer::Receive()\n"));

    if (connection[index].state != CONNECTION_CONNECTED &&
        connection[index].state != CONNECTION_SENDING) return oFAIL;

    UDPEndpointReceiveMsg receiveMsg(connection[index].endpoint,
                                     connection[index].recvData,
                                     connection[index].recvSize);
    receiveMsg.continuation = (void*)index;

    receiveMsg.Send(ipstackRef, myOID_,
                    Extra_Entry[entryReceiveCont], sizeof(receiveMsg));

    return oSUCCESS;
}

void
UDPEchoServer::ReceiveCont(ANTENVMSG msg)
{
    OSYSDEBUG(("UDPEchoServer::ReceiveCont()\n"));

    UDPEndpointReceiveMsg* receiveMsg
        = (UDPEndpointReceiveMsg*)antEnvMsg::Receive(msg);

    int index = (int)(receiveMsg->continuation);
    if (connection[index].state == CONNECTION_CLOSED) return;

    if (receiveMsg->error != UDP_SUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s %d",
                  "UDPEchoServer::ReceiveCont()",
                  "FAILED. receiveMsg->error", receiveMsg->error));
        Close(index);
        return;
    }

    OSYSPRINT(("recvData : %s", receiveMsg->buffer));

    //
    // Send back the message
    //
    connection[index].sendAddress = receiveMsg->address;
    connection[index].sendPort    = receiveMsg->port;
    connection[index].sendSize    = receiveMsg->size;
    memcpy(connection[index].sendData, receiveMsg->buffer, receiveMsg->size);

    Send(index);
}

OStatus
UDPEchoServer::Close(int index)
{
    OSYSDEBUG(("UDPEchoServer::Close()\n"));

    if (connection[index].state == CONNECTION_CLOSED ||
        connection[index].state == CONNECTION_CLOSING) return oFAIL;

    UDPEndpointCloseMsg closeMsg(connection[index].endpoint);
    closeMsg.continuation = (void*)index;

    closeMsg.Send(ipstackRef, myOID_,
                  Extra_Entry[entryCloseCont], sizeof(closeMsg));

    connection[index].state = CONNECTION_CLOSING;

    return oSUCCESS;
}

void
UDPEchoServer::CloseCont(ANTENVMSG msg)
{
    OStatus result;
    OSYSDEBUG(("UDPEchoServer::CloseCont()\n"));
    
    UDPEndpointCloseMsg* closeMsg = (UDPEndpointCloseMsg*)antEnvMsg::Receive(msg);
    int index = (int)(closeMsg->continuation);
    if (connection[index].state == CONNECTION_CLOSED)
        return;

    connection[index].state = CONNECTION_CLOSED;

    result = CreateUDPEndpoint(index);
    if (result != oSUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s[%d]",
                  "UDPEchoServer::CloseCont()",
                  "CreateUDPEndpoint() fail", index));
        return;
    }
    result = Bind(index);
    if (result != oSUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s[%d]",
                  "UDPEchoServer::CloseCont()",
                  "Bind() fail", index));
    }
}

OStatus
UDPEchoServer::InitUDPBuffer(int index)
{
    OSYSDEBUG(("UDPEchoServer::InitUDPBuffer()\n"));

    connection[index].state = CONNECTION_CLOSED;

    // 
    // Allocate send buffer
    //
    antEnvCreateSharedBufferMsg sendBufferMsg(UDPECHOSERVER_BUFFER_SIZE);

    sendBufferMsg.Call(ipstackRef, sizeof(sendBufferMsg));
    if (sendBufferMsg.error != ANT_SUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s[%d] antError %d",
                  "UDPEchoServer::InitUDPBuffer()",
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
    antEnvCreateSharedBufferMsg recvBufferMsg(UDPECHOSERVER_BUFFER_SIZE);

    recvBufferMsg.Call(ipstackRef, sizeof(recvBufferMsg));
    if (recvBufferMsg.error != ANT_SUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s[%d] antError %d",
                  "UDPEchoServer::InitUDPBuffer()",
                  "Can't allocate receive buffer",
                  index, recvBufferMsg.error));
        return oFAIL;
    }

    connection[index].recvBuffer = recvBufferMsg.buffer;
    connection[index].recvBuffer.Map();
    connection[index].recvData
        = (byte*)(connection[index].recvBuffer.GetAddress());
    connection[index].recvSize = UDPECHOSERVER_BUFFER_SIZE;

    return oSUCCESS;
}

OStatus
UDPEchoServer::CreateUDPEndpoint(int index)
{
    OSYSDEBUG(("UDPEchoServer::CreateUDPEndpoint()\n"));

    if (connection[index].state != CONNECTION_CLOSED) return oFAIL;

    //
    // Create UDP endpoint
    //
    antEnvCreateEndpointMsg udpCreateMsg(EndpointType_UDP,
                                         UDPECHOSERVER_BUFFER_SIZE * 2);
    udpCreateMsg.Call(ipstackRef, sizeof(udpCreateMsg));
    if (udpCreateMsg.error != ANT_SUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s[%d] antError %d",
                  "UDPEchoServer::CreateUDPEndpoint()",
                  "Can't create endpoint",
                  index, udpCreateMsg.error));
        return oFAIL;
    }
    connection[index].endpoint = udpCreateMsg.moduleRef;

    return oSUCCESS;
}


OStatus
UDPEchoServer::Bind(int index)
{
    OSYSDEBUG(("UDPEchoServer::Bind()\n"));

    if (connection[index].state != CONNECTION_CLOSED) return oFAIL;

    // 
    // Bind
    //
    UDPEndpointBindMsg bindMsg(connection[index].endpoint, 
                               IP_ADDR_ANY, UDPECHOSERVER_PORT);
    bindMsg.Call(ipstackRef,sizeof(antEnvCreateEndpointMsg));
    if (bindMsg.error != UDP_SUCCESS) {
        return oFAIL;
    }

    connection[index].state = CONNECTION_CONNECTED;
    connection[index].recvSize = UDPECHOSERVER_BUFFER_SIZE;

    Receive(index);

    return oSUCCESS;
}
