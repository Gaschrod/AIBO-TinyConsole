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

#include <OPENR/ODataFormats.h>
#include <OPENR/OPENRAPI.h>
#include <OPENR/OSyslog.h>
#include <OPENR/core_macro.h>
#include "SensorObserver.h"

SensorObserver::SensorObserver() : initSensorIndex(false)
{
    for (int i = 0; i < NUM_COMMON_SENSORS; i++) commonidx[i] = -1;
    for (int i = 0; i < NUM_ERS210_SENSORS; i++) ers210idx[i] = -1;
    for (int i = 0; i < NUM_ERS220_SENSORS; i++) ers220idx[i] = -1;
}

OStatus
SensorObserver::DoInit(const OSystemEvent& event)
{
    NEW_ALL_SUBJECT_AND_OBSERVER;
    REGISTER_ALL_ENTRY;
    SET_ALL_READY_AND_NOTIFY_ENTRY;
    OPENR::SetMotorPower(opowerON);
    return oSUCCESS;
}

OStatus
SensorObserver::DoStart(const OSystemEvent& event)
{
    ENABLE_ALL_SUBJECT;
    ASSERT_READY_TO_ALL_OBSERVER;
    return oSUCCESS;
}    

OStatus
SensorObserver::DoStop(const OSystemEvent& event)
{
    DISABLE_ALL_SUBJECT;
    DEASSERT_READY_TO_ALL_OBSERVER;
    return oSUCCESS;
}

OStatus
SensorObserver::DoDestroy(const OSystemEvent& event)
{
    DELETE_ALL_SUBJECT_AND_OBSERVER;
    return oSUCCESS;
}

void
SensorObserver::NotifyERS210(const ONotifyEvent& event)
{
    OSensorFrameVectorData* sensorVec = (OSensorFrameVectorData*)event.Data(0);

    if (initSensorIndex == false) {
        InitCommonSensorIndex(sensorVec);
        InitERS210SensorIndex(sensorVec);
        initSensorIndex = true;
    }

    OSYSPRINT(("ERS-210 numData %d frameNumber %d\n",
               sensorVec->vectorInfo.numData, sensorVec->info[0].frameNumber));
    
    PrintCommonSensor(sensorVec);
    PrintERS210Sensor(sensorVec);
    WaitReturnKey();

    observer[event.ObsIndex()]->AssertReady();
}

void
SensorObserver::NotifyERS220(const ONotifyEvent& event)
{
    OSensorFrameVectorData* sensorVec = (OSensorFrameVectorData*)event.Data(0);

    if (initSensorIndex == false) {
        InitCommonSensorIndex(sensorVec);
        InitERS220SensorIndex(sensorVec);
        initSensorIndex = true;
    }

    OSYSPRINT(("ERS-220 numData %d frameNumber %d\n",
               sensorVec->vectorInfo.numData, sensorVec->info[0].frameNumber));

    PrintCommonSensor(sensorVec);
    PrintERS220Sensor(sensorVec);
    WaitReturnKey();

    observer[event.ObsIndex()]->AssertReady();
}

void
SensorObserver::InitCommonSensorIndex(OSensorFrameVectorData* sensorVec)
{
    OStatus result;
    OPrimitiveID sensorID;
    
    for (int i = 0; i < NUM_COMMON_SENSORS; i++) {

        result = OPENR::OpenPrimitive(COMMON_SENSOR_LOCATOR[i], &sensorID);
        if (result != oSUCCESS) {
            OSYSLOG1((osyslogERROR, "%s : %s %d",
                      "SensorObserver::InitCommonSensorIndex()",
                      "OPENR::OpenPrimitive() FAILED", result));
            continue;
        }

        for (int j = 0; j < sensorVec->vectorInfo.numData; j++) {
            OSensorFrameInfo* info = sensorVec->GetInfo(j);
            if (info->primitiveID == sensorID) {
                commonidx[i] = j;
                OSYSPRINT(("[%2d] %s\n",
                           commonidx[i], COMMON_SENSOR_LOCATOR[i]));
                break;
            }
        }
    }
}

void
SensorObserver::InitERS210SensorIndex(OSensorFrameVectorData* sensorVec)
{
    OStatus result;
    OPrimitiveID sensorID;
    
    for (int i = 0; i < NUM_ERS210_SENSORS; i++) {

        result = OPENR::OpenPrimitive(ERS210_SENSOR_LOCATOR[i], &sensorID);
        if (result != oSUCCESS) {
            OSYSLOG1((osyslogERROR, "%s : %s %d",
                      "SensorObserver::InitERS210SensorIndex()",
                      "OPENR::OpenPrimitive() FAILED", result));
            continue;
        }

        for (int j = 0; j < sensorVec->vectorInfo.numData; j++) {
            OSensorFrameInfo* info = sensorVec->GetInfo(j);
            if (info->primitiveID == sensorID) {
                ers210idx[i] = j;
                OSYSPRINT(("[%2d] %s\n",
                           ers210idx[i], ERS210_SENSOR_LOCATOR[i]));
                break;
            }
        }
    }
}

