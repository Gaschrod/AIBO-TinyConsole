//
// Copyright 2003 Sony Corporation 
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
#include "MovingHead7.h"

MovingHead7::MovingHead7() : movingHeadState(MHS_IDLE)
{
    for (int i = 0; i < NUM_JOINTS; i++) jointID[i] = oprimitiveID_UNDEF;
    for (int i = 0; i < NUM_COMMAND_VECTOR; i++) region[i] = 0;
}

OStatus
MovingHead7::DoInit(const OSystemEvent& event)
{
    OSYSDEBUG(("MovingHead7::DoInit()\n"));

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
MovingHead7::DoStart(const OSystemEvent& event)
{
    OSYSDEBUG(("MovingHead7::DoStart()\n"));

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
MovingHead7::DoStop(const OSystemEvent& event)
{
    OSYSDEBUG(("MovingHead7::DoStop()\n"));

    movingHeadState = MHS_IDLE;

    DISABLE_ALL_SUBJECT;
    DEASSERT_READY_TO_ALL_OBSERVER;

    return oSUCCESS;
}

OStatus
MovingHead7::DoDestroy(const OSystemEvent& event)
{
    DELETE_ALL_SUBJECT_AND_OBSERVER;
    return oSUCCESS;
}

void
MovingHead7::Ready(const OReadyEvent& event)
{
    OSYSDEBUG(("MovingHead7::Ready()\n"));

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
MovingHead7::OpenPrimitives()
{
    for (int i = 0; i < NUM_JOINTS; i++) {
        OStatus result = OPENR::OpenPrimitive(JOINT_LOCATOR[i], &jointID[i]);
        if (result != oSUCCESS) {
            OSYSLOG1((osyslogERROR, "%s : %s %d",
                      "MovingHead7::OpenPrimitives()",
                      "OPENR::OpenPrimitive() FAILED", result));
        }
    }
}

void
MovingHead7::NewCommandVectorData()
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
                      "MovingHead7::NewCommandVectorData()",
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
MovingHead7::SetJointGain()
{
    OPENR::EnableJointGain(jointID[TILT1_INDEX]);
    OPENR::SetJointGain(jointID[TILT1_INDEX],
                        TILT1_PGAIN,
                        TILT1_IGAIN,
                        TILT1_DGAIN,
                        PSHIFT, ISHIFT, DSHIFT);
    
    OPENR::EnableJointGain(jointID[PAN_INDEX]);
    OPENR::SetJointGain(jointID[PAN_INDEX],
                        PAN_PGAIN,
                        PAN_IGAIN,
                        PAN_DGAIN,
                        PSHIFT, ISHIFT, DSHIFT);

    OPENR::EnableJointGain(jointID[TILT2_INDEX]);
    OPENR::SetJointGain(jointID[TILT2_INDEX],
                        TILT2_PGAIN,
                        TILT2_IGAIN,
                        TILT2_DGAIN,
                        PSHIFT, ISHIFT, DSHIFT);
}

MovingResult
MovingHead7::AdjustDiffJointValue()
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
MovingHead7::MoveToZeroPos()
{
    static int counter = -1;
    static double s_tilt1, d_tilt1;
    static double s_pan,  d_pan;
    static double s_tilt2, d_tilt2;

    if (counter == -1) {

        OJointValue current;

        OPENR::GetJointValue(jointID[TILT1_INDEX], &current);
        s_tilt1 = degrees(current.value/1000000.0);
        d_tilt1 = (TILT1_ZERO_POS - s_tilt1) / (double)ZERO_POS_MAX_COUNTER;

        OPENR::GetJointValue(jointID[PAN_INDEX], &current);
        s_pan = degrees(current.value/1000000.0);
        d_pan = (PAN_ZERO_POS - s_pan) / (double)ZERO_POS_MAX_COUNTER;

        OPENR::GetJointValue(jointID[TILT2_INDEX], &current);
        s_tilt2 = degrees(current.value/1000000.0);
        d_tilt2 = (TILT2_ZERO_POS - s_tilt2) / (double)ZERO_POS_MAX_COUNTER;

        counter = 0;

        RCRegion* rgn = FindFreeRegion();
        OSYSDEBUG(("FindFreeRegion() %x \n", rgn));
        SetJointValue(rgn, TILT1_INDEX, s_tilt1, s_tilt1 + d_tilt1);
        SetJointValue(rgn, PAN_INDEX,   s_pan,   s_pan   + d_pan);
        SetJointValue(rgn, TILT2_INDEX, s_tilt2, s_tilt2 + d_tilt2);
        subject[sbjMove]->SetData(rgn);

        s_tilt1 += d_tilt1;
        s_pan   += d_pan;
        s_tilt2 += d_tilt2;

        counter++;
    } 

    RCRegion* rgn = FindFreeRegion();
    OSYSDEBUG(("FindFreeRegion()%x \n", rgn));
    SetJointValue(rgn, TILT1_INDEX, s_tilt1, s_tilt1 + d_tilt1);
    SetJointValue(rgn, PAN_INDEX,   s_pan,   s_pan   + d_pan);
    SetJointValue(rgn, TILT2_INDEX, s_tilt2, s_tilt2 + d_tilt2);
    subject[sbjMove]->SetData(rgn);
    subject[sbjMove]->NotifyObservers();

    s_tilt1 += d_tilt1;
    s_pan   += d_pan;
    s_tilt2 += d_tilt2;

    counter++;
    return (counter == ZERO_POS_MAX_COUNTER) ? MOVING_FINISH : MOVING_CONT;
}

MovingResult
MovingHead7::SwingHead()
{
    static int phase = 0; // [0:127]

    OSYSDEBUG(("MovingHead7::SwingHead() phase %d\n", phase));

    RCRegion* rgn = FindFreeRegion();
    if (rgn == 0) {
        OSYSLOG1((osyslogERROR, "%s : %s",
                  "MovingHead7::SwingHead()", "FindFreeRegion() FAILED"));
        return MOVING_FINISH;
    }

    SetJointValue(rgn, TILT1_INDEX, 0.0, 0.0);
    SetJointValue(rgn, PAN_INDEX,   phase);
    SetJointValue(rgn, TILT2_INDEX, 0.0, 0.0);
    subject[sbjMove]->SetData(rgn);
    subject[sbjMove]->NotifyObservers();

    phase = (phase + 1) % MAX_PHASE;
    return MOVING_CONT;
}

RCRegion*
MovingHead7::FindFreeRegion()
{
    for (int i = 0; i < NUM_COMMAND_VECTOR; i++) {
        if (region[i]->NumberOfReference() == 1) return region[i];
    }

    return 0;
}

void
MovingHead7::SetJointValue(RCRegion* rgn, int idx, double start, double end)
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
MovingHead7::SetJointValue(RCRegion* rgn, int idx, int phase)
{
    OCommandVectorData* cmdVecData = (OCommandVectorData*)rgn->Base();

    OCommandInfo* info = cmdVecData->GetInfo(idx);
    info->Set(odataJOINT_COMMAND2, jointID[idx], ocommandMAX_FRAMES);

    OCommandData* data = cmdVecData->GetData(idx);
    OJointCommandValue2* jval = (OJointCommandValue2*)data->value;

    double d1 = 2.0 * M_PI / (double)MAX_PHASE;
    double d2 = d1 / (double)ocommandMAX_FRAMES;
    for (int i = 0; i < ocommandMAX_FRAMES; i++) {
        double dval = SWING_AMPLITUDE * sin(d1 * phase + d2 * i);
        jval[i].value = oradians(dval);
    }
}
