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

#ifndef PIDControl_h_DEFINED
#define PIDControl_h_DEFINED

#include <OPENR/OObject.h>
#include <OPENR/OSubject.h>
#include <OPENR/OObserver.h>
#include "def.h"

enum PIDControlState {
    PCS_IDLE,
    PCS_START,
    PCS_ADJUSTING_DIFF_JOINT_VALUE,
    PCS_MOVING_TO_BROADBASE,
    PCS_MOVING_TO_SLEEPING,
    PCS_DISABLE_JOINT_GAIN,
    PCS_INPUT_PARAMETER,
    PCS_STEP_INPUT
};

enum MovingResult {
    MOVING_CONT,
    MOVING_FINISH
};

const int HEAD_TILT  =  0;
const int HEAD_PAN   =  1;
const int HEAD_ROLL  =  2;
const int RFLEG_J1   =  3;
const int RFLEG_J2   =  4;
const int RFLEG_J3   =  5;
const int LFLEG_J1   =  6;
const int LFLEG_J2   =  7;
const int LFLEG_J3   =  8;
const int RRLEG_J1   =  9;
const int RRLEG_J2   = 10;
const int RRLEG_J3   = 11;
const int LRLEG_J1   = 12;
const int LRLEG_J2   = 13;
const int LRLEG_J3   = 14;
const int NUM_JOINTS = 15;

static const char* const JOINT_LOCATOR[] = {
    "PRM:/r1/c1-Joint2:j1",       // HEAD TILT
    "PRM:/r1/c1/c2-Joint2:j2",    // HEAD PAN
    "PRM:/r1/c1/c2/c3-Joint2:j3", // HEAD ROLL

    "PRM:/r4/c1-Joint2:j1",       // RFLEG J1 (Right Front Leg)
    "PRM:/r4/c1/c2-Joint2:j2",    // RFLEG J2
    "PRM:/r4/c1/c2/c3-Joint2:j3", // RFLEG J3

    "PRM:/r2/c1-Joint2:j1",       // LFLEG J1 (Left Front Leg)
    "PRM:/r2/c1/c2-Joint2:j2",    // LFLEG J2
    "PRM:/r2/c1/c2/c3-Joint2:j3", // LFLEG J3

    "PRM:/r5/c1-Joint2:j1",       // RRLEG J1 (Right Rear Leg)
    "PRM:/r5/c1/c2-Joint2:j2",    // RRLEG J2
    "PRM:/r5/c1/c2/c3-Joint2:j3", // RRLEG J3

    "PRM:/r3/c1-Joint2:j1",       // LRLEG J1 (Left Rear Leg)
    "PRM:/r3/c1/c2-Joint2:j2",    // LRLEG J2
    "PRM:/r3/c1/c2/c3-Joint2:j3"  // LRLEG J3
};

const double BROADBASE_ANGLE[] = {
    45,   // TILT
    0,    // PAN
    0,    // ROLL

    117,  // RFLEG J1
    90,   // RFLEG J2
    30,   // RFLEG J3

    117,  // LFLEG J1
    90,   // LFLEG J2
    30,   // LFLEG J3
    
    -117, // RRLEG J1
    70,   // RRLEG J2
    30,   // RRLEG J3

    -117, // LRLEG J1
    70,   // LRLEG J2
    30    // LRLEG J3
};

const double SLEEPING_ANGLE[] = {
    -10,  // TILT
    0,    // PAN
    0,    // ROLL

    60,   // RFLEG J1
    0,    // RFLEG J2
    30,   // RFLEG J3

    60,   // LFLEG J1
    0,    // LFLEG J2
    30,   // LFLEG J3

    -117, // RRLEG J1
    0,    // RRLEG J2
    147,  // RRLEG J3

    -117, // LRLEG J1
    0,    // LRLEG J2
    147   // LRLEG J3
};

struct PIDControlInfo {
    int  jointIndex;
    word pgain;
    word igain;
    word dgain;
    int  desiredValue;
    int  effectorCounter;
    int  sensorCounter;

    PIDControlInfo() {
        jointIndex      = RFLEG_J1;
        pgain           = 0;
        igain           = 0;
        dgain           = 0;
        desiredValue    = 0;
        effectorCounter = 0;
        sensorCounter   = 0;
    }
};

class PIDControl : public OObject {
public:
    PIDControl();
    virtual ~PIDControl() {}

    OSubject*  subject[numOfSubject];
    OObserver* observer[numOfObserver];

    virtual OStatus DoInit   (const OSystemEvent& event);
    virtual OStatus DoStart  (const OSystemEvent& event);
    virtual OStatus DoStop   (const OSystemEvent& event);
    virtual OStatus DoDestroy(const OSystemEvent& event);
    
    void ReadyEffector(const OReadyEvent& event);
    void NotifySensor(const ONotifyEvent& event);

private:
    void          OpenPrimitives();
    void          NewCommandVectorData();
    void          SetJointGain();

    MovingResult  AdjustDiffJointValue();
    MovingResult  MoveToBroadBase();
    MovingResult  MoveToSleeping();

    RCRegion* FindFreeRegion();
    void SetJointValue(RCRegion* rgn, int idx, double start, double end);
    void SetJointValueForStepInput(RCRegion* rgn, int idx, double val);
    void InitSensorIndex(OSensorFrameVectorData* sensorVecData);
    void DisableJointGainOfAllLegs();
    void InputParameter();
    MovingResult StepInput();
    void SaveLogData();

    bool YesNo(const char* prompt);
    int  InputKey(const char* prompt);

    static const size_t NUM_JOINTS         = 15;
    static const size_t NUM_COMMAND_VECTOR = 2;

    static const word   TILT_PGAIN = 0x000a;
    static const word   TILT_IGAIN = 0x0008;
    static const word   TILT_DGAIN = 0x000c;

    static const word   PAN_PGAIN  = 0x000d;
    static const word   PAN_IGAIN  = 0x0008;
    static const word   PAN_DGAIN  = 0x000b;

    static const word   ROLL_PGAIN = 0x000a;
    static const word   ROLL_IGAIN = 0x0008;
    static const word   ROLL_DGAIN = 0x000c;
    
    static const word   J1_PGAIN   = 0x0016;
    static const word   J1_IGAIN   = 0x0004;
    static const word   J1_DGAIN   = 0x0008;

    static const word   J2_PGAIN   = 0x0014;
    static const word   J2_IGAIN   = 0x0004;
    static const word   J2_DGAIN   = 0x0006;

    static const word   J3_PGAIN   = 0x0023;
    static const word   J3_IGAIN   = 0x0004;
    static const word   J3_DGAIN   = 0x0005;

    static const word   PSHIFT     = 0x000e;
    static const word   ISHIFT     = 0x0002;
    static const word   DSHIFT     = 0x000f;

    static const int BROADBASE_MAX_COUNTER = 48; // 128ms * 48 = 6144ms
    static const int SLEEPING_MAX_COUNTER  = 48; // 128ms * 48 = 6144ms

    static const int STEP_INPUT_NUM_FRAMES = 4;
    static const int HOLD_TIME_NUM_FRAMES  = 256; // 8ms * 512 = 4096ms
    
    PIDControlState pidControlState;
    PIDControlInfo  pidControlInfo;

    OPrimitiveID jointID[NUM_JOINTS];
    RCRegion*    region[NUM_COMMAND_VECTOR];
    OJointValue* logData;

    bool      initSensorIndex;
    int       sensorIndex[NUM_JOINTS];
};

#endif // PIDControl_h_DEFINED
