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

#include <OPENR/ODataFormats.h>
#include <OPENR/OPENRAPI.h>
#include <OPENR/OSyslog.h>
#include <OPENR/core_macro.h>
#include "SensorObserver7.h"

SensorObserver7::SensorObserver7() : initSensorIndex(false)
{
    for (int i = 0; i < NUM_ERS7_SENSORS; i++) ers7idx[i] = -1;
}

OStatus
SensorObserver7::DoInit(const OSystemEvent& event)
{
    NEW_ALL_SUBJECT_AND_OBSERVER;
    REGISTER_ALL_ENTRY;
    SET_ALL_READY_AND_NOTIFY_ENTRY;
    OPENR::SetMotorPower(opowerON);
    return oSUCCESS;
}

OStatus
SensorObserver7::DoStart(const OSystemEvent& event)
{
    ENABLE_ALL_SUBJECT;
    ASSERT_READY_TO_ALL_OBSERVER;
    return oSUCCESS;
}    

OStatus
SensorObserver7::DoStop(const OSystemEvent& event)
{
    DISABLE_ALL_SUBJECT;
    DEASSERT_READY_TO_ALL_OBSERVER;
    return oSUCCESS;
}

OStatus
SensorObserver7::DoDestroy(const OSystemEvent& event)
{
    DELETE_ALL_SUBJECT_AND_OBSERVER;
    return oSUCCESS;
}

void
SensorObserver7::NotifyERS7(const ONotifyEvent& event)
{
    OSensorFrameVectorData* sensorVec = (OSensorFrameVectorData*)event.Data(0);

    if (initSensorIndex == false) {
        InitERS7SensorIndex(sensorVec);
        initSensorIndex = true;
    }

    OSYSPRINT(("ERS-7 numData %d frameNumber %d\n",
               sensorVec->vectorInfo.numData, sensorVec->info[0].frameNumber));
    
    PrintERS7Sensor(sensorVec);
    WaitReturnKey();

    observer[event.ObsIndex()]->AssertReady();
}

void
SensorObserver7::InitERS7SensorIndex(OSensorFrameVectorData* sensorVec)
{
    OStatus result;
    OPrimitiveID sensorID;
    
    for (int i = 0; i < NUM_ERS7_SENSORS; i++) {

        result = OPENR::OpenPrimitive(ERS7_SENSOR_LOCATOR[i], &sensorID);
        if (result != oSUCCESS) {
            OSYSLOG1((osyslogERROR, "%s : %s %d",
                      "SensorObserver7::InitERS7SensorIndex()",
                      "OPENR::OpenPrimitive() FAILED", result));
            continue;
        }

        for (int j = 0; j < sensorVec->vectorInfo.numData; j++) {
            OSensorFrameInfo* info = sensorVec->GetInfo(j);
            if (info->primitiveID == sensorID) {
                ers7idx[i] = j;
                OSYSPRINT(("[%2d] %s\n",
                           ers7idx[i], ERS7_SENSOR_LOCATOR[i]));
                break;
            }
        }
    }
}

