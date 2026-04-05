//
// Copyright 2004 Sony Corporation 
//
// Permission to use, copy, modify, and redistribute this software for
// non-commercial use is hereby granted.
//
// This software is provided "as is" without warranty of any kind,
// either expressed or implied, including but not limited to the
// implied warranties of fitness for a particular purpose.
//

#include <stdio.h>
#include <string.h>
#include <OPENR/OSyslog.h>
#include <OPENR/OPENRAPI.h>
#include <OPENR/OTime.h>
#include <OPENR/OCalendarTime.h>
#include <ant.h>
#include <EndpointTypes.h>
#include <DNSEndpointMsg.h>
#include <UDPEndpointMsg.h>
#include "NTP.h"
#include "entry.h"

static bool
is_ipv4addr_dotted_decimal_notation(const char* str)
{
    int part[4];
    int index;
    char c;

    if (str == 0) return false;

    part[0] = part[1] = part[2] = part[3] = 0;
    for (index = 0; c = *str; *str++) {
        if ('.' == c) {
            /* we go to next part */
            if (++index >= 4) {
                break;
            }
        } else if (isdigit(c)) {
            part[index] = part[index] * 10 + (c - '0');
        } else {
            return false;
        }
    }
    
    if (index == 3 &&
        part[0] < 256 && part[1] < 256 && part[2] < 256 && part[3] < 256) {
        return true;
    } else {
        return false;
    }
}

NTP::NTP ()
{
}

OStatus
NTP::DoInit(const OSystemEvent& event)
{
    OSYSDEBUG(("NTP::DoInit()\n"));
    return oSUCCESS;
}

OStatus
NTP::DoStart(const OSystemEvent& event)
{
    OSYSDEBUG(("NTP::DoStart()\n"));

    ipstackRef = antStackRef("IPStack");

    OStatus result = InitUDPBuffer();
    if (result != oSUCCESS) return oFAIL;

    {
        FILE* fp = fopen(NTP_CONF_FILE, "r");
        if (fp == NULL) {
            OSYSLOG1((osyslogERROR, "Can't open %s", NTP_CONF_FILE));
            return oFAIL;
        }

        fgets(ntpserver, 128, fp);
        fclose(fp);

        char* p;
        if ((p = strchr(ntpserver, '\r')) != NULL) *p = '\0';
        if ((p = strchr(ntpserver, '\n')) != NULL) *p = '\0';
        if (ntpserver[strlen(ntpserver)-1] != '.') {
            strcat(ntpserver, ".");
        }

        OSYSDEBUG(("ntpserver %s\n", ntpserver));
    }

    antEnvCreateEndpointMsg createMsg(EndpointType_DNS, 16*1024);
    createMsg.Call(ipstackRef, sizeof(createMsg));
    if (createMsg.error != ANT_SUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s %d",
                  "NTP::DoStart()",
                  "createMsg.Call() FAILED", createMsg.error));
        return oFAIL;
    }

    dnsEndpoint = createMsg.moduleRef;
    GetHostByName(ntpserver);

    return oSUCCESS;
}    

OStatus
NTP::DoStop(const OSystemEvent& event)
{
    OSYSDEBUG(("NTP::DoStop()\n"));
    return oSUCCESS;
}

OStatus
NTP::DoDestroy(const OSystemEvent& event)
{
    return oSUCCESS;
}

void
NTP::GetHostByName(char* hostname)
{
    antError error;
    DNSEndpointGetHostByNameMsg getHostMsg(dnsEndpoint, hostname);
    error = getHostMsg.Send(ipstackRef,
                            myOID_, Extra_Entry[entryGetHostByNameCont],
                            sizeof(getHostMsg));
    if (error != ANT_SUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s %d",
                  "NTP::GetHostByName()",
                  "getHostMsg.Send() FAILED", error));
    }
}

