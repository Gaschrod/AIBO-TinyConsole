//
// TinyConsole.h
// Minimal OPEN-R TCP console with application-layer XOR.
//
//   PC client                   AIBO (this object)
//   ---------                   ------------------
//   connect()        ------>    ListenCont()  -> send BANNER (plaintext)
//   recv BANNER
//   -- XOR session starts here on both sides --
//   send XOR(cmd\n)  ------>    ReceiveCont() -> decode -> dispatch
//                               Send()        -> encode response
//   recv XOR(resp)   <------
//   send XOR(QUIT\n) ------>    ReceiveCont() -> sets pendingClose_
//                               Send() BYE
//   recv XOR(BYE)    <------
//                               SendCont()    -> Close()
//                               CloseCont()   -> Listen() (next client)
//

#ifndef _TinyConsole_h_DEFINED
#define _TinyConsole_h_DEFINED

#include <OPENR/OObject.h>
#include <OPENR/OSubject.h>
#include <OPENR/OObserver.h>
#include <ant.h>
#include <EndpointTypes.h>
#include <TCPEndpointMsg.h>
#include <OPENR/core_macro.h>

#include "TCPConnection.h"
#include "ConsoleConfig.h"
#include "def.h"
#include "entry.h"

class TinyConsole : public OObject
{
public:
    TinyConsole();
    virtual ~TinyConsole() {}

    OSubject*  subject[numOfSubject];
    OObserver* observer[numOfObserver];

    // OPEN-R lifecycle
    virtual OStatus DoInit   (const OSystemEvent& event);
    virtual OStatus DoStart  (const OSystemEvent& event);
    virtual OStatus DoStop   (const OSystemEvent& event);
    virtual OStatus DoDestroy(const OSystemEvent& event);

    // ANT extra-entry callbacks
    void ListenCont (ANTENVMSG msg);
    void SendCont   (ANTENVMSG msg);
    void ReceiveCont(ANTENVMSG msg);
    void CloseCont  (ANTENVMSG msg);

private:
    OStatus Listen ();
    OStatus Send   (const byte* data, int size);
    OStatus Receive();
    OStatus Close  ();

    static void XorBuffer(byte* buf, int len, int& offset);

    antStackRef   ipstackRef_;
    TCPConnection conn_;

    int  xorTxOffset_;
    int  xorRxOffset_;
    bool pendingClose_;  // set by QUIT handler, acted on in SendCont
};

#endif // _TinyConsole_h_DEFINED

