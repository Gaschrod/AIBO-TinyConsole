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
#include "LMasterRSlave.h"

LMasterRSlave::LMasterRSlave() : lmasterRSlaveState(LMRS_IDLE),
                                 initSensorIndex(false), latestSensorRegion(0)
{
    for (int i = 0; i < NUM_JOINTS; i++) jointID[i] = oprimitiveID_UNDEF;
    for (int i = 0; i < NUM_COMMAND_VECTOR; i++) region[i] = 0;
    for (int i = 0; i < NUM_JOINTS; i++) sensorIndex[i] = -1;
}

OStatus
LMasterRSlave::DoInit(const OSystemEvent& event)
{
    OSYSDEBUG(("LMasterRSlave::DoInit()\n"));

    NEW_ALL_SUBJECT_AND_OBSERVER;
    REGISTER_ALL_ENTRY;
    SET_ALL_READY_AND_NOTIFY_ENTRY;

    OpenPrimitives();
    NewCommandVectorData();
    OPENR::SetMotorPower(opowerON);
    
    return oSUCCESS;
}

OStatus
LMasterRSlave::DoStart(const OSystemEvent& event)
{
    OSYSDEBUG(("LMasterRSlave::DoStart()\n"));

    if (subject[sbjEffector]->IsReady() == true) {
        AdjustDiffJointValue();
        lmasterRSlaveState = LMRS_ADJUSTING_DIFF_JOINT_VALUE;
    } else {
        lmasterRSlaveState = LMRS_START;
    }

    ENABLE_ALL_SUBJECT;
    ASSERT_READY_TO_ALL_OBSERVER;

    return oSUCCESS;
}

OStatus
LMasterRSlave::DoStop(const OSystemEvent& event)
{
    OSYSDEBUG(("LMasterRSlave::DoStop()\n"));

    lmasterRSlaveState = LMRS_IDLE;

    DISABLE_ALL_SUBJECT;
    DEASSERT_READY_TO_ALL_OBSERVER;

    return oSUCCESS;
}

OStatus
LMasterRSlave::DoDestroy(const OSystemEvent& event)
{
    DELETE_ALL_SUBJECT_AND_OBSERVER;
    return oSUCCESS;
}

void
LMasterRSlave::ReadyEffector(const OReadyEvent& event)
{
    OSYSDEBUG(("LMasterRSlave::Ready()\n"));

    if (lmasterRSlaveState == LMRS_IDLE) {

        OSYSDEBUG(("LMRS_IDLE\n"));
        ; // do nothing

    } else if (lmasterRSlaveState == LMRS_START) {

        OSYSDEBUG(("LMRS_START\n"));
        AdjustDiffJointValue();
        lmasterRSlaveState = LMRS_ADJUSTING_DIFF_JOINT_VALUE;

    } else if (lmasterRSlaveState == LMRS_ADJUSTING_DIFF_JOINT_VALUE) {

        OSYSDEBUG(("LMRS_ADJUSTING_DIFF_JOINT_VALUE\n"));
        SetJointGain();
        MovingResult r = MoveToBroadBase();
        lmasterRSlaveState = LMRS_MOVING_TO_BROADBASE;

    } else if (lmasterRSlaveState == LMRS_MOVING_TO_BROADBASE) {

        OSYSDEBUG(("LMRS_MOVING_TO_BROADBASE\n"));
        MovingResult r = MoveToBroadBase();
        if (r == MOVING_FINISH) {
            lmasterRSlaveState = LMRS_MOVING_TO_SLEEPING;
        }

    } else if (lmasterRSlaveState == LMRS_MOVING_TO_SLEEPING) {

        OSYSDEBUG(("LMRS_MOVING_TO_SLEEPING\n"));
        MovingResult r = MoveToSleeping();
        if (r == MOVING_FINISH) {
            lmasterRSlaveState = LMRS_MASTER_SLAVE;
        }

    } else if (lmasterRSlaveState == LMRS_MASTER_SLAVE) {

        OSYSDEBUG(("LMRS_MASTER_SLAVE\n"));
        MasterSlave();
        
    }
}