void
NTP::GetHostByNameCont(ANTENVMSG msg)
{
    DNSEndpointGetHostByNameMsg* hostMsg
        = (DNSEndpointGetHostByNameMsg*)antEnvMsg::Receive(msg);

#ifdef OPENR_DEBUG
    char addr[16];
    OSYSDEBUG(("hostMsg->error          %d\n", hostMsg->error));
    OSYSDEBUG(("hostMsg->name           %s\n", hostMsg->name));
    hostMsg->server_address.GetAsString(addr);
    OSYSDEBUG(("hostMsg->server_address %s\n", addr));
    hostMsg->host_address.GetAsString(addr);
    OSYSDEBUG(("hostMsg->host_address   %s\n", addr));
    OSYSDEBUG(("hostMsg->n_address      %d\n", hostMsg->n_address));
    OSYSDEBUG(("hostMsg->n_alias        %d\n", hostMsg->n_alias));
#endif

    //
    // DNS Endpoint close : NOT YET
    //

    int len = strlen(hostMsg->name);
    if (hostMsg->name[len-1] == '.') hostMsg->name[len-1] = '\0';

    if (hostMsg->error == DNS_SUCCESS && hostMsg->n_address > 0) {
        
        connection.sendAddress = hostMsg->host_address.Address();

    } else if (is_ipv4addr_dotted_decimal_notation(hostMsg->name) == true) {

        OSYSDEBUG(("hostMsg->name %s is IPv4 address.\n", hostMsg->name));
        connection.sendAddress= IPAddress(hostMsg->name);
        
    } else {

        OSYSLOG1((osyslogERROR,
                  "GetHostByNameCont() : %s Non-existent host",
                  hostMsg->name));
	return;
    }
    
    OStatus result = CreateUDPEndpoint();
    if (result != oSUCCESS) return;
    
    SendNTPPacket();
}

OStatus
NTP::SendNTPPacket()
{
    byte* ptr = (byte*)connection.sendData;
    for (int i = 0; i < 48; i++) {
        ptr[i] = 0x00;
    }
    ptr[0] = 0x0b;
    connection.sendSize = 48;
    connection.sendPort = NTP_PORT;

    return Send();
}

OStatus
NTP::Send()
{
    OSYSDEBUG(("NTP::Send()\n"));

    UDPEndpointSendMsg sendMsg(connection.endpoint,
			       connection.sendAddress,
			       connection.sendPort,
                               connection.sendData,
                               connection.sendSize);
    sendMsg.continuation = (void*)0;

    sendMsg.Send(ipstackRef, myOID_,
                 Extra_Entry[entrySendCont],
                 sizeof(UDPEndpointSendMsg));

    connection.state = CONNECTION_SENDING;
    connection.sendSize = 0;
    return oSUCCESS;
}

void
NTP::SendCont(ANTENVMSG msg)
{
    OSYSDEBUG(("NTP::SendCont()\n"));

    UDPEndpointSendMsg* sendMsg = (UDPEndpointSendMsg*)antEnvMsg::Receive(msg);

    if (sendMsg->error != UDP_SUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s %d",
                  "NTP::SendCont()",
                  "FAILED. sendMsg->error", sendMsg->error));
        CloseUDPEndpoint();
        return;
    }

    connection.state = CONNECTION_CONNECTED;

    connection.recvSize = 48;
    Receive();
}

OStatus
NTP::Receive()
{
    OSYSDEBUG(("NTP::Receive()\n"));

    UDPEndpointReceiveMsg receiveMsg(connection.endpoint,
                                     connection.recvData,
                                     connection.recvSize);
    receiveMsg.continuation = (void*)0;

    receiveMsg.Send(ipstackRef, myOID_,
                    Extra_Entry[entryReceiveCont], sizeof(receiveMsg));

    return oSUCCESS;
}

