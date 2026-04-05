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

#ifndef EchoClient_h_DEFINED
#define EchoClient_h_DEFINED

#include <OPENR/OObject.h>
#include <OPENR/OSubject.h>
#include <OPENR/OObserver.h>
#include "TCPConnection.h"
#include "EchoClientConfig.h"
#include "def.h"

class EchoClient : public OObject {
public:
    EchoClient();
    virtual ~EchoClient() {}

    OSubject*   subject[numOfSubject];
    OObserver*  observer[numOfObserver];     

    virtual OStatus DoInit   (const OSystemEvent& event);
    virtual OStatus DoStart  (const OSystemEvent& event);
    virtual OStatus DoStop   (const OSystemEvent& event);
    virtual OStatus DoDestroy(const OSystemEvent& event);

    void ConnectCont(ANTENVMSG msg);
    void SendCont   (ANTENVMSG msg);
    void ReceiveCont(ANTENVMSG msg);
    void CloseCont  (ANTENVMSG msg);

private:
    OStatus Connect(int index);
    OStatus Send   (int index);
    OStatus Receive(int index);
    OStatus Close  (int index);
    OStatus InitTCPConnection(int index);

    void SetSendData(int index);
    void SetReceiveData(int index);

    antStackRef    ipstackRef;
    TCPConnection  connection[ECHOCLIENT_CONNECTION_MAX];
};

#endif // EchoClient_h_DEFINED