void
LMasterRSlave::NotifySensor(const ONotifyEvent& event)
{
    if (lmasterRSlaveState != LMRS_IDLE) {
        
        RCRegion* rgn = event.RCData(0);
        rgn->AddReference();

        if (initSensorIndex == false) {
            OSensorFrameVectorData* sensorVec
                = (OSensorFrameVectorData*)rgn->Base();
            InitSensorIndex(sensorVec);
            initSensorIndex = true;
        }

        if (latestSensorRegion != 0) latestSensorRegion->RemoveReference();
        latestSensorRegion = rgn;

        observer[event.ObsIndex()]->AssertReady();
    }
}

void
LMasterRSlave::OpenPrimitives()
{
    for (int i = 0; i < NUM_JOINTS; i++) {
        OStatus result = OPENR::OpenPrimitive(JOINT_LOCATOR[i], &jointID[i]);
        if (result != oSUCCESS) {
            OSYSLOG1((osyslogERROR, "%s : %s %d",
                      "LMasterRSlave::DoInit()",
                      "OPENR::OpenPrimitive() FAILED", result));
        }
    }
}

void
LMasterRSlave::NewCommandVectorData()
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
                      "LMasterRSlave::NewCommandVectorData()",
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
LMasterRSlave::SetJointGain()
{
    OPENR::EnableJointGain(jointID[HEAD_TILT]);
    OPENR::SetJointGain(jointID[HEAD_TILT],
                        TILT_PGAIN, TILT_IGAIN, TILT_DGAIN,
                        PSHIFT, ISHIFT, DSHIFT);
    
    OPENR::EnableJointGain(jointID[HEAD_PAN]);
    OPENR::SetJointGain(jointID[HEAD_PAN],
                        PAN_PGAIN, PAN_IGAIN, PAN_DGAIN,
                        PSHIFT, ISHIFT, DSHIFT);

    OPENR::EnableJointGain(jointID[HEAD_ROLL]);
    OPENR::SetJointGain(jointID[HEAD_ROLL],
                        ROLL_PGAIN, ROLL_IGAIN, ROLL_DGAIN,
                        PSHIFT, ISHIFT, DSHIFT);

    int base = RFLEG_J1;
    for (int i = 0; i < 4; i++) {

        int j1 = base + 3 * i;
        int j2 = base + 3 * i + 1;
        int j3 = base + 3 * i + 2; 
            
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
LMasterRSlave::AdjustDiffJointValue()
{
    OJointValue current[NUM_JOINTS];

    for (int i = 0; i < NUM_JOINTS; i++) {
        OJointValue current;
        OPENR::GetJointValue(jointID[i], &current);
        SetJointValue(region[0], i,
                      degrees(current.value/1000000.0),
                      degrees(current.value/1000000.0));
    }

    subject[sbjEffector]->SetData(region[0]);
    subject[sbjEffector]->NotifyObservers();

    return MOVING_FINISH;
}

MovingResult
LMasterRSlave::MoveToBroadBase()
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

        subject[sbjEffector]->SetData(rgn);
        counter ++;
    }

    RCRegion* rgn = FindFreeRegion();
    for (int i = 0; i < NUM_JOINTS; i++) {
        SetJointValue(rgn, i, start[i], start[i] + delta[i]);
        start[i] += delta[i];
    }

    subject[sbjEffector]->SetData(rgn);
    subject[sbjEffector]->NotifyObservers();

    counter++;
    return (counter == BROADBASE_MAX_COUNTER) ? MOVING_FINISH : MOVING_CONT;
}

MovingResult
LMasterRSlave::MoveToSleeping()
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

    subject[sbjEffector]->SetData(rgn);
    subject[sbjEffector]->NotifyObservers();

    counter++;
    return (counter == SLEEPING_MAX_COUNTER) ? MOVING_FINISH : MOVING_CONT;
}