void
SensorObserver7::PrintERS7Sensor(OSensorFrameVectorData* sensorVec)
{
    PrintSeparator();

    //
    // BODY
    //
    OSYSPRINT(("ACC X      | "));
    PrintSensorValue(sensorVec, ers7idx[ACC_X]);

    OSYSPRINT(("ACC Y      | "));
    PrintSensorValue(sensorVec, ers7idx[ACC_Y]);

    OSYSPRINT(("ACC Z      | "));
    PrintSensorValue(sensorVec, ers7idx[ACC_Z]);

    OSYSPRINT(("BODY PSD   | "));
    PrintSensorValue(sensorVec, ers7idx[BODY_PSD]);

    OSYSPRINT(("WLAN SW    | "));
    PrintSensorValue(sensorVec, ers7idx[WLAN_SW]);

    OSYSPRINT(("BACK SW F  | "));
    PrintSensorValue(sensorVec, ers7idx[BACK_SW_F]);

    OSYSPRINT(("BACK SW M  | "));
    PrintSensorValue(sensorVec, ers7idx[BACK_SW_M]);

    OSYSPRINT(("BACK SW R  | "));
    PrintSensorValue(sensorVec, ers7idx[BACK_SW_R]);

    //
    // HEAD
    //
    OSYSPRINT(("HEAD SENSOR| "));
    PrintSensorValue(sensorVec, ers7idx[HEAD_SENSOR]);

    OSYSPRINT(("CHIN SW    | "));
    PrintSensorValue(sensorVec, ers7idx[CHIN_SW]);

    OSYSPRINT(("PSD NEAR   | "));
    PrintSensorValue(sensorVec, ers7idx[HEAD_PSD_NEAR]);

    OSYSPRINT(("PSD FAR    | "));
    PrintSensorValue(sensorVec, ers7idx[HEAD_PSD_FAR]);

    OSYSPRINT(("HEAD TILT1 | "));
    PrintJointValue(sensorVec, ers7idx[HEAD_TILT1]);

    OSYSPRINT(("HEAD PAN   | "));
    PrintJointValue(sensorVec, ers7idx[HEAD_PAN]);

    OSYSPRINT(("HEAD TILT2 | "));
    PrintJointValue(sensorVec, ers7idx[HEAD_TILT2]);

    OSYSPRINT(("MOUTH      | "));
    PrintJointValue(sensorVec, ers7idx[MOUTH]);

    //
    // RFLEG
    //
    OSYSPRINT(("RFLEG J1   | "));
    PrintJointValue(sensorVec, ers7idx[RFLEG_J1]);

    OSYSPRINT(("RFLEG J2   | "));
    PrintJointValue(sensorVec, ers7idx[RFLEG_J2]);

    OSYSPRINT(("RFLEG J3   | "));
    PrintJointValue(sensorVec, ers7idx[RFLEG_J3]);

    OSYSPRINT(("RFLEG SW   | "));
    PrintSensorValue(sensorVec, ers7idx[RFLEG_SW]);

    //
    // LFLEG
    //
    OSYSPRINT(("LFLEG J1   | "));
    PrintJointValue(sensorVec, ers7idx[LFLEG_J1]);

    OSYSPRINT(("LFLEG J2   | "));
    PrintJointValue(sensorVec, ers7idx[LFLEG_J2]);

    OSYSPRINT(("LFLEG J3   | "));
    PrintJointValue(sensorVec, ers7idx[LFLEG_J3]);

    OSYSPRINT(("LFLEG SW   | "));
    PrintSensorValue(sensorVec, ers7idx[LFLEG_SW]);

    //
    // RRLEG
    //
    OSYSPRINT(("RRLEG J1   | "));
    PrintJointValue(sensorVec, ers7idx[RRLEG_J1]);

    OSYSPRINT(("RRLEG J2   | "));
    PrintJointValue(sensorVec, ers7idx[RRLEG_J2]);

    OSYSPRINT(("RRLEG J3   | "));
    PrintJointValue(sensorVec, ers7idx[RRLEG_J3]);

    OSYSPRINT(("RRLEG SW   | "));
    PrintSensorValue(sensorVec, ers7idx[RRLEG_SW]);

    //
    // LRLEG
    //
    OSYSPRINT(("LRLEG J1   | "));
    PrintJointValue(sensorVec, ers7idx[LRLEG_J1]);

    OSYSPRINT(("LRLEG J2   | "));
    PrintJointValue(sensorVec, ers7idx[LRLEG_J2]);

    OSYSPRINT(("LRLEG J3   | "));
    PrintJointValue(sensorVec, ers7idx[LRLEG_J3]);

    OSYSPRINT(("LRLEG SW   | "));
    PrintSensorValue(sensorVec, ers7idx[LRLEG_SW]);

    //
    // TAIL
    //
    OSYSPRINT(("TAIL TILT  | "));
    PrintJointValue(sensorVec, ers7idx[TAIL_TILT]);

    OSYSPRINT(("TAIL PAN   | "));
    PrintJointValue(sensorVec, ers7idx[TAIL_PAN]);
}

void
SensorObserver7::PrintSensorValue(OSensorFrameVectorData* sensorVec, int index)
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
SensorObserver7::PrintJointValue(OSensorFrameVectorData* sensorVec, int index)
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
SensorObserver7::PrintSeparator()
{
    OSYSPRINT(("-----------+"));
    OSYSPRINT(("---------------------------------------------------------\n"));
}

#ifdef WAIT_RETURN_KEY
#include <stdio.h> // for getchar()
#endif 

void
SensorObserver7::WaitReturnKey()
{
#ifdef WAIT_RETURN_KEY
    OSYSPRINT(("Hit return key> "));
    char c = getchar();
#endif
}
