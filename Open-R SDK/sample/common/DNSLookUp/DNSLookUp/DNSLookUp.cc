//
// Copyright 2003 Sony Corporation 
//
// Permission to use, copy, modify, and redistribute this software for
// non-commercial use is hereby granted.
//
// This software is provided "as is" without warranty of any kind,
// either expressed or implied, including but not limited to the
// implied warranties of fitness for a particular purpose.
//

#include <string.h>
#include <OPENR/OPENRAPI.h>
#include "DNSLookUp.h"
#include <EndpointTypes.h>
#include <DNSEndpointMsg.h>
#include "entry.h"

DNSLookUp::DNSLookUp()
{
}

OStatus
DNSLookUp::DoInit(const OSystemEvent& event)
{
    return oSUCCESS;
}

OStatus
DNSLookUp::DoStart(const OSystemEvent& event)
{
    ipstackRef = antStackRef("IPStack");
    antEnvCreateEndpointMsg createMsg(EndpointType_DNS, 16*1024);
    createMsg.Call(ipstackRef, sizeof(createMsg));
    if (createMsg.error != ANT_SUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s %d",
                  "DNSLookUp::DoStart()",
                  "createMsg.Call() FAILED", createMsg.error));
        return oFAIL;
    }

    dnsEndpoint = createMsg.moduleRef;
    GetDefaultServerByAddr();

    return oSUCCESS;
}    

OStatus
DNSLookUp::DoStop(const OSystemEvent& event)
{
    return oSUCCESS;
}

OStatus
DNSLookUp::DoDestroy(const OSystemEvent& event)
{
    return oSUCCESS;
}

void
DNSLookUp::GetDefaultServerByAddrCont(ANTENVMSG msg)
{
    DNSEndpointGetHostByAddrMsg* hostMsg
        = (DNSEndpointGetHostByAddrMsg*)antEnvMsg::Receive(msg);
    
    char addr[16];
    hostMsg->host_address.GetAsString(addr);

    if (hostMsg->n_address > 0) {
        OSYSPRINT(("Default Server : %s\n", hostMsg->name));
        OSYSPRINT(("Address        : %s\n", addr));
    } else {
        OSYSPRINT(("%s Non-existent host/domain\n", addr));
    }

    DNSEndpointGetDefaultDomainNameMsg getDomainMsg(dnsEndpoint);
    getDomainMsg.Call(ipstackRef, sizeof(getDomainMsg));
    if (getDomainMsg.error == DNS_SUCCESS) {
        OSYSPRINT(("Default domain : %s\n", getDomainMsg.name));
    }

    Prompt();
}

void
DNSLookUp::GetHostByNameCont(ANTENVMSG msg)
{
    DNSEndpointGetHostByNameMsg* hostMsg
        = (DNSEndpointGetHostByNameMsg*)antEnvMsg::Receive(msg);

    if (hostMsg->n_address > 0) {
        char addr[16];
        hostMsg->host_address.GetAsString(addr);
        OSYSPRINT(("Name    : %s\n", hostMsg->name));
        OSYSPRINT(("Address : %s\n", addr));
    } else {
        OSYSPRINT(("%s Non-existent host/domain\n", hostMsg->name));
    }
    
    Prompt();
}

void
DNSLookUp::GetDefaultServerByAddr()
{
    //
    // DNS server and default domain name are specified in WLANCONF.TXT
    // 
    //   DNS_SERVER_1=10.0.1.1
    //   DNS_DEFDNAME=example.net
    //

    DNSEndpointGetServerAddressesMsg getServerAddressesMsg(dnsEndpoint);
    getServerAddressesMsg.Call(ipstackRef, sizeof(getServerAddressesMsg));
    if (getServerAddressesMsg.error != DNS_SUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s %d",
                  "DNSLookUp::GetDefaultServerByAddr()",
                  "getServerAddressesMsg.Call() DNSEndpointError",
                  getServerAddressesMsg.error));
        return;
    }

    antError error;
    DNSEndpointGetHostByAddrMsg getHostMsg(dnsEndpoint,
                                           getServerAddressesMsg.addrList[0]);
    error = getHostMsg.Send(ipstackRef, myOID_,
                            Extra_Entry[entryGetDefaultServerByAddrCont],
                            sizeof(getHostMsg));
    if (error != ANT_SUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s %d",
                  "DNSLookUp::GetDefaultServerByAddr()",
                  "getHostMsg.Send() antError", error));
    }
}

void
DNSLookUp::GetHostByName(char* hostname)
{
    antError error;
    DNSEndpointGetHostByNameMsg getHostMsg(dnsEndpoint, hostname);
    error = getHostMsg.Send(ipstackRef,
                            myOID_, Extra_Entry[entryGetHostByNameCont],
                            sizeof(getHostMsg));
    if (error != ANT_SUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s %d",
                  "DNSLookUp::GetHostByName()",
                  "getHostMsg.Send() FAILED", error));
    }
}

void
DNSLookUp::Prompt()
{
    OSYSPRINT(("DNSLookUp> "));
    char line[128];
    gets(line);

    if (strcmp(line, "shutdown") == 0) {
        OSYSPRINT(("SHUTDOWN ...\n"));
        OBootCondition bootCond(obcbPAUSE_SW);
        OPENR::Shutdown(bootCond);
    } else {
        //
        // If you don't put a '.' at the end of an FQDN, 
        // it's not recognized as an FQDN.
        //
        // See RFC1912.
        //
        char* ptr;
        if ((ptr = strrchr(line, '.')) != 0) {
            int len = strlen(line);
            char* last = line + len - 1;
            if (ptr != last) {
                line[len]   = '.';
                line[len+1] = 0;
            }
        }

        GetHostByName(line);
    }
}
