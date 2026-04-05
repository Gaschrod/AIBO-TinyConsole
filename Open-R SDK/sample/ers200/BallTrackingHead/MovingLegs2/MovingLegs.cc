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

#include <math.h>
#include <OPENR/OPENRAPI.h>
#include <OPENR/OUnits.h>
#include <OPENR/OSyslog.h>
#include <OPENR/core_macro.h>
#include <BallTrackingHeadData.h>
#include "MovingLegs.h"

MovingLegs::MovingLegs() : movingLegsState(MLS_IDLE)
{
    for (int i = 0; i < NUM_JOINTS; i++) jointID[i] = oprimitiveID_UNDEF;
    for (int i = 0; i < NUM_COMMAND_VECTOR; i++) region[i] = 0;
}

OStatus
MovingLegs::DoInit(const OSystemEvent& event)
{
    OSYSDEBUG(("MovingLegs::DoInit()\n"));

    NEW_ALL_SUBJECT_AND_OBSERVER;
    REGISTER_ALL_ENTRY;
    SET_ALL_READY_AND_NOTIFY_ENTRY;

    OpenPrimitives();
    NewCommandVectorData();

    // 
    // OPENR::SetMotorPower(opowerON) is executed in lostFoundSound.
    // So, it isn't necessary here.
    //
    
    return oSUCCESS;
}

OStatus
MovingLegs::DoStart(const OSystemEvent& event)
{
    OSYSDEBUG(("MovingLegs::DoStart()\n"));

    ENABLE_ALL_SUBJECT;
    ASSERT_READY_TO_ALL_OBSERVER;

    return oSUCCESS;
}

OStatus
MovingLegs::DoStop(const OSystemEvent& event)
{
    OSYSDEBUG(("MovingLegs::DoStop()\n"));

    movingLegsState = MLS_IDLE;

    DISABLE_ALL_SUBJECT;
    DEASSERT_READY_TO_ALL_OBSERVER;

    return oSUCCESS;
}

OStatus
MovingLegs::DoDestroy(const OSystemEvent& event)
{
    DELETE_ALL_SUBJECT_AND_OBSERVER;
    return oSUCCESS;
}

void
MovingLegs::NotifyCommand(const ONotifyEvent& event)
{
    OSYSDEBUG(("MovingLegs::NotifyCommand()\n"));

    BallTrackingHeadCommand* cmd = (BallTrackingHeadCommand*)event.Data(0);
    if (cmd->type == BTHCMD_MOVE_TO_SLEEPING) {

        if (subject[sbjMove]->IsReady() == true) {
            AdjustDiffJointValue();
            movingLegsState = MLS_ADJUSTING_DIFF_JOINT_VALUE;
        } else {
            movingLegsState = MLS_START;
        }

    } else {

        BallTrackingHeadResult result(BTH_INVALID_ARG);
        subject[sbjResult]->SetData(&result, sizeof(result));
        subject[sbjResult]->NotifyObservers();

    }

    observer[event.ObsIndex()]->AssertReady();
}

void
MovingLegs::ReadyMove(const OReadyEvent& event)
{
    OSYSDEBUG(("MovingLegs::Ready()\n"));

    if (movingLegsState == MLS_IDLE) {

        OSYSDEBUG(("MLS_IDLE\n"));
        ; // do nothing

    } else if (movingLegsState == MLS_START) {

        OSYSDEBUG(("MLS_START\n"));
        AdjustDiffJointValue();
        movingLegsState = MLS_ADJUSTING_DIFF_JOINT_VALUE;

    } else if (movingLegsState == MLS_ADJUSTING_DIFF_JOINT_VALUE) {

        OSYSDEBUG(("MLS_ADJUSTING_DIFF_JOINT_VALUE\n"));
        SetJointGain();
        MovingResult r = MoveToBroadBase();
        movingLegsState = MLS_MOVING_TO_BROADBASE;

    } else if (movingLegsState == MLS_MOVING_TO_BROADBASE) {

        OSYSDEBUG(("MLS_MOVING_TO_BROADBASE\n"));
        MovingResult r = MoveToBroadBase();
        if (r == MOVING_FINISH) {
            movingLegsState = MLS_MOVING_TO_SLEEPING;
        }

    } else if (movingLegsState == MLS_MOVING_TO_SLEEPING) {

        OSYSDEBUG(("MLS_MOVING_TO_SLEEPING\n"));
        MovingResult r = MoveToSleeping();
        if (r == MOVING_FINISH) {
            BallTrackingHeadResult result(BTH_SUCCESS);
            subject[sbjResult]->SetData(&result, sizeof(result));
            subject[sbjResult]->NotifyObservers();
            movingLegsState = MLS_IDLE;
        }
    }
}

void
MovingLegs::OpenPrimitives()
{
    for (int i = 0; i < NUM_JOINTS; i++) {
        OStatus result = OPENR::OpenPrimitive(JOINT_LOCATOR[i], &jointID[i]);
        if (result != oSUCCESS) {
            OSYSLOG1((osyslogERROR, "%s : %s %d",
                      "MovingLegs::DoInit()",
                      "OPENR::OpenPrimitive() FAILED", result));
        }
    }
}

