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

#ifndef MovingLegs7_h_DEFINED
#define MovingLegs7_h_DEFINED

#include <OPENR/OObject.h>
#include <OPENR/OSubject.h>
#include <OPENR/OObserver.h>
#include "def.h"

enum MovingLegs7State {
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
    "PRM:/r3/c1/c2/c3-Joint2:33"  // LRLEG J3
};

const double BROADBASE_ANGLE[] = {
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
    30    // LRLEG J3
};

const double SLEEPING_ANGLE[] = {
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
    122   // LRLEG J3
};

class MovingLegs7 : public OObject {
public:
    MovingLegs7();
    virtual ~MovingLegs7() {}

    OSubject*  subject[numOfSubject];
    OObserver* observer[numOfObserver];

    virtual OStatus DoInit   (const OSystemEvent& event);
    virtual OStatus DoStart  (const OSystemEvent& event);
    virtual OStatus DoStop   (const OSystemEvent& event);
    virtual OStatus DoDestroy(const OSystemEvent& event);
    
    void Ready(const OReadyEvent& event);

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

    static const word   J1_PGAIN = 0x0010;
    static const word   J1_IGAIN = 0x0004;
    static const word   J1_DGAIN = 0x0001;

    static const word   J2_PGAIN = 0x000a;
    static const word   J2_IGAIN = 0x0004;
    static const word   J2_DGAIN = 0x0001;

    static const word   J3_PGAIN = 0x0010;
    static const word   J3_IGAIN = 0x0004;
    static const word   J3_DGAIN = 0x0001;

    static const word   PSHIFT   = 0x000e;
    static const word   ISHIFT   = 0x0002;
    static const word   DSHIFT   = 0x000f;

    static const int BROADBASE_MAX_COUNTER = 24; // 128ms * 24 = 3072ms
    static const int SLEEPING_MAX_COUNTER  = 24; // 128ms * 24 = 3072ms
    
    MovingLegs7State  movingLegsState;
    OPrimitiveID      jointID[NUM_JOINTS];
    RCRegion*         region[NUM_COMMAND_VECTOR];
};

#endif // MovingLegs7_h_DEFINED
