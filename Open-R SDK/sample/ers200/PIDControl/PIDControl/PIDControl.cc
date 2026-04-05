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
#include <OPENR/OUnits.h>
#include <OPENR/OSyslog.h>
#include <OPENR/core_macro.h>
#include "PIDControl.h"

PIDControl::PIDControl() : pidControlState(PCS_IDLE), pidControlInfo(),
                           initSensorIndex(false)
{
    for (int i = 0; i < NUM_JOINTS; i++) jointID[i] = oprimitiveID_UNDEF;
    for (int i = 0; i < NUM_COMMAND_VECTOR; i++) region[i] = 0;
    for (int i = 0; i < NUM_JOINTS; i++) sensorIndex[i] = -1;
}

OStatus
PIDControl::DoInit(const OSystemEvent& event)
{
    OSYSDEBUG(("PIDControl::DoInit()\n"));

    NEW_ALL_SUBJECT_AND_OBSERVER;
    REGISTER_ALL_ENTRY;
    SET_ALL_READY_AND_NOTIFY_ENTRY;

    OpenPrimitives();
    NewCommandVectorData();
    OPENR::SetMotorPower(opowerON);

    size_t memsize = HOLD_TIME_NUM_FRAMES*sizeof(OJointValue);
    logData = (OJointValue*)malloc(memsize);
    if (logData == 0) {
        OSYSLOG1((osyslogERROR, "PIDControl::DoInit() : malloc() failed."));
    }
    
    return oSUCCESS;
}

OStatus
PIDControl::DoStart(const OSystemEvent& event)
{
    OSYSDEBUG(("PIDControl::DoStart()\n"));

    if (subject[sbjEffector]->IsReady() == true) {
        AdjustDiffJointValue();
        pidControlState = PCS_ADJUSTING_DIFF_JOINT_VALUE;
    } else {
        pidControlState = PCS_START;
    }

    ENABLE_ALL_SUBJECT;
    ASSERT_READY_TO_ALL_OBSERVER;

    return oSUCCESS;
}

OStatus
PIDControl::DoStop(const OSystemEvent& event)
{
    OSYSDEBUG(("PIDControl::DoStop()\n"));

    pidControlState = PCS_IDLE;

    DISABLE_ALL_SUBJECT;
    DEASSERT_READY_TO_ALL_OBSERVER;

    return oSUCCESS;
}

OStatus
PIDControl::DoDestroy(const OSystemEvent& event)
{
    DELETE_ALL_SUBJECT_AND_OBSERVER;
    return oSUCCESS;
}

void
PIDControl::ReadyEffector(const OReadyEvent& event)
{
    OSYSDEBUG(("PIDControl::Ready()\n"));

    if (pidControlState == PCS_IDLE) {

        OSYSDEBUG(("PCS_IDLE\n"));
        ; // do nothing

    } else if (pidControlState == PCS_START) {

        OSYSDEBUG(("PCS_START\n"));
        AdjustDiffJointValue();
        pidControlState = PCS_ADJUSTING_DIFF_JOINT_VALUE;

    } else if (pidControlState == PCS_ADJUSTING_DIFF_JOINT_VALUE) {

        OSYSDEBUG(("PCS_ADJUSTING_DIFF_JOINT_VALUE\n"));
        SetJointGain();
        MovingResult r = MoveToBroadBase();
        pidControlState = PCS_MOVING_TO_BROADBASE;

    } else if (pidControlState == PCS_MOVING_TO_BROADBASE) {

        OSYSDEBUG(("PCS_MOVING_TO_BROADBASE\n"));
        MovingResult r = MoveToBroadBase();
        if (r == MOVING_FINISH) {
            pidControlState = PCS_MOVING_TO_SLEEPING;
        }

    } else if (pidControlState == PCS_MOVING_TO_SLEEPING) {

        OSYSDEBUG(("PCS_MOVING_TO_SLEEPING\n"));
        MovingResult r = MoveToSleeping();
        if (r == MOVING_FINISH) {
            pidControlState = PCS_DISABLE_JOINT_GAIN;
        }

    } else if (pidControlState == PCS_DISABLE_JOINT_GAIN) {

        OSYSDEBUG(("PCS_DISABLE_JOINT_GAIN\n"));
        DisableJointGainOfAllLegs();       
        if (YesNo("PIDControl continue? (y/n) > ") != true) {
            OBootCondition bootCond(obcbPAUSE_SW);
            OPENR::Shutdown(bootCond);
            pidControlState = PCS_IDLE;
            return;
        }
        AdjustDiffJointValue();
        pidControlState = PCS_INPUT_PARAMETER;
        
    } else if (pidControlState == PCS_INPUT_PARAMETER) {

        OSYSDEBUG(("PCS_INPUT_PARAMETER\n"));
        SetJointGain();
        InputParameter();
        observer[obsSensor]->AssertReady();
        StepInput();
        pidControlState = PCS_STEP_INPUT;

    } else if (pidControlState == PCS_STEP_INPUT) {

        OSYSDEBUG(("PCS_INPUT_PARAMETER\n"));
        MovingResult r = StepInput();
        if (r == MOVING_FINISH) {
            SaveLogData();
            pidControlState = PCS_DISABLE_JOINT_GAIN;
        }
    }
}