void
MovingLegs::NewCommandVectorData()
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
                      "MovingLegs::NewCommandVectorData()",
                      "OPENR::NewCommandVectorData() FAILED", result));
        }

        region[i] = new RCRegion(cmdVecData->vectorInfo.memRegionID,
                                 cmdVecData->vectorInfo.offset,
                                 (void*)cmdVecData,
                                 cmdVecData->vectorInfo.totalSize);

        cmdVecData->SetNumData(NUM_JOINTS);

        for (int j = 0; j < NUM_JOINTS; j++) {
            info = cmdVecData->GetInfo(j);
            info->Set(odataJOINT_COMMAND2, jointID[j], ocommandMAX_FRAMES);
        }
    }
}

void
MovingLegs::SetJointGain()
{
    for (int i = 0; i < 4; i++) {

        int j1 = 3 * i;
        int j2 = 3 * i + 1;
        int j3 = 3 * i + 2; 
            
        OPENR::EnableJointGain(jointID[j1]);        
        OPENR::SetJointGain(jointID[j1],
                            J1_PGAIN, J1_IGAIN, J1_DGAIN,
                            PSHIFT, ISHIFT, DSHIFT);

        OPENR::EnableJointGain(jointID[j2]);        
        OPENR::SetJointGain(jointID[j2],
                            J2_PGAIN, J2_IGAIN, J2_DGAIN,
                            PSHIFT, ISHIFT, DSHIFT);

        OPENR::EnableJointGain(jointID[j3]);        
        OPENR::SetJointGain(jointID[j3],
                            J3_PGAIN, J3_IGAIN, J3_DGAIN,
                            PSHIFT, ISHIFT, DSHIFT);
    }
}

MovingResult
MovingLegs::AdjustDiffJointValue()
{
    OJointValue current[NUM_JOINTS];

    for (int i = 0; i < NUM_JOINTS; i++) {
        OJointValue current;
        OPENR::GetJointValue(jointID[i], &current);
        SetJointValue(region[0], i,
                      degrees(current.value/1000000.0),
                      degrees(current.value/1000000.0));
    }

    subject[sbjMove]->SetData(region[0]);
    subject[sbjMove]->NotifyObservers();

    return MOVING_FINISH;
}

MovingResult
MovingLegs::MoveToBroadBase()
{
    static int counter = -1;
    static double start[NUM_JOINTS];
    static double delta[NUM_JOINTS];
    double ndiv = (double)BROADBASE_MAX_COUNTER;

    if (counter == -1) {

        for (int i = 0; i < NUM_JOINTS; i++) {
            OJointValue current;
            OPENR::GetJointValue(jointID[i], &current);
            start[i] = degrees(current.value/1000000.0);
            delta[i] = (BROADBASE_ANGLE[i] - start[i]) / ndiv;
        }

        counter = 0;

        RCRegion* rgn = FindFreeRegion();
        for (int i = 0; i < NUM_JOINTS; i++) {
            SetJointValue(rgn, i, start[i], start[i] + delta[i]);
            start[i] += delta[i];
        }

        subject[sbjMove]->SetData(rgn);
        counter ++;
    }

    RCRegion* rgn = FindFreeRegion();
    for (int i = 0; i < NUM_JOINTS; i++) {
        SetJointValue(rgn, i, start[i], start[i] + delta[i]);
        start[i] += delta[i];
    }

    subject[sbjMove]->SetData(rgn);
    subject[sbjMove]->NotifyObservers();

    counter++;
    return (counter == BROADBASE_MAX_COUNTER) ? MOVING_FINISH : MOVING_CONT;
}

MovingResult
MovingLegs::MoveToSleeping()
{
    static int counter = -1;
    static double start[NUM_JOINTS];
    static double delta[NUM_JOINTS];
    double ndiv = (double)SLEEPING_MAX_COUNTER;

    if (counter == -1) {

        for (int i = 0; i < NUM_JOINTS; i++) {
            start[i] = BROADBASE_ANGLE[i];
            delta[i] = (SLEEPING_ANGLE[i] - start[i]) / ndiv;
        }

        counter = 0;
    }

    RCRegion* rgn = FindFreeRegion();
    for (int i = 0; i < NUM_JOINTS; i++) {
        SetJointValue(rgn, i, start[i], start[i] + delta[i]);
        start[i] += delta[i];
    }

    subject[sbjMove]->SetData(rgn);
    subject[sbjMove]->NotifyObservers();

    counter++;
    return (counter == SLEEPING_MAX_COUNTER) ? MOVING_FINISH : MOVING_CONT;
}

RCRegion*
MovingLegs::FindFreeRegion()
{
    for (int i = 0; i < NUM_COMMAND_VECTOR; i++) {
        if (region[i]->NumberOfReference() == 1) return region[i];
    }

    return 0;
}

void
MovingLegs::SetJointValue(RCRegion* rgn, int idx, double start, double end)
{
    OCommandVectorData* cmdVecData = (OCommandVectorData*)rgn->Base();

    OCommandInfo* info = cmdVecData->GetInfo(idx);
    info->Set(odataJOINT_COMMAND2, jointID[idx], ocommandMAX_FRAMES);

    OCommandData* data = cmdVecData->GetData(idx);
    OJointCommandValue2* jval = (OJointCommandValue2*)data->value;

    double delta = end - start;
    for (int i = 0; i < ocommandMAX_FRAMES; i++) {
        double dval = start + (delta * i) / (double)ocommandMAX_FRAMES;
        jval[i].value = oradians(dval);
    }
}
