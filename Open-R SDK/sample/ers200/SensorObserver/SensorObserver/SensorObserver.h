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

#ifndef SensorObserver_h_DEFINED
#define SensorObserver_h_DEFINED

#include <OPENR/OObject.h>
#include <OPENR/OSubject.h> 
#include <OPENR/OObserver.h>
#include "def.h"

const int NUM_COMMON_SENSORS = 28;
const int NUM_ERS210_SENSORS = 3;
const int NUM_ERS220_SENSORS = 3;

const int ACC_Y      =  0;
const int ACC_X      =  1;
const int ACC_Z      =  2; 
const int TACT_R     =  3;
const int TACT_F     =  4;
const int TIN_SW     =  5;
const int PSD        =  6;
const int HEAD_TILT  =  7;
const int HEAD_PAN   =  8;
const int HEAD_ROLL  =  9;
const int RFLEG_J1   = 10;
const int RFLEG_J2   = 11;
const int RFLEG_J3   = 12;
const int RFLEG_SW   = 13;
const int LFLEG_J1   = 14;
const int LFLEG_J2   = 15;
const int LFLEG_J3   = 16;
const int LFLEG_SW   = 17;
const int RRLEG_J1   = 18;
const int RRLEG_J2   = 19;
const int RRLEG_J3   = 20;
const int RRLEG_SW   = 21;
const int LRLEG_J1   = 22;
const int LRLEG_J2   = 23;
const int LRLEG_J3   = 24;
const int LRLEG_SW   = 25;
const int BACK_SW    = 26;
const int THERMO     = 27;

const int MOUTH      =  0;
const int TAIL_PAN   =  1;
const int TAIL_TILT  =  2;

const int TAIL_SW_L  =  0;
const int TAIL_SW_M  =  1;
const int TAIL_SW_R  =  2;

static const char* const COMMON_SENSOR_LOCATOR[] = {
    // BODY CPC
    "PRM:/a1-Sensor:a1",                // ACCELEROMETER Y
    "PRM:/a2-Sensor:a2",                // ACCELEROMETER X
    "PRM:/a3-Sensor:a3",                // ACCELEROMETER Z

    // HEAD CPC
    "PRM:/r1/c1/c2/c3/f1-Sensor:f1",    // HEAD TACTILE SENSOR (REAR)
    "PRM:/r1/c1/c2/c3/f2-Sensor:f2",    // HEAD TACTILE SENSOR (FRONT)
    "PRM:/r1/c1/c2/c3/c4/s5-Sensor:s5", // TIN SWITCH
    "PRM:/r1/c1/c2/c3/p1-Sensor:p1",    // PSD
    "PRM:/r1/c1-Joint2:j1",             // HEAD TILT
    "PRM:/r1/c1/c2-Joint2:j2",          // HEAD PAN
    "PRM:/r1/c1/c2/c3-Joint2:j3",       // HEAD ROLL

    // RFLEG CPC
    "PRM:/r4/c1-Joint2:j1",             // RFLEG J1 (Right Front Leg)
    "PRM:/r4/c1/c2-Joint2:j2",          // RFLEG J2
    "PRM:/r4/c1/c2/c3-Joint2:j3",       // RFLEG J3
    "PRM:/r4/c1/c2/c3/c4-Sensor:s4",    // RFLEG SW

    // LFLEG CPC
    "PRM:/r2/c1-Joint2:j1",             // LFLEG J1 (Left Front Leg)
    "PRM:/r2/c1/c2-Joint2:j2",          // LFLEG J2
    "PRM:/r2/c1/c2/c3-Joint2:j3",       // LFLEG J3
    "PRM:/r2/c1/c2/c3/c4-Sensor:s4",    // LFLEG SW 

    // RRLEG CPC
    "PRM:/r5/c1-Joint2:j1",             // RRLEG J1 (Right Rear Leg)
    "PRM:/r5/c1/c2-Joint2:j2",          // RRLEG J2
    "PRM:/r5/c1/c2/c3-Joint2:j3",       // RRLEG J3
    "PRM:/r5/c1/c2/c3/c4-Sensor:s4",    // RRLEG SW

    // LRLEG CPC
    "PRM:/r3/c1-Joint2:j1",             // LRLEG J1 (Left Rear Leg)
    "PRM:/r3/c1/c2-Joint2:j2",          // LRLEG J2
    "PRM:/r3/c1/c2/c3-Joint2:j3",       // LRLEG J3
    "PRM:/r3/c1/c2/c3/c4-Sensor:s4",    // LRLEG SW

    // TAIL CPC
    "PRM:/r6/s1-Sensor:s1",             // BACK SWITCH
    "PRM:/r6/t1-Sensor:t1"              // THERMOMETER
};

static const char* const ERS210_SENSOR_LOCATOR[] = {
    // HEAD CPC
    "PRM:/r1/c1/c2/c3/c4-Joint2:j4",    // MOUTH 
    
    // TAIL CPC
    "PRM:/r6/c1-Joint2:j1",             // TAIL PAN
    "PRM:/r6/c2-Joint2:j2"              // TAIL TILT
};

static const char* const ERS220_SENSOR_LOCATOR[] = {
    // TAIL CPC
    "PRM:/r6/s2-Sensor:s2",             // TAIL SWITCH (LEFT)
    "PRM:/r6/s3-Sensor:s3",             // TAIL SWITCH (MID)
    "PRM:/r6/s4-Sensor:s4"              // TAIL SWITCH (RIGHT)
};

class SensorObserver : public OObject {
public:  
    SensorObserver();
    virtual ~SensorObserver() {}
  
    OSubject*    subject[numOfSubject];
    OObserver*   observer[numOfObserver];

    virtual OStatus DoInit   (const OSystemEvent& event);
    virtual OStatus DoStart  (const OSystemEvent& event);
    virtual OStatus DoStop   (const OSystemEvent& event);
    virtual OStatus DoDestroy(const OSystemEvent& event);
    
    void NotifyERS210(const ONotifyEvent& event);
    void NotifyERS220(const ONotifyEvent& event);

private:
    void InitCommonSensorIndex(OSensorFrameVectorData* sensorVec);
    void InitERS210SensorIndex(OSensorFrameVectorData* sensorVec);
    void InitERS220SensorIndex(OSensorFrameVectorData* sensorVec);
    void PrintCommonSensor(OSensorFrameVectorData* sensorVec);
    void PrintERS210Sensor(OSensorFrameVectorData* sensorVec);
    void PrintERS220Sensor(OSensorFrameVectorData* sensorVec);
    void PrintSensorValue(OSensorFrameVectorData* sensorVec, int index);
    void PrintJointValue(OSensorFrameVectorData* sensorVec, int index);
    void PrintSeparator();
    void WaitReturnKey();

    bool initSensorIndex;
    int  commonidx[NUM_COMMON_SENSORS];
    int  ers210idx[NUM_ERS210_SENSORS];
    int  ers220idx[NUM_ERS220_SENSORS];
};

#endif // SensorObserver_h_DEFINED