void
PIDControl::NotifySensor(const ONotifyEvent& event)
{
    OSensorFrameVectorData* sensorVec;

    if (initSensorIndex == false) {
        sensorVec = (OSensorFrameVectorData*)event.Data(0);
        InitSensorIndex(sensorVec);
        initSensorIndex = true;
        observer[event.ObsIndex()]->DeassertReady();
    }

    if (pidControlState == PCS_STEP_INPUT) {
        
        OJointValue* log = &logData[pidControlInfo.sensorCounter];
        sensorVec = (OSensorFrameVectorData*)event.Data(0);
        int sidx = sensorIndex[pidControlInfo.jointIndex];
        OSensorFrameData* data = sensorVec->GetData(sidx);

        memcpy((void*)log, (void*)data->frame,
               STEP_INPUT_NUM_FRAMES * sizeof(OJointValue));

        if (pidControlInfo.sensorCounter < HOLD_TIME_NUM_FRAMES) {
            pidControlInfo.sensorCounter += STEP_INPUT_NUM_FRAMES;
        }
                
        if (pidControlInfo.sensorCounter == HOLD_TIME_NUM_FRAMES) {
            observer[event.ObsIndex()]->DeassertReady();
        } else {
            observer[event.ObsIndex()]->AssertReady();
        }
    }
}

void
PIDControl::OpenPrimitives()
{
    for (int i = 0; i < NUM_JOINTS; i++) {
        OStatus result = OPENR::OpenPrimitive(JOINT_LOCATOR[i], &jointID[i]);
        if (result != oSUCCESS) {
            OSYSLOG1((osyslogERROR, "%s : %s %d",
                      "PIDControl::DoInit()",
                      "OPENR::OpenPrimitive() FAILED", result));
        }
    }
}

void
PIDControl::NewCommandVectorData()
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
                      "PIDControl::NewCommandVectorData()",
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
PIDControl::SetJointGain()
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
PIDControl::AdjustDiffJointValue()
{
    RCRegion* rgn = FindFreeRegion();
    OCommandVectorData* cmdVecData = (OCommandVectorData*)rgn->Base();
    cmdVecData->SetNumData(NUM_JOINTS);

    OJointValue current[NUM_JOINTS];    
    for (int i = 0; i < NUM_JOINTS; i++) {
        OJointValue current;
        OPENR::GetJointValue(jointID[i], &current);
        SetJointValue(rgn, i,
                      degrees(current.value/1000000.0),
                      degrees(current.value/1000000.0));
    }

    subject[sbjEffector]->SetData(rgn);
    subject[sbjEffector]->NotifyObservers();

    return MOVING_FINISH;
}

MovingResult
PIDControl::MoveToBroadBase()
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
PIDControl::MoveToSleeping()
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

RCRegion*
PIDControl::FindFreeRegion()
{
    for (int i = 0; i < NUM_COMMAND_VECTOR; i++) {
        if (region[i]->NumberOfReference() == 1) return region[i];
    }

    return 0;
}

void
PIDControl::SetJointValue(RCRegion* rgn, int idx, double start, double end)
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
PIDControl::SetJointValueForStepInput(RCRegion* rgn, int idx, double val)
{
    OCommandVectorData* cmdVecData = (OCommandVectorData*)rgn->Base();

    OCommandInfo* info = cmdVecData->GetInfo(0);
    info->Set(odataJOINT_COMMAND2, jointID[idx], STEP_INPUT_NUM_FRAMES);

    OCommandData* data = cmdVecData->GetData(0);
    OJointCommandValue2* jval = (OJointCommandValue2*)data->value;

    for (int i = 0; i < STEP_INPUT_NUM_FRAMES; i++) {
        jval[i].value = oradians(val);
    }
}

