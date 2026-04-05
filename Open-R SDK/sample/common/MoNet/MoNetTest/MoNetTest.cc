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

#include <string.h>
#include <OPENR/OPENRAPI.h>
#include <OPENR/OSyslog.h>
#include <OPENR/core_macro.h>
#include "MoNetTest.h"

MoNetTest::MoNetTest() : moNetTestState(MNTS_IDLE), commandQueue()
{
}

OStatus
MoNetTest::DoInit(const OSystemEvent& event)
{
    NEW_ALL_SUBJECT_AND_OBSERVER;
    REGISTER_ALL_ENTRY;
    SET_ALL_READY_AND_NOTIFY_ENTRY;

    return oSUCCESS;
}

OStatus
MoNetTest::DoStart(const OSystemEvent& event)
{
    if (subject[sbjCommand]->IsReady() == true) {
        Execute(SLEEP2SLEEP_NULL);
        moNetTestState = MNTS_WAITING_RESULT;
    } else {
        moNetTestState = MNTS_START;
    }

    ENABLE_ALL_SUBJECT;
    ASSERT_READY_TO_ALL_OBSERVER;

    return oSUCCESS;
}

OStatus
MoNetTest::DoStop(const OSystemEvent& event)
{
    moNetTestState = MNTS_IDLE;

    DISABLE_ALL_SUBJECT;
    DEASSERT_READY_TO_ALL_OBSERVER;

    return oSUCCESS;
}

OStatus
MoNetTest::DoDestroy(const OSystemEvent& event)
{
    DELETE_ALL_SUBJECT_AND_OBSERVER;
    return oSUCCESS;
}

void
MoNetTest::ReadyCommand(const OReadyEvent& event)
{
    if (moNetTestState == MNTS_START) {
        Execute(SLEEP2SLEEP_NULL);
        moNetTestState = MNTS_WAITING_RESULT;
    }
}

void
MoNetTest::NotifyResult(const ONotifyEvent& event)
{
    if (moNetTestState == MNTS_IDLE) {

        ; // do nothing

    } else if (moNetTestState == MNTS_START) {

        OSYSLOG1((osyslogERROR,
                  "MoNetTest::NotifyResult() : moNetTestState ERROR\n"));
        
        observer[event.ObsIndex()]->AssertReady();

    } else if (moNetTestState == MNTS_WAITING_RESULT) {

        MoNetResult* result = (MoNetResult*)event.Data(0);
        OSYSPRINT(("MONET RESULT : commandID %d status %d posture %d\n",
                   result->commandID, result->status, result->posture));
        
        if (commandQueue.size() > 0) {

            Execute(commandQueue.front());
            commandQueue.pop_front();

        } else {

            char cmdline[256];
            do {
                OSYSPRINT(("MoNetTest> "));
                gets(cmdline);
            } while (strlen(cmdline) == 0);

            if (strcmp(cmdline, "shutdown") == 0) {
                Execute(SLEEP2SLEEP_NULL);
                moNetTestState = MNTS_SHUTDOWN;
            } else {
                ParseCommandLine(cmdline);
                Execute(commandQueue.front());
                commandQueue.pop_front();
            }
        }

        observer[event.ObsIndex()]->AssertReady();

    } else if (moNetTestState == MNTS_SHUTDOWN) {

        OSYSPRINT(("SHUTDOWN ...\n"));
        OBootCondition bootCond(obcbPAUSE_SW);
        OPENR::Shutdown(bootCond);
    }
}

void
MoNetTest::Execute(MoNetCommandID cmdID)
{
    MoNetCommand cmd(cmdID);
    subject[sbjCommand]->SetData(&cmd, sizeof(cmd));
    subject[sbjCommand]->NotifyObservers();
}

void
MoNetTest::ParseCommandLine(char* cmdline)
{
    char* ptr= strtok(cmdline, " \t");
    if (ptr == 0) return;
    commandQueue.push_back(atoi(ptr));
    
    while ((ptr = strtok(0, " \t")) != 0) {
        commandQueue.push_back(atoi(ptr));
    }
}
