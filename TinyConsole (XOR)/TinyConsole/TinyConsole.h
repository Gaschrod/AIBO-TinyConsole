//
// TinyConsole.h
// Minimal OPEN-R TCP console with application-layer XOR.
//
// ANT TCP lifecycle:
//  1. DoStart()     -> Listen()         allocate endpoint, post ListenMsg
//  2. ListenCont()  <- scheduler        client connected
//                   -> send BANNER (plain text, no XOR)
//  3. SendCont()    <- scheduler        send complete
//                   -> Receive()        arm for next incoming data
//  4. ReceiveCont() <- scheduler        data arrived
//                   -> XOR decode, dispatch, XOR encode reply, Send()
//  5. SendCont()    <- scheduler        reply sent
//                   -> Receive() again, or Close() if QUIT was received
//  6. CloseCont()   <- scheduler        connection closed
//                   -> Listen() again   wait for next client
//

#ifndef TinyConsole_h_DEFINED
#define TinyConsole_h_DEFINED

#include <OPENR/OObject.h>
#include <OPENR/OSubject.h>
#include <OPENR/OObserver.h>
#include <ant.h>
#include <EndpointTypes.h>
#include <TCPEndpointMsg.h>

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

    antStackRef   ipstackRef;
    TCPConnection conn;

    int  xorTxOffset;
    int  xorRxOffset;
    bool pendingClose;  // set by QUIT handler, acted on in SendCont
};

#endif // TinyConsole_h_DEFINED