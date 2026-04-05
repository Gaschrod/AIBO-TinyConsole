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

#include <OPENR/OPENRAPI.h>
#include <OPENR/OSyslog.h>
#include <OPENR/core_macro.h>
#include <MoNetData.h>
#include "MotionAgents.h"

MotionAgents::MotionAgents() : motionAgentsState(MAS_IDLE),
                               moNetAgentManager(), neutralAgent(), mtnAgent()
{
}

OStatus
MotionAgents::DoInit(const OSystemEvent& event)
{
    NEW_ALL_SUBJECT_AND_OBSERVER;
    REGISTER_ALL_ENTRY;
    SET_ALL_READY_AND_NOTIFY_ENTRY;

    char design[orobotdesignNAME_MAX+1];
    OStatus result = OPENR::GetRobotDesign(design);
    if (result != oSUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s %d",
                  "MotionAgents::DoInit()",
                  "OPENR::GetRobotDesign() FAILED", result));
    }

    if (!strcmp(design, "ERS-210") || !strcmp(design, "ERS-220")) {
        moNetAgentManager.RegisterMoNetAgent(&neutralAgent);
        moNetAgentManager.RegisterMoNetAgent(&mtnAgent);
        moNetAgentManager.InitDRX900(); // DRX-900 : Code name of ERS-210/220
    } else if (!strcmp(design, "ERS-7")) {
        moNetAgentManager.RegisterMoNetAgent(&neutralAgent7);
        moNetAgentManager.RegisterMoNetAgent(&mtnAgent7);
        moNetAgentManager.RegisterMoNetAgent(&mtnwalkAgent7);
        moNetAgentManager.InitDRX1000(); // DRX-1000 : Code name of ERS-7
    } else {
        OSYSLOG1((osyslogERROR,
                  "MotionAgents::DoInit() UNKNOWN ROBOT DESIGN"));
    }

    return oSUCCESS;
}

OStatus
MotionAgents::DoStart(const OSystemEvent& event)
{
    moNetAgentManager.Start(subject[sbjEffector]);
    motionAgentsState = MAS_START;

    ENABLE_ALL_SUBJECT;
    ASSERT_READY_TO_ALL_OBSERVER;

    return oSUCCESS;
}

OStatus
MotionAgents::DoStop(const OSystemEvent& event)
{
    motionAgentsState = MAS_IDLE;

    DISABLE_ALL_SUBJECT;
    DEASSERT_READY_TO_ALL_OBSERVER;

    return oSUCCESS;
}

OStatus
MotionAgents::DoDestroy(const OSystemEvent& event)
{
    DELETE_ALL_SUBJECT_AND_OBSERVER;
    return oSUCCESS;
}

void
MotionAgents::NotifyCommand(const ONotifyEvent& event)
{
    OSYSDEBUG(("MotionAgents::NotifyCommand()\n"));

    if (motionAgentsState == MAS_START) {

        MoNetAgentResult result;
        moNetAgentManager.NotifyCommand(event, &result);
        if (result.status != monetSUCCESS) {
            subject[sbjResult]->SetData(&result, sizeof(result));
            subject[sbjResult]->NotifyObservers();
        }

        observer[event.ObsIndex()]->AssertReady();
    }
}

void
MotionAgents::ReadyEffector(const OReadyEvent& event)
{
    OSYSDEBUG(("MotionAgents::ReadyEffector()\n"));

    if (motionAgentsState == MAS_START) {

        MoNetAgentResult result;
        moNetAgentManager.ReadyEffector(event, &result);
        if (result.status != monetSUCCESS) {
            subject[sbjResult]->SetData(&result, sizeof(result));
            subject[sbjResult]->NotifyObservers();
        }
    }
}
