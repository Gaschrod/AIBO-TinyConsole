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

#ifndef UDPEchoServer_h_DEFINED
#define UDPEchoServer_h_DEFINED

#include <OPENR/OObject.h>
#include <OPENR/OSubject.h>
#include <OPENR/OObserver.h>
#include "UDPConnection.h"
#include "UDPEchoServerConfig.h"
#include "def.h"

class UDPEchoServer : public OObject {
public:
    UDPEchoServer();
    virtual ~UDPEchoServer() {}

    OSubject*   subject[numOfSubject];
    OObserver*  observer[numOfObserver];     

    virtual OStatus DoInit   (const OSystemEvent& event);
    virtual OStatus DoStart  (const OSystemEvent& event);
    virtual OStatus DoStop   (const OSystemEvent& event);
    virtual OStatus DoDestroy(const OSystemEvent& event);

    void SendCont   (ANTENVMSG msg);
    void ReceiveCont(ANTENVMSG msg);
    void CloseCont  (ANTENVMSG msg);

private:
    OStatus Bind   (int index);
    OStatus Send   (int index);
    OStatus Receive(int index);
    OStatus Close  (int index);
    OStatus InitUDPBuffer(int index);
    OStatus CreateUDPEndpoint(int index);

    antStackRef    ipstackRef;
    UDPConnection  connection[UDPECHOSERVER_CONNECTION_MAX];
};

#endif // UDPEchoServer_h_DEFINED
