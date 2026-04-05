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

#ifndef SampleObserver_h_DEFINED
#define SampleObserver_h_DEFINED

#include <OPENR/OObject.h>
#include <OPENR/OSubject.h>
#include <OPENR/OObserver.h>
#include "def.h"

class SampleObserver : public OObject {
public:
    SampleObserver();
    virtual ~SampleObserver() {}

    OSubject*    subject[numOfSubject];
    OObserver*   observer[numOfObserver];

    virtual OStatus DoInit   (const OSystemEvent& event);
    virtual OStatus DoStart  (const OSystemEvent& event);
    virtual OStatus DoStop   (const OSystemEvent& event);
    virtual OStatus DoDestroy(const OSystemEvent& event);

    void Notify(const ONotifyEvent& event);
};

#endif // SampleObserver_h_DEFINED