void 
PIDControl::InitSensorIndex(OSensorFrameVectorData* sensorVec)
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
PIDControl::DisableJointGainOfAllLegs()
{
    OPENR::DisableJointGain(jointID[RFLEG_J1]);
    OPENR::DisableJointGain(jointID[RFLEG_J2]);
    OPENR::DisableJointGain(jointID[RFLEG_J3]);
    OPENR::DisableJointGain(jointID[RRLEG_J1]);
    OPENR::DisableJointGain(jointID[RRLEG_J2]);
    OPENR::DisableJointGain(jointID[RRLEG_J3]);
    OPENR::DisableJointGain(jointID[LFLEG_J1]);
    OPENR::DisableJointGain(jointID[LFLEG_J2]);
    OPENR::DisableJointGain(jointID[LFLEG_J3]);
    OPENR::DisableJointGain(jointID[LRLEG_J1]);
    OPENR::DisableJointGain(jointID[LRLEG_J2]);
    OPENR::DisableJointGain(jointID[LRLEG_J3]);
}

void
PIDControl::InputParameter()
{
    OSYSPRINT(("[ 0]  RFLEG_J1\n"));
    OSYSPRINT(("[ 1]  RFLEG_J2\n"));
    OSYSPRINT(("[ 2]  RFLEG_J3\n"));
    OSYSPRINT(("[ 3]  LFLEG_J1\n"));
    OSYSPRINT(("[ 4]  LFLEG_J2\n"));
    OSYSPRINT(("[ 5]  LFLEG_J3\n"));
    OSYSPRINT(("[ 6]  RRLEG_J1\n"));
    OSYSPRINT(("[ 7]  RRLEG_J2\n"));
    OSYSPRINT(("[ 8]  RRLEG_J3\n"));
    OSYSPRINT(("[ 9]  LRLEG_J1\n"));
    OSYSPRINT(("[10]  LRLEG_J2\n"));
    OSYSPRINT(("[11]  LRLEG_J3\n"));
    pidControlInfo.jointIndex   = InputKey("jointIndex > ") + RFLEG_J1;
    pidControlInfo.pgain        = InputKey("pgain > ");
    pidControlInfo.igain        = InputKey("igain > ");
    pidControlInfo.dgain        = InputKey("dgain > ");
    pidControlInfo.desiredValue = InputKey("desiredValue [deg] > ");

    pidControlInfo.effectorCounter = 0;
    pidControlInfo.sensorCounter   = 0;

    OPENR::SetJointGain(jointID[pidControlInfo.jointIndex],
                        pidControlInfo.pgain,
                        pidControlInfo.igain,
                        pidControlInfo.dgain,
                        PSHIFT, ISHIFT, DSHIFT);
}

MovingResult
PIDControl::StepInput()
{
    RCRegion* rgn = FindFreeRegion();
    OCommandVectorData* cmdVecData = (OCommandVectorData*)rgn->Base();

    cmdVecData->SetNumData(1);
    SetJointValueForStepInput(rgn, pidControlInfo.jointIndex,
                              (double)pidControlInfo.desiredValue);

    subject[sbjEffector]->SetData(rgn);
    subject[sbjEffector]->NotifyObservers();

    if (pidControlInfo.effectorCounter < HOLD_TIME_NUM_FRAMES) {
        pidControlInfo.effectorCounter += STEP_INPUT_NUM_FRAMES;
    }

    if (pidControlInfo.effectorCounter == HOLD_TIME_NUM_FRAMES &&
        pidControlInfo.sensorCounter == HOLD_TIME_NUM_FRAMES) {
        return MOVING_FINISH;
    } else {
        return MOVING_CONT;
    }
}

bool
PIDControl::YesNo(const char* prompt)
{
    char line[80];
    
    while (1) {
        OSYSPRINT(("%s", prompt));
        gets(line);
        if (strcmp(line, "y") == 0) return true;
        if (strcmp(line, "n") == 0) return false;
    }
}

int
PIDControl::InputKey(const char* prompt)
{
    OSYSPRINT(("%s", prompt));
    char line[80];
    gets(line);
    return strtol(line, (char**)NULL, 0);
}

void
PIDControl::SaveLogData()
{
    char file[16];
    char path[128];

    OSYSPRINT(("filename > "));
    gets(file);
    sprintf(path, "/MS/OPEN-R/MW/DATA/P/%s", file);
    
    FILE* fp = fopen(path, "w");
    if (fp == 0) {
        OSYSLOG1((osyslogERROR, "fopen() : can't open %s", path));
        return;
    }
    
    for (int i = 0; i < HOLD_TIME_NUM_FRAMES; i++) {
        fprintf(fp, "%f\t%f\t%d\n",
                degrees(logData[i].value / 1000000.0),
                degrees(logData[i].refValue / 1000000.0),
                logData[i].pwmDuty);
    }

    fclose(fp);
}
