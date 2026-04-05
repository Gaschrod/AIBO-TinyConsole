//
// Copyright 2002,2004 Sony Corporation 
//
// Permission to use, copy, modify, and redistribute this software for
// non-commercial use is hereby granted.
//
// This software is provided "as is" without warranty of any kind,
// either expressed or implied, including but not limited to the
// implied warranties of fitness for a particular purpose.
//

#ifndef MotionAgents_h_DEFINED
#define MotionAgents_h_DEFINED

#include <OPENR/OObject.h>
#include <OPENR/OSubject.h>
#include <OPENR/OObserver.h>
#include "def.h"
#include "MoNetAgentManager.h"
#include "NeutralAgent.h"
#include "MTNAgent.h"
#include "NeutralAgent7.h"
#include "MTNAgent7.h"
#include "MTNWALKAgent7.h"

enum MotionAgentsState {
    MAS_IDLE,
    MAS_START
};

class MotionAgents : public OObject {
public:
    MotionAgents();
    virtual ~MotionAgents() {}

    OSubject*  subject[numOfSubject];
    OObserver* observer[numOfObserver];

    virtual OStatus DoInit   (const OSystemEvent& event);
    virtual OStatus DoStart  (const OSystemEvent& event);
    virtual OStatus DoStop   (const OSystemEvent& event);
    virtual OStatus DoDestroy(const OSystemEvent& event);
    
    void NotifyCommand(const ONotifyEvent& event);
    void ReadyEffector(const OReadyEvent& event);

private:
    MotionAgentsState motionAgentsState;
    MoNetAgentManager moNetAgentManager;
    NeutralAgent      neutralAgent;
    MTNAgent          mtnAgent;
    NeutralAgent7     neutralAgent7;
    MTNAgent7         mtnAgent7;
    MTNWALKAgent7     mtnwalkAgent7;
};

#endif // MotionAgents_h_DEFINED
