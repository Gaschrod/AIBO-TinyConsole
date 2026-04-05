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
#include "MovingHead.h"

MovingHead::MovingHead() : movingHeadState(MHS_IDLE)
{
    for (int i = 0; i < NUM_JOINTS; i++) jointID[i] = oprimitiveID_UNDEF;
    for (int i = 0; i < NUM_COMMAND_VECTOR; i++) region[i] = 0;
}

OStatus
MovingHead::DoInit(const OSystemEvent& event)
{
    OSYSDEBUG(("MovingHead::DoInit()\n"));

    NEW_ALL_SUBJECT_AND_OBSERVER;
    REGISTER_ALL_ENTRY;
    SET_ALL_READY_AND_NOTIFY_ENTRY;

    OpenPrimitives();
    NewCommandVectorData();

    // 
    // OPENR::SetMotorPower(opowerON) is executed in blinkingLED.
    // So, it isn't necessary here.
    //
    
    return oSUCCESS;
}

OStatus
MovingHead::DoStart(const OSystemEvent& event)
{
    OSYSDEBUG(("MovingHead::DoStart()\n"));

    if (subject[sbjMove]->IsReady() == true) {
        AdjustDiffJointValue();
        movingHeadState = MHS_ADJUSTING_DIFF_JOINT_VALUE;
    } else {
        movingHeadState = MHS_START;
    }

    ENABLE_ALL_SUBJECT;
    ASSERT_READY_TO_ALL_OBSERVER;

    return oSUCCESS;
}

OStatus
MovingHead::DoStop(const OSystemEvent& event)
{
    OSYSDEBUG(("MovingHead::DoStop()\n"));

    movingHeadState = MHS_IDLE;

    DISABLE_ALL_SUBJECT;
    DEASSERT_READY_TO_ALL_OBSERVER;

    return oSUCCESS;
}

OStatus
MovingHead::DoDestroy(const OSystemEvent& event)
{
    DELETE_ALL_SUBJECT_AND_OBSERVER;
    return oSUCCESS;
}

void
MovingHead::Ready(const OReadyEvent& event)
{
    OSYSDEBUG(("MovingHead::Ready()\n"));

    if (movingHeadState == MHS_IDLE) {

        OSYSDEBUG(("MHS_IDLE\n"));
        ; // do nothing

    } else if (movingHeadState == MHS_START) {

        OSYSDEBUG(("MHS_START\n"));
        AdjustDiffJointValue();
        movingHeadState = MHS_ADJUSTING_DIFF_JOINT_VALUE;

    } else if (movingHeadState == MHS_ADJUSTING_DIFF_JOINT_VALUE) {

        OSYSDEBUG(("MHS_ADJUSTING_DIFF_JOINT_VALUE\n"));
        SetJointGain();
        MovingResult r = MoveToZeroPos();
        movingHeadState = MHS_MOVING_TO_ZERO_POS;

    } else if (movingHeadState == MHS_MOVING_TO_ZERO_POS) {

        OSYSDEBUG(("MHS_MOVING_TO_ZERO_POS\n"));
        MovingResult r = MoveToZeroPos();
        if (r == MOVING_FINISH) {
            movingHeadState = MHS_SWING_HEAD;
        }

    } else { // movingHeadState == MHS_SWING_HEAD

        OSYSDEBUG(("MHS_SWING_HEAD\n"));
        MovingResult r = SwingHead();
        if (r == MOVING_FINISH) {
            movingHeadState = MHS_IDLE;
        }
    }
}

void
MovingHead::OpenPrimitives()
{
    for (int i = 0; i < NUM_JOINTS; i++) {
        OStatus result = OPENR::OpenPrimitive(JOINT_LOCATOR[i], &jointID[i]);
        if (result != oSUCCESS) {
            OSYSLOG1((osyslogERROR, "%s : %s %d",
                      "MovingHead::OpenPrimitives()",
                      "OPENR::OpenPrimitive() FAILED", result));
        }
    }
}

void
MovingHead::NewCommandVectorData()
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
                      "MovingHead::NewCommandVectorData()",
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
MovingHead::SetJointGain()
{
    OPENR::EnableJointGain(jointID[TILT_INDEX]);
    OPENR::SetJointGain(jointID[TILT_INDEX],
                        TILT_PGAIN,
                        TILT_IGAIN,
                        TILT_DGAIN,
                        PSHIFT, ISHIFT, DSHIFT);
    
    OPENR::EnableJointGain(jointID[PAN_INDEX]);
    OPENR::SetJointGain(jointID[PAN_INDEX],
                        PAN_PGAIN,
                        PAN_IGAIN,
                        PAN_DGAIN,
                        PSHIFT, ISHIFT, DSHIFT);

    OPENR::EnableJointGain(jointID[ROLL_INDEX]);
    OPENR::SetJointGain(jointID[ROLL_INDEX],
                        ROLL_PGAIN,
                        ROLL_IGAIN,
                        ROLL_DGAIN,
                        PSHIFT, ISHIFT, DSHIFT);
}