MovingResult
LMasterRSlave::MasterSlave()
{
    OSYSDEBUG(("LMasterRSlave::MasterSlave()\n"));

    static bool isFirstTime = true;

    if (isFirstTime == true) {
        OPENR::DisableJointGain(jointID[LFLEG_J1]);
        OPENR::DisableJointGain(jointID[LFLEG_J2]);
        OPENR::DisableJointGain(jointID[LFLEG_J3]);
        OPENR::DisableJointGain(jointID[LRLEG_J1]);
        OPENR::DisableJointGain(jointID[LRLEG_J2]);
        OPENR::DisableJointGain(jointID[LRLEG_J3]);
        isFirstTime = false;
    }

    RCRegion* rgn = FindFreeRegion();
    OCommandVectorData* commandVec = (OCommandVectorData*)rgn->Base();
    OSensorFrameVectorData* sensorVec
        = (OSensorFrameVectorData*)latestSensorRegion->Base();
    
    int cmdIdx = 0;

    Sensor2Command(sensorVec, sensorIndex[LFLEG_J1],
                   commandVec, cmdIdx++, jointID[RFLEG_J1]);

    Sensor2Command(sensorVec, sensorIndex[LFLEG_J2],
                   commandVec, cmdIdx++, jointID[RFLEG_J2]);

    Sensor2Command(sensorVec, sensorIndex[LFLEG_J3],
                   commandVec, cmdIdx++, jointID[RFLEG_J3]);

    Sensor2Command(sensorVec, sensorIndex[LRLEG_J1],
                   commandVec, cmdIdx++, jointID[RRLEG_J1]);

    Sensor2Command(sensorVec, sensorIndex[LRLEG_J2],
                   commandVec, cmdIdx++, jointID[RRLEG_J2]);

    Sensor2Command(sensorVec, sensorIndex[LRLEG_J3],
                   commandVec, cmdIdx++, jointID[RRLEG_J3]);

    commandVec->SetNumData(cmdIdx);

    subject[sbjEffector]->SetData(rgn);
    subject[sbjEffector]->NotifyObservers();

    return MOVING_CONT;
}

RCRegion*
LMasterRSlave::FindFreeRegion()
{
    for (int i = 0; i < NUM_COMMAND_VECTOR; i++) {
        if (region[i]->NumberOfReference() == 1) return region[i];
    }

    return 0;
}

void
LMasterRSlave::SetJointValue(RCRegion* rgn, int idx, double start, double end)
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
LMasterRSlave::InitSensorIndex(OSensorFrameVectorData* sensorVec)
{
    for (int i = 0; i < NUM_JOINTS; i++) {
        for (int j = 0; j < sensorVec->vectorInfo.numData; j++) {
            OSensorFrameInfo* info = sensorVec->GetInfo(j);
            if (info->primitiveID == jointID[i]) {
                sensorIndex[i] = j;
                OSYSDEBUG(("[%2d] %s\n", sensorIndex[i], JOINT_LOCATOR[i]));
                break;
            }
        }
    }
}

void
LMasterRSlave::Sensor2Command(OSensorFrameVectorData* sensorVec, int sensorIdx,
                              OCommandVectorData* commandVec, int commandIndex,
                              OPrimitiveID id)
{
    OSensorFrameInfo* sinfo = sensorVec->GetInfo(sensorIdx);
    OSensorFrameData* sdata = sensorVec->GetData(sensorIdx);

    OCommandInfo* cinfo = commandVec->GetInfo(commandIndex);
    OCommandData* cdata = commandVec->GetData(commandIndex);

    for (int i = 0; i < sinfo->numFrames; i++) {
        OJointCommandValue2* jval = (OJointCommandValue2*)cdata->value;
        jval[i].value = sdata->frame[i].value;
    }

    cinfo->Set(odataJOINT_COMMAND2, id, sinfo->numFrames);
}
