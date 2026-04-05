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

#ifndef SensorObserver7_h_DEFINED
#define SensorObserver7_h_DEFINED

#include <OPENR/OObject.h>
#include <OPENR/OSubject.h> 
#include <OPENR/OObserver.h>
#include "def.h"

const int NUM_ERS7_SENSORS = 34;

const int ACC_Y         =  0;
const int ACC_X         =  1;
const int ACC_Z         =  2; 
const int BODY_PSD      =  3; 
const int WLAN_SW       =  4;
const int BACK_SW_R     =  5;
const int BACK_SW_M     =  6;
const int BACK_SW_F     =  7;
const int HEAD_SENSOR   =  8;
const int CHIN_SW       =  9;
const int HEAD_PSD_NEAR = 10;
const int HEAD_PSD_FAR  = 11;
const int HEAD_TILT1    = 12;
const int HEAD_PAN      = 13;
const int HEAD_TILT2    = 14;
const int MOUTH         = 15;
const int RFLEG_J1      = 16;
const int RFLEG_J2      = 17;
const int RFLEG_J3      = 18;
const int RFLEG_SW      = 19;
const int LFLEG_J1      = 20;
const int LFLEG_J2      = 21;
const int LFLEG_J3      = 22;
const int LFLEG_SW      = 23;
const int RRLEG_J1      = 24;
const int RRLEG_J2      = 25;
const int RRLEG_J3      = 26;
const int RRLEG_SW      = 27;
const int LRLEG_J1      = 28;
const int LRLEG_J2      = 29;
const int LRLEG_J3      = 30;
const int LRLEG_SW      = 31;
const int TAIL_TILT     = 32;
const int TAIL_PAN      = 33;

static const char* const ERS7_SENSOR_LOCATOR[] = {
    // BODY
    "PRM:/a1-Sensor:a1",                // ACCELEROMETER Y
    "PRM:/a2-Sensor:a2",                // ACCELEROMETER X
    "PRM:/a3-Sensor:a3",                // ACCELEROMETER Z
    "PRM:/p1-Sensor:p1",                // BODY PSD
    "PRM:/b1-Sensor:b1",                // WIRELESS LAN SWITCH
    "PRM:/t2-Sensor:t2",                // BACK SENSOR (REAR)
    "PRM:/t3-Sensor:t3",                // BACK SENSOR (MIDDLE)
    "PRM:/t4-Sensor:t4",                // BACK SENSOR (FRONT)

    // HEAD
    "PRM:/r1/c1/c2/c3/t1-Sensor:t1",    // HEAD SENSOR
    "PRM:/r1/c1/c2/c3/c4/s5-Sensor:s5", // CHIN SWITCH
    "PRM:/r1/c1/c2/c3/p1-Sensor:p1",    // HEAD PSD (NEAR)
    "PRM:/r1/c1/c2/c3/p2-Sensor:p2",    // HEAD PSD (FAR)
    "PRM:/r1/c1-Joint2:11",             // HEAD TILT1
    "PRM:/r1/c1/c2-Joint2:12",          // HEAD PAN
    "PRM:/r1/c1/c2/c3-Joint2:13",       // HEAD TILT2
    "PRM:/r1/c1/c2/c3/c4-Joint2:14",    // MOUTH

    // RFLEG (Right Front Leg)
    "PRM:/r4/c1-Joint2:41",             // RFLEG J1
    "PRM:/r4/c1/c2-Joint2:42",          // RFLEG J2
    "PRM:/r4/c1/c2/c3-Joint2:43",       // RFLEG J3
    "PRM:/r4/c1/c2/c3/c4-Sensor:44",    // RFLEG SW

    // LFLEG (Left Front Leg)
    "PRM:/r2/c1-Joint2:21",             // LFLEG J1
    "PRM:/r2/c1/c2-Joint2:22",          // LFLEG J2
    "PRM:/r2/c1/c2/c3-Joint2:23",       // LFLEG J3
    "PRM:/r2/c1/c2/c3/c4-Sensor:24",    // LFLEG SW 

    // RRLEG (Right Rear Leg)
    "PRM:/r5/c1-Joint2:51",             // RRLEG J1
    "PRM:/r5/c1/c2-Joint2:52",          // RRLEG J2
    "PRM:/r5/c1/c2/c3-Joint2:53",       // RRLEG J3
    "PRM:/r5/c1/c2/c3/c4-Sensor:54",    // RRLEG SW

    // LRLEG (Left Rear Leg)
    "PRM:/r3/c1-Joint2:31",             // LRLEG J1
    "PRM:/r3/c1/c2-Joint2:32",          // LRLEG J2
    "PRM:/r3/c1/c2/c3-Joint2:33",       // LRLEG J3
    "PRM:/r3/c1/c2/c3/c4-Sensor:34",    // LRLEG SW

    // TAIL CPC
    "PRM:/r6/c1-Joint2:61",             // TAIL TILT
    "PRM:/r6/c2-Joint2:62"              // TAIL PAN
};

class SensorObserver7 : public OObject {
public:  
    SensorObserver7();
    virtual ~SensorObserver7() {}
  
    OSubject*    subject[numOfSubject];
    OObserver*   observer[numOfObserver];

    virtual OStatus DoInit   (const OSystemEvent& event);
    virtual OStatus DoStart  (const OSystemEvent& event);
    virtual OStatus DoStop   (const OSystemEvent& event);
    virtual OStatus DoDestroy(const OSystemEvent& event);
    
    void NotifyERS7(const ONotifyEvent& event);

private:
    void InitERS7SensorIndex(OSensorFrameVectorData* sensorVec);
    void PrintERS7Sensor(OSensorFrameVectorData* sensorVec);
    void PrintSensorValue(OSensorFrameVectorData* sensorVec, int index);
    void PrintJointValue(OSensorFrameVectorData* sensorVec, int index);
    void PrintSeparator();
    void WaitReturnKey();

    bool initSensorIndex;
    int  ers7idx[NUM_ERS7_SENSORS];
};

#endif // SensorObserver7_h_DEFINED
