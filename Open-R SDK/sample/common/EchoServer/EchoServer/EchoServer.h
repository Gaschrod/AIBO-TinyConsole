//
// Copyright 2002 Sony Corporation 
//
// Permission to use, copy, modify, and redistribute this software for
// non-commercial use is hereby granted.
//
// This software is provided "as is" without warranty of any kind,
// either expressed or implied, including but not limited to the
// implied warranties of fitness for a particular purpose.
//

#ifndef EchoServer_h_DEFINED
#define EchoServer_h_DEFINED

#include <OPENR/OObject.h>
#include <OPENR/OSubject.h>
#include <OPENR/OObserver.h>
#include "TCPConnection.h"
#include "EchoServerConfig.h"
#include "def.h"

class EchoServer : public OObject {
public:
    EchoServer();
    virtual ~EchoServer() {}

    OSubject*   subject[numOfSubject];
    OObserver*  observer[numOfObserver];     

    virtual OStatus DoInit   (const OSystemEvent& event);
    virtual OStatus DoStart  (const OSystemEvent& event);
    virtual OStatus DoStop   (const OSystemEvent& event);
    virtual OStatus DoDestroy(const OSystemEvent& event);

    void ListenCont (ANTENVMSG msg);
    void SendCont   (ANTENVMSG msg);
    void ReceiveCont(ANTENVMSG msg);
    void CloseCont  (ANTENVMSG msg);

private:
    OStatus Listen (int index);
    OStatus Send   (int index);
    OStatus Receive(int index);
    OStatus Close  (int index);
    OStatus InitTCPConnection(int index);

    void SetSendData(int index);
    void SetReceiveData(int index);

    antStackRef    ipstackRef;
    TCPConnection  connection[ECHOSERVER_CONNECTION_MAX];
};

#endif // EchoServer_h_DEFINED