void
SensorObserver::InitERS220SensorIndex(OSensorFrameVectorData* sensorVec)
{
    OStatus result;
    OPrimitiveID sensorID;
    
    for (int i = 0; i < NUM_ERS220_SENSORS; i++) {

        result = OPENR::OpenPrimitive(ERS220_SENSOR_LOCATOR[i], &sensorID);
        if (result != oSUCCESS) {
            OSYSLOG1((osyslogERROR, "%s : %s %d",
                      "SensorObserver::InitERS220SensorIndex()",
                      "OPENR::OpenPrimitive() FAILED", result));
            continue;
        }

        for (int j = 0; j < sensorVec->vectorInfo.numData; j++) {
            OSensorFrameInfo* info = sensorVec->GetInfo(j);
            if (info->primitiveID == sensorID) {
                ers220idx[i] = j;
                OSYSPRINT(("[%2d] %s\n",
                           ers220idx[i], ERS220_SENSOR_LOCATOR[i]));
                break;
            }
        }
    }
}

void
SensorObserver::PrintCommonSensor(OSensorFrameVectorData* sensorVec)
{
    PrintSeparator();

    //
    // BODY CPC
    //
    OSYSPRINT(("ACC X      | "));
    PrintSensorValue(sensorVec, commonidx[ACC_X]);

    OSYSPRINT(("ACC Y      | "));
    PrintSensorValue(sensorVec, commonidx[ACC_Y]);

    OSYSPRINT(("ACC Z      | "));
    PrintSensorValue(sensorVec, commonidx[ACC_Z]);

    //
    // HEAD CPC
    //
    OSYSPRINT(("TACT FRONT | "));
    PrintSensorValue(sensorVec, commonidx[TACT_F]);

    OSYSPRINT(("TACT REAR  | "));
    PrintSensorValue(sensorVec, commonidx[TACT_R]);

    OSYSPRINT(("TIN SW     | "));
    PrintSensorValue(sensorVec, commonidx[TIN_SW]);

    OSYSPRINT(("PSD        | "));
    PrintSensorValue(sensorVec, commonidx[PSD]);

    OSYSPRINT(("HEAD TILT  | "));
    PrintJointValue(sensorVec, commonidx[HEAD_TILT]);

    OSYSPRINT(("HEAD PAN   | "));
    PrintJointValue(sensorVec, commonidx[HEAD_PAN]);

    OSYSPRINT(("HEAD ROLL  | "));
    PrintJointValue(sensorVec, commonidx[HEAD_ROLL]);

    //
    // RFLEG CPC
    //
    OSYSPRINT(("RFLEG J1   | "));
    PrintJointValue(sensorVec, commonidx[RFLEG_J1]);

    OSYSPRINT(("RFLEG J2   | "));
    PrintJointValue(sensorVec, commonidx[RFLEG_J2]);

    OSYSPRINT(("RFLEG J3   | "));
    PrintJointValue(sensorVec, commonidx[RFLEG_J3]);

    OSYSPRINT(("RFLEG SW   | "));
    PrintSensorValue(sensorVec, commonidx[RFLEG_SW]);

    //
    // LFLEG CPC
    //
    OSYSPRINT(("LFLEG J1   | "));
    PrintJointValue(sensorVec, commonidx[LFLEG_J1]);

    OSYSPRINT(("LFLEG J2   | "));
    PrintJointValue(sensorVec, commonidx[LFLEG_J2]);

    OSYSPRINT(("LFLEG J3   | "));
    PrintJointValue(sensorVec, commonidx[LFLEG_J3]);

    OSYSPRINT(("LFLEG SW   | "));
    PrintSensorValue(sensorVec, commonidx[LFLEG_SW]);

    //
    // RRLEG CPC
    //
    OSYSPRINT(("RRLEG J1   | "));
    PrintJointValue(sensorVec, commonidx[RRLEG_J1]);

    OSYSPRINT(("RRLEG J2   | "));
    PrintJointValue(sensorVec, commonidx[RRLEG_J2]);

    OSYSPRINT(("RRLEG J3   | "));
    PrintJointValue(sensorVec, commonidx[RRLEG_J3]);

    OSYSPRINT(("RRLEG SW   | "));
    PrintSensorValue(sensorVec, commonidx[RRLEG_SW]);

    //
    // LRLEG CPC
    //
    OSYSPRINT(("LRLEG J1   | "));
    PrintJointValue(sensorVec, commonidx[LRLEG_J1]);

    OSYSPRINT(("LRLEG J2   | "));
    PrintJointValue(sensorVec, commonidx[LRLEG_J2]);

    OSYSPRINT(("LRLEG J3   | "));
    PrintJointValue(sensorVec, commonidx[LRLEG_J3]);

    OSYSPRINT(("LRLEG SW   | "));
    PrintSensorValue(sensorVec, commonidx[LRLEG_SW]);

    //
    // TAIL CPC
    //
    OSYSPRINT(("BACK SW    | "));
    PrintSensorValue(sensorVec, commonidx[BACK_SW]);

    OSYSPRINT(("THERMO     | "));
    PrintSensorValue(sensorVec, commonidx[THERMO]);

}

