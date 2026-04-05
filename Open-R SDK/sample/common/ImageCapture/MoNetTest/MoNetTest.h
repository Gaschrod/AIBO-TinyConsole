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

#ifndef MoNetTest_h_DEFINED
#define MoNetTest_h_DEFINED

#include <OPENR/OObject.h>
#include <OPENR/OSubject.h>
#include <OPENR/OObserver.h>
#include <OPENR/ODataArchive.h>
#include <MoNetData.h>
#include "def.h"

#include <list>
using namespace std;

enum MoNetTestState {
    MNTS_IDLE,
    MNTS_START,
    MNTS_WAITING_RESULT,
    MNTS_SHUTDOWN
};

const MoNetCommandID SLEEP2SLEEP_NULL = 0; // see MONETCMD.CFG
const MoNetCommandID STAND2STAND_NULL = 2; // see MONETCMD.CFG

class MoNetTest : public OObject {
public:
    MoNetTest();
    virtual ~MoNetTest() {}

    OSubject*  subject[numOfSubject];
    OObserver* observer[numOfObserver];

    virtual OStatus DoInit   (const OSystemEvent& event);
    virtual OStatus DoStart  (const OSystemEvent& event);
    virtual OStatus DoStop   (const OSystemEvent& event);
    virtual OStatus DoDestroy(const OSystemEvent& event);

    void ReadyCommand(const OReadyEvent& event);
    void NotifyResult(const ONotifyEvent& event);

private:
    void ParseCommandLine(char* cmdline);
    void Execute(MoNetCommandID cmdID);

    MoNetTestState moNetTestState;
    list<MoNetCommandID> commandQueue;
};

#endif // MoNetTest_h_DEFINED