void
NTP::ReceiveCont(ANTENVMSG msg)
{
    OSYSDEBUG(("NTP::ReceiveCont()\n"));

    UDPEndpointReceiveMsg* receiveMsg
        = (UDPEndpointReceiveMsg*)antEnvMsg::Receive(msg);

    if (receiveMsg->error != UDP_SUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s %d",
                  "NTP::ReceiveCont()",
                  "FAILconnectionED. receiveMsg->error", receiveMsg->error));
    }

    OSYSDEBUG(("recvSize %d\n", connection.recvSize));
    NTPMessage* ntpmsg = (NTPMessage*)connection.recvData;
    byte* p = (byte*)&(ntpmsg->data[10]);
    longword t0 = (longword)p[0];
    longword t1 = (longword)p[1];
    longword t2 = (longword)p[2];
    longword t3 = (longword)p[3];
    longword t = (t0<<24)|(t1<<16)|(t2<<8)|t3;
    OSYSDEBUG(("ntpmsg->data[10] %u t %u\n", ntpmsg->data[10], t));

    //
    // NTP  : relative seconds to 0h on 1 January 1900. See RFC2030.
    // AIBO : relative seconds to 0h on 1 January 2000.
    //
    t -= SECONDS_OF_100YEARS;

    sbyte d;
    OPENR::GetTimeDifference(&d);
    OTime time(t, d);
    OPENR::SetTime(time);

    char date[128];
    OCalendarTime ctime(time);
    ctime.RFC2822(date);
    OSYSPRINT(("%s\n", date));

    CloseUDPEndpoint();
}

OStatus
NTP::InitUDPBuffer()
{
    OSYSDEBUG(("NTP::InitUDPBuffer()\n"));

    connection.state = CONNECTION_CLOSED;

    // 
    // Allocate send buffer
    //
    antEnvCreateSharedBufferMsg sendBufferMsg(NTP_BUFFER_SIZE);

    sendBufferMsg.Call(ipstackRef, sizeof(sendBufferMsg));
    if (sendBufferMsg.error != ANT_SUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s antError %d",
                  "NTP::InitUDPBuffer()",
                  "Can't allocate send buffer",
                  sendBufferMsg.error));
        return oFAIL;
    }

    connection.sendBuffer = sendBufferMsg.buffer;
    connection.sendBuffer.Map();
    connection.sendData
        = (byte*)(connection.sendBuffer.GetAddress());

    //
    // Allocate receive buffer
    //
    antEnvCreateSharedBufferMsg recvBufferMsg(NTP_BUFFER_SIZE);

    recvBufferMsg.Call(ipstackRef, sizeof(recvBufferMsg));
    if (recvBufferMsg.error != ANT_SUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s antError %d",
                  "NTP::InitUDPBuffer()",
                  "Can't allocate receive buffer",
                  recvBufferMsg.error));
        return oFAIL;
    }

    connection.recvBuffer = recvBufferMsg.buffer;
    connection.recvBuffer.Map();
    connection.recvData
        = (byte*)(connection.recvBuffer.GetAddress());
    connection.recvSize = NTP_BUFFER_SIZE;

    return oSUCCESS;
}

OStatus
NTP::CreateUDPEndpoint()
{
    OSYSDEBUG(("NTP::CreateUDPEndpoint()\n"));

    if (connection.state != CONNECTION_CLOSED) return oFAIL;

    //
    // Create UDP endpoint
    //
    antEnvCreateEndpointMsg udpCreateMsg(EndpointType_UDP,
                                         NTP_BUFFER_SIZE * 2);
    udpCreateMsg.Call(ipstackRef, sizeof(udpCreateMsg));
    if (udpCreateMsg.error != ANT_SUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s antError %d",
                  "NTP::CreateUDPEndpoint()",
                  "Can't create endpoint",
                  udpCreateMsg.error));
        return oFAIL;
    }
    connection.endpoint = udpCreateMsg.moduleRef;

    // 
    // Bind
    //
    UDPEndpointBindMsg bindMsg(connection.endpoint, 
                               IP_ADDR_ANY, IP_PORT_ANY);
    bindMsg.Call(ipstackRef,sizeof(antEnvCreateEndpointMsg));
    if (bindMsg.error != UDP_SUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s antError %d",
                  "NTP::CreateUDPEndpoint()",
                  "Bind error", bindMsg.error));
        return oFAIL;
    }

    return oSUCCESS;
}

OStatus
NTP::CloseUDPEndpoint()
{
    OSYSDEBUG(("NTP::CloseUDPEndpoint()\n"));
}