void
SensorObserver::PrintERS210Sensor(OSensorFrameVectorData* sensorVec)
{
    //
    // HEAD CPC
    //
    OSYSPRINT(("MOUTH      | "));
    PrintJointValue(sensorVec, ers210idx[MOUTH]);

    //
    // TAIL CPC
    //
    OSYSPRINT(("TAIL PAN   | "));
    PrintJointValue(sensorVec, ers210idx[TAIL_PAN]);

    OSYSPRINT(("TAIL TILT  | "));
    PrintJointValue(sensorVec, ers210idx[TAIL_TILT]);
}

void
SensorObserver::PrintERS220Sensor(OSensorFrameVectorData* sensorVec)
{
    //
    // TAIL CPC 
    //
    OSYSPRINT(("TAIL SW L  | "));
    PrintSensorValue(sensorVec, ers220idx[TAIL_SW_L]);

    OSYSPRINT(("TAIL SW M  | "));
    PrintSensorValue(sensorVec, ers220idx[TAIL_SW_M]);

    OSYSPRINT(("TAIL SW R  | "));
    PrintSensorValue(sensorVec, ers220idx[TAIL_SW_R]);
}

void
SensorObserver::PrintSensorValue(OSensorFrameVectorData* sensorVec, int index)
{
    if (index == -1) {
        OSYSPRINT(("[%d] INVALID INDEX\n", index));
        PrintSeparator();
        return;
    }

    OSensorFrameData* data = sensorVec->GetData(index);
    OSYSPRINT(("[%2d] val    %d %d %d %d\n",
               index,
               data->frame[0].value, data->frame[1].value,
               data->frame[2].value, data->frame[3].value));

    OSYSPRINT(("           | "));
    OSYSPRINT(("     sig    %d %d %d %d\n",
               data->frame[0].signal, data->frame[1].signal,
               data->frame[2].signal, data->frame[3].signal));

    PrintSeparator();
}

void
SensorObserver::PrintJointValue(OSensorFrameVectorData* sensorVec, int index)
{
    if (index == -1) {
        OSYSPRINT(("[%d] INVALID INDEX\n", index));
        PrintSeparator();
        return;
    }

    OSensorFrameData* data = sensorVec->GetData(index);
    OJointValue* jval = (OJointValue*)data->frame;

    OSYSPRINT(("[%2d] val    %d %d %d %d\n",
               index,
               jval[0].value, jval[1].value,
               jval[2].value, jval[3].value));

    OSYSPRINT(("           | "));
    OSYSPRINT(("     sig    %d %d %d %d\n",
               jval[0].signal, jval[1].signal,
               jval[2].signal, jval[3].signal));

    OSYSPRINT(("           | "));
    OSYSPRINT(("     pwm    %d %d %d %d\n",
               jval[0].pwmDuty, jval[1].pwmDuty,
               jval[2].pwmDuty, jval[3].pwmDuty));

    OSYSPRINT(("           | "));
    OSYSPRINT(("     refval %d %d %d %d\n",
               jval[0].refValue, jval[1].refValue,
               jval[2].refValue, jval[3].refValue));

    OSYSPRINT(("           | "));
    OSYSPRINT(("     refsig %d %d %d %d\n",
               jval[0].refSignal, jval[1].refSignal,
               jval[2].refSignal, jval[3].refSignal));

    PrintSeparator();
}

void
SensorObserver::PrintSeparator()
{
    OSYSPRINT(("-----------+"));
    OSYSPRINT(("---------------------------------------------------------\n"));
}

#ifdef WAIT_RETURN_KEY
#include <stdio.h> // for getchar()
#endif 

void
SensorObserver::WaitReturnKey()
{
#ifdef WAIT_RETURN_KEY
    OSYSPRINT(("Hit return key> "));
    char c = getchar();
#endif
}
