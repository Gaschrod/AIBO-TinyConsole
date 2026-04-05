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

#ifndef PIDControl7_h_DEFINED
#define PIDControl7_h_DEFINED

#include <OPENR/OObject.h>
#include <OPENR/OSubject.h>
#include <OPENR/OObserver.h>
#include "def.h"

enum PIDControl7State {
    PCS7_IDLE,
    PCS7_START,
    PCS7_ADJUSTING_DIFF_JOINT_VALUE,
    PCS7_MOVING_TO_BROADBASE,
    PCS7_MOVING_TO_SLEEPING,
    PCS7_DISABLE_JOINT_GAIN,
    PCS7_INPUT_PARAMETER,
    PCS7_STEP_INPUT
};

enum MovingResult {
    MOVING_CONT,
    MOVING_FINISH
};

const int HEAD_TILT1 =  0;
const int HEAD_PAN   =  1;
const int HEAD_TILT2 =  2;
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
const int TAIL_TILT  = 15;
const int TAIL_PAN   = 16;
const int NUM_JOINTS = 17;

static const char* const JOINT_LOCATOR[] = {
    "PRM:/r1/c1-Joint2:11",       // HEAD TILT1
    "PRM:/r1/c1/c2-Joint2:12",    // HEAD PAN
    "PRM:/r1/c1/c2/c3-Joint2:13", // HEAD TILT2

    "PRM:/r4/c1-Joint2:41",       // RFLEG J1 (Right Front Leg)
    "PRM:/r4/c1/c2-Joint2:42",    // RFLEG J2
    "PRM:/r4/c1/c2/c3-Joint2:43", // RFLEG J3

    "PRM:/r2/c1-Joint2:21",       // LFLEG J1 (Left Front Leg)
    "PRM:/r2/c1/c2-Joint2:22",    // LFLEG J2
    "PRM:/r2/c1/c2/c3-Joint2:23", // LFLEG J3

    "PRM:/r5/c1-Joint2:51",       // RRLEG J1 (Right Rear Leg)
    "PRM:/r5/c1/c2-Joint2:52",    // RRLEG J2
    "PRM:/r5/c1/c2/c3-Joint2:53", // RRLEG J3

    "PRM:/r3/c1-Joint2:31",       // LRLEG J1 (Left Rear Leg)
    "PRM:/r3/c1/c2-Joint2:32",    // LRLEG J2
    "PRM:/r3/c1/c2/c3-Joint2:33", // LRLEG J3

    "PRM:/r6/c1-Joint2:61",       // TAIL TILT
    "PRM:/r6/c2-Joint2:62"        // TAIL PAN
};

const double BROADBASE_ANGLE[] = {
    0,    // TILT1
    0,    // PAN
    40,   // TILT2

    120,  // RFLEG J1
    90,   // RFLEG J2
    30,   // RFLEG J3

    120,  // LFLEG J1
    90,   // LFLEG J2
    30,   // LFLEG J3
    
    -120, // RRLEG J1
    70,   // RRLEG J2
    30,   // RRLEG J3

    -120, // LRLEG J1
    70,   // LRLEG J2
    30,   // LRLEG J3

    0,    // TAIL_TILT
    0     // TAIL_PAN
};

const double SLEEPING_ANGLE[] = {
    0,    // TILT1
    0,    // PAN
    0,    // TILT2

    59,   // RFLEG J1
    0,    // RFLEG J2
    30,   // RFLEG J3

    59,   // LFLEG J1
    0,    // LFLEG J2
    30,   // LFLEG J3

    -119, // RRLEG J1
    4,    // RRLEG J2
    122,  // RRLEG J3

    -119, // LRLEG J1
    4,    // LRLEG J2
    122,  // LRLEG J3

    0,    // TAIL_TILT
    0     // TAIL_PAN
};

struct PIDControl7Info {
    int  jointIndex;
    word pgain;
    word igain;
    word dgain;
    int  desiredValue;
    int  effectorCounter;
    int  sensorCounter;

    PIDControl7Info() {
        jointIndex      = RFLEG_J1;
        pgain           = 0;
        igain           = 0;
        dgain           = 0;
        desiredValue    = 0;
        effectorCounter = 0;
        sensorCounter   = 0;
    }
};

class PIDControl7 : public OObject {
public:
    PIDControl7();
    virtual ~PIDControl7() {}

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

    static const size_t NUM_COMMAND_VECTOR = 2;

    static const word   TILT1_PGAIN = 0x000a;
    static const word   TILT1_IGAIN = 0x0004;
    static const word   TILT1_DGAIN = 0x0002;

    static const word   PAN_PGAIN   = 0x0008;
    static const word   PAN_IGAIN   = 0x0002;
    static const word   PAN_DGAIN   = 0x0004;

    static const word   TILT2_PGAIN = 0x000a;
    static const word   TILT2_IGAIN = 0x0008;
    static const word   TILT2_DGAIN = 0x000c;
    
    static const word   J1_PGAIN    = 0x0010;
    static const word   J1_IGAIN    = 0x0004;
    static const word   J1_DGAIN    = 0x0001;

    static const word   J2_PGAIN    = 0x000a;
    static const word   J2_IGAIN    = 0x0004;
    static const word   J2_DGAIN    = 0x0001;

    static const word   J3_PGAIN    = 0x0010;
    static const word   J3_IGAIN    = 0x0004;
    static const word   J3_DGAIN    = 0x0001;

    static const word   TAIL_PGAIN  = 0x000a;
    static const word   TAIL_IGAIN  = 0x0004;
    static const word   TAIL_DGAIN  = 0x0004;

    static const word   PSHIFT      = 0x000e;
    static const word   ISHIFT      = 0x0002;
    static const word   DSHIFT      = 0x000f;

    static const int BROADBASE_MAX_COUNTER = 24; // 128ms * 24 = 3072ms
    static const int SLEEPING_MAX_COUNTER  = 24; // 128ms * 24 = 3072ms

    static const int STEP_INPUT_NUM_FRAMES = 4;
    static const int HOLD_TIME_NUM_FRAMES  = 256; // 8ms * 512 = 4096ms
    
    PIDControl7State pidControl7State;
    PIDControl7Info  pidControl7Info;

    OPrimitiveID jointID[NUM_JOINTS];
    RCRegion*    region[NUM_COMMAND_VECTOR];
    OJointValue* logData;

    bool      initSensorIndex;
    int       sensorIndex[NUM_JOINTS];
};

#endif // PIDControl7_h_DEFINED
