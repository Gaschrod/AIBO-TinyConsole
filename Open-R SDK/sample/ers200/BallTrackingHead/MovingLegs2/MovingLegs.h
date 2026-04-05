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

#ifndef MovingLegs_h_DEFINED
#define MovingLegs_h_DEFINED

#include <OPENR/OObject.h>
#include <OPENR/OSubject.h>
#include <OPENR/OObserver.h>
#include "def.h"

enum MovingLegsState {
    MLS_IDLE,
    MLS_START,
    MLS_ADJUSTING_DIFF_JOINT_VALUE,
    MLS_MOVING_TO_BROADBASE,
    MLS_MOVING_TO_SLEEPING
};

enum MovingResult {
    MOVING_CONT,
    MOVING_FINISH
};

static const char* const JOINT_LOCATOR[] = {
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

class MovingLegs : public OObject {
public:
    MovingLegs();
    virtual ~MovingLegs() {}

    OSubject*  subject[numOfSubject];
    OObserver* observer[numOfObserver];

    virtual OStatus DoInit   (const OSystemEvent& event);
    virtual OStatus DoStart  (const OSystemEvent& event);
    virtual OStatus DoStop   (const OSystemEvent& event);
    virtual OStatus DoDestroy(const OSystemEvent& event);
    
    void NotifyCommand(const ONotifyEvent& event);
    void ReadyMove(const OReadyEvent& event);

private:
    void          OpenPrimitives();
    void          NewCommandVectorData();
    void          SetJointGain();
    MovingResult  AdjustDiffJointValue();
    MovingResult  MoveToBroadBase();
    MovingResult  MoveToSleeping();

    RCRegion* FindFreeRegion();
    void SetJointValue(RCRegion* rgn, int idx, double start, double end);

    static const size_t NUM_JOINTS         = 12;
    static const size_t NUM_COMMAND_VECTOR = 2;

    static const word   J1_PGAIN = 0x0016;
    static const word   J1_IGAIN = 0x0004;
    static const word   J1_DGAIN = 0x0008;

    static const word   J2_PGAIN = 0x0014;
    static const word   J2_IGAIN = 0x0004;
    static const word   J2_DGAIN = 0x0006;

    static const word   J3_PGAIN = 0x0023;
    static const word   J3_IGAIN = 0x0004;
    static const word   J3_DGAIN = 0x0005;

    static const word   PSHIFT   = 0x000e;
    static const word   ISHIFT   = 0x0002;
    static const word   DSHIFT   = 0x000f;

    static const int BROADBASE_MAX_COUNTER = 48; // 128ms * 48 = 6144ms
    static const int SLEEPING_MAX_COUNTER  = 48; // 128ms * 48 = 6144ms
    
    MovingLegsState  movingLegsState;
    OPrimitiveID     jointID[NUM_JOINTS];
    RCRegion*        region[NUM_COMMAND_VECTOR];
};

#endif // MovingLegs_h_DEFINED
