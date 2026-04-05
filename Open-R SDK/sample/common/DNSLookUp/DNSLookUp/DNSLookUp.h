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

#ifndef DNSLookUp_h_DEFINED
#define DNSLookUp_h_DEFINED

#include <OPENR/OObject.h>
#include <OPENR/OSubject.h>
#include <OPENR/OObserver.h>
#include <ant.h>
#include "def.h"

class DNSLookUp : public OObject {
public:
    DNSLookUp();
    virtual ~DNSLookUp() {}

    OSubject*   subject[numOfSubject];
    OObserver*	observer[numOfObserver];     

    virtual OStatus DoInit   (const OSystemEvent& event);
    virtual OStatus DoStart  (const OSystemEvent& event);
    virtual OStatus DoStop   (const OSystemEvent& event);
    virtual OStatus DoDestroy(const OSystemEvent& event);

    void GetDefaultServerByAddrCont(ANTENVMSG msg);
    void GetHostByNameCont(ANTENVMSG msg);

private:
    void GetDefaultServerByAddr();
    void GetHostByName(char* hostname);
    void Prompt();

    antStackRef  ipstackRef;
    antModuleRef dnsEndpoint;
};

#endif // DNSLookUp_h_DEFINED
