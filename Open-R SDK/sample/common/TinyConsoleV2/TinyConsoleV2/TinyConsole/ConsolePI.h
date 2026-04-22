#ifndef _ConsolePI_h_DEFINED
#define _ConsolePI_h_DEFINED

#include "ConsoleConfig.h"
#include <OPENR/OSyslog.h>
#include <EndpointTypes.h>
#include <TCPEndpointMsg.h>

class ConsolePI {
public:
    ConsolePI();
    virtual ~ConsolePI() {}

    OStatus Initialize(const OID& myoid, const antStackRef& ipstack);
    
    void ListenCont (TCPEndpointListenMsg* msg);
    void SendCont   (TCPEndpointSendMsg* msg);
    void ReceiveCont(TCPEndpointReceiveMsg* msg);
    void CloseCont  (TCPEndpointCloseMsg* msg);
    
    OStatus Close();

private:
    OStatus Listen();
    OStatus Send(const char* data);
    OStatus Receive();
    void ApplyXOR(byte* buf, int len, int& offset);

    OID myOID;
    antStackRef ipstackRef;
    TCPConnection conn;
    
    int xorRxOffset;
    int xorTxOffset;
};

#endif