MovingResult
MovingHead::AdjustDiffJointValue()
{
    OJointValue current[NUM_JOINTS];
    for (int i = 0; i < NUM_JOINTS; i++) {
        OPENR::GetJointValue(jointID[i], &current[i]);
        SetJointValue(region[0], i,
                      degrees(current[i].value/1000000.0),
                      degrees(current[i].value/1000000.0));
    }

    subject[sbjMove]->SetData(region[0]);
    subject[sbjMove]->NotifyObservers();

    return MOVING_FINISH;
}

MovingResult
MovingHead::MoveToZeroPos()
{
    static int counter = -1;
    static double s_tilt, d_tilt;
    static double s_pan,  d_pan;
    static double s_roll, d_roll;

    if (counter == -1) {

        OJointValue current;

        OPENR::GetJointValue(jointID[TILT_INDEX], &current);
        s_tilt = degrees(current.value/1000000.0);
        d_tilt = (0.0 - s_tilt) / (double)ZERO_POS_MAX_COUNTER;

        OPENR::GetJointValue(jointID[PAN_INDEX], &current);
        s_pan = degrees(current.value/1000000.0);
        d_pan = (0.0 - s_pan) / (double)ZERO_POS_MAX_COUNTER;

        OPENR::GetJointValue(jointID[ROLL_INDEX], &current);
        s_roll = degrees(current.value/1000000.0);
        d_roll = (0.0 - s_roll) / (double)ZERO_POS_MAX_COUNTER;

        counter = 0;

        RCRegion* rgn = FindFreeRegion();
        OSYSDEBUG(("FindFreeRegion() %x \n", rgn));
        SetJointValue(rgn, TILT_INDEX, s_tilt, s_tilt + d_tilt);
        SetJointValue(rgn, PAN_INDEX,  s_pan,  s_pan  + d_pan);
        SetJointValue(rgn, ROLL_INDEX, s_roll, s_roll + d_roll);
        subject[sbjMove]->SetData(rgn);

        s_tilt += d_tilt;
        s_pan  += d_pan;
        s_roll += d_roll;

        counter++;
    } 

    RCRegion* rgn = FindFreeRegion();
    OSYSDEBUG(("FindFreeRegion()%x \n", rgn));
    SetJointValue(rgn, TILT_INDEX, s_tilt, s_tilt + d_tilt);
    SetJointValue(rgn, PAN_INDEX,  s_pan,  s_pan  + d_pan);
    SetJointValue(rgn, ROLL_INDEX, s_roll, s_roll + d_roll);
    subject[sbjMove]->SetData(rgn);
    subject[sbjMove]->NotifyObservers();

    s_tilt += d_tilt;
    s_pan  += d_pan;
    s_roll += d_roll;

    counter++;
    return (counter == ZERO_POS_MAX_COUNTER) ? MOVING_FINISH : MOVING_CONT;
}

MovingResult
MovingHead::SwingHead()
{
    static int phase = 0; // [0:127]

    OSYSDEBUG(("MovingHead::SwingHead() phase %d\n", phase));

    RCRegion* rgn = FindFreeRegion();
    if (rgn == 0) {
        OSYSLOG1((osyslogERROR, "%s : %s",
                  "MovingHead::SwingHead()", "FindFreeRegion() FAILED"));
        return MOVING_FINISH;
    }

    SetJointValue(rgn, TILT_INDEX, 0.0, 0.0);
    SetJointValue(rgn, PAN_INDEX,  phase);
    SetJointValue(rgn, ROLL_INDEX, 0.0, 0.0);
    subject[sbjMove]->SetData(rgn);
    subject[sbjMove]->NotifyObservers();

    phase = (phase + 1) % MAX_PHASE;
    return MOVING_CONT;
}

RCRegion*
MovingHead::FindFreeRegion()
{
    for (int i = 0; i < NUM_COMMAND_VECTOR; i++) {
        if (region[i]->NumberOfReference() == 1) return region[i];
    }

    return 0;
}

void
MovingHead::SetJointValue(RCRegion* rgn, int idx, double start, double end)
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

void
MovingHead::SetJointValue(RCRegion* rgn, int idx, int phase)
{
    OCommandVectorData* cmdVecData = (OCommandVectorData*)rgn->Base();

    OCommandInfo* info = cmdVecData->GetInfo(idx);
    info->Set(odataJOINT_COMMAND2, jointID[idx], ocommandMAX_FRAMES);

    OCommandData* data = cmdVecData->GetData(idx);
    OJointCommandValue2* jval = (OJointCommandValue2*)data->value;

    double delta = M_PI / 64.0 / (double)ocommandMAX_FRAMES;
    for (int i = 0; i < ocommandMAX_FRAMES; i++) {
        double dval = 80.0 * sin(M_PI / 64.0 * phase + delta * i);
        jval[i].value = oradians(dval);
    }
}
