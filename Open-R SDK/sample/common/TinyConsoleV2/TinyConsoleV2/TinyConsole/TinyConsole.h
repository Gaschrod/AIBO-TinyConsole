#ifndef _TinyConsole_h_DEFINED
#define _TinyConsole_h_DEFINED

#include <OPENR/OObject.h>
#include <OPENR/OSubject.h>
#include <OPENR/OObserver.h>
#include "ConsolePI.h"
#include "def.h"

class TinyConsole : public OObject {
public:
    TinyConsole() {}
    virtual ~TinyConsole() {}

    OSubject* subject[numOfSubject];
    OObserver* observer[numOfObserver];     

    virtual OStatus DoInit   (const OSystemEvent& event);
    virtual OStatus DoStart  (const OSystemEvent& event);
    virtual OStatus DoStop   (const OSystemEvent& event);
    virtual OStatus DoDestroy(const OSystemEvent& event);

    void ListenCont (ANTENVMSG msg);
    void SendCont   (ANTENVMSG msg);
    void ReceiveCont(ANTENVMSG msg);
    void CloseCont  (ANTENVMSG msg);

private:
    antStackRef ipstackRef;
    ConsolePI console;
};

#endif

