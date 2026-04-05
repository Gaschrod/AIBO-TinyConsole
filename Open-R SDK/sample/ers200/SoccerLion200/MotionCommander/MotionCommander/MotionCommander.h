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

#ifndef _MotionCommander_h_DEFINED
#define _MotionCommander_h_DEFINED

#include <OPENR/OObject.h>
#include <OPENR/OSubject.h> 
#include <OPENR/OObserver.h>
#include <OMWares/MoNetMessageMaker.h>
#include "def.h"

class MotionCommander : public OObject {
public:  
    MotionCommander();
    virtual ~MotionCommander() {}
  
    OSubject*    subject[numOfSubject];
    OObserver*   observer[numOfObserver];

    virtual OStatus DoInit(const OSystemEvent& event);
    virtual OStatus DoStart(const OSystemEvent& event);
    virtual OStatus DoStop(const OSystemEvent& event);
    virtual OStatus DoDestroy(const OSystemEvent& event);
    
    void readySetMotion(const OReadyEvent &event);
    void notifyMotionResult(const ONotifyEvent& event);

private:
    MoNetMessageMaker mmm;
    bool waitResult;
    bool haveNewMotion;

    void sendNewMotion();
    void help();
    bool readCommand();
};

#endif // _MotionCommander_h_DEFINED
