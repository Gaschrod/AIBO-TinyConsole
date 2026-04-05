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
#include "BallTrackingHead7.h"

BallTrackingHead7::BallTrackingHead7() : state(BTHS7_IDLE),
                                         fbkID(oprimitiveID_UNDEF),
                                         initSensorIndex(false),
                                         sensorRegions(),
                                         lastRefPan(0),
                                         lastRefTilt(0)
{
    for (int i = 0; i < NUM_JOINTS; i++) {
        jointID[i] = oprimitiveID_UNDEF;
        sensoridx[i] = -1;
    }
    for (int i = 0; i < NUM_COMMAND_VECTOR; i++) region[i] = 0;
}

OStatus
BallTrackingHead7::DoInit(const OSystemEvent& event)
{
    OSYSDEBUG(("BallTrackingHead7::DoInit()\n"));

    NEW_ALL_SUBJECT_AND_OBSERVER;
    REGISTER_ALL_ENTRY;
    SET_ALL_READY_AND_NOTIFY_ENTRY;

    OpenPrimitives();
    NewCommandVectorData();
    SetCdtVectorDataOfPinkBall();
    
    return oSUCCESS;
}

OStatus
BallTrackingHead7::DoStart(const OSystemEvent& event)
{
    OSYSDEBUG(("BallTrackingHead7::DoStart()\n"));

    Execute(STAND2STAND_NULL);
    state = BTHS7_WAITING_STAND2STAND_RESULT;

    ENABLE_ALL_SUBJECT;
    ASSERT_READY_TO_ALL_OBSERVER;

    return oSUCCESS;
}

OStatus
BallTrackingHead7::DoStop(const OSystemEvent& event)
{
    OSYSDEBUG(("BallTrackingHead7::DoStop()\n"));

    state = BTHS7_IDLE;

    DISABLE_ALL_SUBJECT;
    DEASSERT_READY_TO_ALL_OBSERVER;

    return oSUCCESS;
}

OStatus
BallTrackingHead7::DoDestroy(const OSystemEvent& event)
{
    DELETE_ALL_SUBJECT_AND_OBSERVER;
    return oSUCCESS;
}

void
BallTrackingHead7::NotifyResult(const ONotifyEvent& event)
{
    MoNetResult* result = (MoNetResult*)event.Data(0);
    if (result->status != monetCOMPLETION &&
        result->status != monetBUSY) {
        OSYSLOG1((osyslogERROR,
                  "%s : MoNetResult error [commandID %d,status %d]",
                  "BallTrackingHead7::NotifyResult()",
                  result->commandID, result->status));
    }
    
    if (state == BTHS7_IDLE) {
        
        ; // do nothing

    } else if (state == BTHS7_WAITING_STAND2STAND_RESULT) {

        SearchBall();
        state = BTHS7_SEARCHING_BALL;

    } else { // BTHS7_SEARCHING_BALL or BTHS7_TRACKING_BALL

        if (result->status == monetBUSY) {
            if (result->commandID == PLAY_FOUND_SOUND) {
                OSYSPRINT(("Execute(PLAY_FOUND_SOUND) monetBUSY\n"));
            } else if (result->commandID == PLAY_LOST_SOUND) {
                OSYSPRINT(("Execute(PLAY_LOST_SOUND) monetBUSY\n"));
            }
        }

    }

    observer[event.ObsIndex()]->AssertReady();
}

void
BallTrackingHead7::OpenPrimitives()
{
    OStatus result;

    for (int i = 0; i < NUM_JOINTS; i++) {
        result = OPENR::OpenPrimitive(JOINT_LOCATOR[i], &jointID[i]);
        if (result != oSUCCESS) {
            OSYSLOG1((osyslogERROR, "%s : %s %d",
                      "BallTrackingHead7::OpenPrimitives()",
                      "OPENR::OpenPrimitive() FAILED", result));
        }
    }

    result = OPENR::OpenPrimitive(FBK_LOCATOR, &fbkID);
    if (result != oSUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s %d",
                  "BallTrackingHead7::OpenPrimitives()",
                  "OPENR::OpenPrimitive() FAILED", result));
    }
}

void
BallTrackingHead7::NewCommandVectorData()
{
    OStatus result;
    MemoryRegionID      cmdVecDataID;
    OCommandVectorData* cmdVecData;
    OCommandInfo*       info;

    for (int i = 0; i < NUM_COMMAND_VECTOR; i++) {

        result = OPENR::NewCommandVectorData(NUM_JOINTS, 
                                             &cmdVecDataID, &cmdVecData);
        if (result != oSUCCESS) {
            OSYSLOG1((osyslogERROR, "%s : %s %d",
                      "BallTrackingHead7::NewCommandVectorData()",
                      "OPENR::NewCommandVectorData() FAILED", result));
        }

        region[i] = new RCRegion(cmdVecData->vectorInfo.memRegionID,
                                 cmdVecData->vectorInfo.offset,
                                 (void*)cmdVecData,
                                 cmdVecData->vectorInfo.totalSize);

        cmdVecData->SetNumData(NUM_JOINTS);

        for (int j = 0; j < NUM_JOINTS; j++) {
            info = cmdVecData->GetInfo(j);
            info->Set(odataJOINT_COMMAND2, jointID[j], NUM_FRAMES);
        }
    }
}

void
BallTrackingHead7::Execute(MoNetCommandID cmdID)
{
    MoNetCommand cmd(cmdID);
    subject[sbjCommand]->SetData(&cmd, sizeof(cmd));
    subject[sbjCommand]->NotifyObservers();
}
