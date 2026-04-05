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
#include "BallTrackingHead.h"

BallTrackingHead::BallTrackingHead() : ballTrackingHeadState(BTHS_IDLE),
                                       fbkID(oprimitiveID_UNDEF),
                                       initSensorIndex(false),
                                       sensorRegions(),
                                       lastRefPan(0),
                                       lastRefTilt(0)
{
    for (int i = 0; i < NUM_JOINTS; i++) jointID[i] = oprimitiveID_UNDEF;
    for (int i = 0; i < NUM_COMMAND_VECTOR; i++) region[i] = 0;
    for (int i = 0; i < NUM_JOINTS; i++) sensoridx[i] = -1;
}

OStatus
BallTrackingHead::DoInit(const OSystemEvent& event)
{
    OSYSDEBUG(("BallTrackingHead::DoInit()\n"));

    NEW_ALL_SUBJECT_AND_OBSERVER;
    REGISTER_ALL_ENTRY;
    SET_ALL_READY_AND_NOTIFY_ENTRY;

    OpenPrimitives();
    NewCommandVectorData();
    SetCdtVectorDataOfPinkBall();
    
    return oSUCCESS;
}

OStatus
BallTrackingHead::DoStart(const OSystemEvent& event)
{
    OSYSDEBUG(("BallTrackingHead::DoStart()\n"));

    BallTrackingHeadCommand headcmd(BTHCMD_MOVE_TO_ZERO_POS);
    subject[sbjMovingHead]->SetData(&headcmd, sizeof(headcmd));
    subject[sbjMovingHead]->NotifyObservers();

    BallTrackingHeadCommand legscmd(BTHCMD_MOVE_TO_SLEEPING);
    subject[sbjMovingLegs]->SetData(&legscmd, sizeof(legscmd));
    subject[sbjMovingLegs]->NotifyObservers();

    ballTrackingHeadState = BTHS_START;

    ENABLE_ALL_SUBJECT;
    ASSERT_READY_TO_ALL_OBSERVER;

    return oSUCCESS;
}

OStatus
BallTrackingHead::DoStop(const OSystemEvent& event)
{
    OSYSDEBUG(("BallTrackingHead::DoStop()\n"));

    ballTrackingHeadState = BTHS_IDLE;

    DISABLE_ALL_SUBJECT;
    DEASSERT_READY_TO_ALL_OBSERVER;

    return oSUCCESS;
}

OStatus
BallTrackingHead::DoDestroy(const OSystemEvent& event)
{
    DELETE_ALL_SUBJECT_AND_OBSERVER;
    return oSUCCESS;
}

void
BallTrackingHead::NotifyMovingHeadResult(const ONotifyEvent& event)
{
    OSYSDEBUG(("BallTrackingHead::NotifyMovingHeadResult()\n"));

    if (ballTrackingHeadState == BTHS_IDLE) return; // do nothing

    BallTrackingHeadResult* result = (BallTrackingHeadResult*)event.Data(0);
    if (result->status == BTH_SUCCESS) {
        
        if (ballTrackingHeadState == BTHS_START) {
            ballTrackingHeadState = BTHS_HEAD_ZERO_POS;
        } else if (ballTrackingHeadState == BTHS_LEGS_SLEEPING) {
            SearchBall();
            ballTrackingHeadState = BTHS_SEARCHING_BALL;
        }

    } else {

        OSYSLOG1((osyslogERROR, "%s : %s %d",
                  "BallTrackingHead::NotifyMovingHeadResult()", 
                  "FAILED. result->status", result->status));

    }

    observer[event.ObsIndex()]->AssertReady();
}

void
BallTrackingHead::NotifyMovingLegsResult(const ONotifyEvent& event)
{
    OSYSDEBUG(("BallTrackingHead::NotifyMovingLegsResult()\n"));

    if (ballTrackingHeadState == BTHS_IDLE) return; // do nothing

    BallTrackingHeadResult* result = (BallTrackingHeadResult*)event.Data(0);
    if (result->status == BTH_SUCCESS) {
        
        if (ballTrackingHeadState == BTHS_START) {
            ballTrackingHeadState = BTHS_LEGS_SLEEPING;
        } else if (ballTrackingHeadState == BTHS_HEAD_ZERO_POS) {
            SearchBall();
            ballTrackingHeadState = BTHS_SEARCHING_BALL;
        }

    } else {

        OSYSLOG1((osyslogERROR, "%s : %s %d",
                  "BallTrackingHead::NotifyMovingLegsResult()", 
                  "FAILED. result->status", result->status));

    }

    observer[event.ObsIndex()]->AssertReady();
}

void
BallTrackingHead::NotifyLostFoundSoundResult(const ONotifyEvent& event)
{
    OSYSDEBUG(("BallTrackingHead::NotifyLostFoundSoundResult()\n"));

    if (ballTrackingHeadState == BTHS_IDLE) return; // do nothing

    BallTrackingHeadResult* result = (BallTrackingHeadResult*)event.Data(0);
    if (result->status != BTH_SUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s %d",
                  "BallTrackingHead::NotifyLostFoundSoundResult()", 
                  "FAILED. result->status", result->status));
    }

    observer[event.ObsIndex()]->AssertReady();
}

void
BallTrackingHead::OpenPrimitives()
{
    OStatus result;

    for (int i = 0; i < NUM_JOINTS; i++) {
        result = OPENR::OpenPrimitive(JOINT_LOCATOR[i], &jointID[i]);
        if (result != oSUCCESS) {
            OSYSLOG1((osyslogERROR, "%s : %s %d",
                      "BallTrackingHead::OpenPrimitives()",
                      "OPENR::OpenPrimitive() FAILED", result));
        }
    }

    result = OPENR::OpenPrimitive(FBK_LOCATOR, &fbkID);
    if (result != oSUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s %d",
                  "BallTrackingHead::OpenPrimitives()",
                  "OPENR::OpenPrimitive() FAILED", result));
    }
}

void
BallTrackingHead::NewCommandVectorData()
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
                      "BallTrackingHead::NewCommandVectorData()",
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
BallTrackingHead::PlaySound(BallTrackingHeadCommandType type)
{
    OSYSDEBUG(("BallTrackingHead::PlaySound()\n"));
    BallTrackingHeadCommand cmd(type);
    subject[sbjLostFoundSound]->SetData(&cmd, sizeof(cmd));
    subject[sbjLostFoundSound]->NotifyObservers();
}
