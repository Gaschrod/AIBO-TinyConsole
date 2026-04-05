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

#ifndef NTP_h_DEFINED
#define NTP_h_DEFINED

#include <OPENR/OObject.h>
#include <OPENR/OSubject.h>
#include <OPENR/OObserver.h>
#include "UDPConnection.h"
#include "NTPConfig.h"
#include "def.h"

struct NTPMessage {
    longword data[12];
};

class NTP : public OObject {
public:
    NTP();
    virtual ~NTP() {}

    OSubject*   subject[numOfSubject];
    OObserver*  observer[numOfObserver];     

    virtual OStatus DoInit   (const OSystemEvent& event);
    virtual OStatus DoStart  (const OSystemEvent& event);
    virtual OStatus DoStop   (const OSystemEvent& event);
    virtual OStatus DoDestroy(const OSystemEvent& event);

    void GetHostByNameCont(ANTENVMSG msg);
    void SendCont         (ANTENVMSG msg);
    void ReceiveCont      (ANTENVMSG msg);

private:
    static const longword SECONDS_OF_100YEARS = 3155673600U;

    void GetHostByName(char* hostname);
    OStatus InitUDPBuffer();
    OStatus CreateUDPEndpoint();
    OStatus CloseUDPEndpoint();
    OStatus SendNTPPacket();
    OStatus Send   ();
    OStatus Receive();

    antStackRef    ipstackRef;
    antModuleRef   dnsEndpoint;
    UDPConnection  connection;
    char           ntpserver[128];
};

#endif // NTP_h_DEFINED
