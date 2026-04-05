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

#ifndef MovingHead_h_DEFINED
#define MovingHead_h_DEFINED

#include <OPENR/OObject.h>
#include <OPENR/OSubject.h>
#include <OPENR/OObserver.h>
#include "def.h"

enum MovingHeadState {
    MHS_IDLE,
    MHS_START,
    MHS_ADJUSTING_DIFF_JOINT_VALUE,
    MHS_MOVING_TO_ZERO_POS,
    MHS_SWING_HEAD
};

enum MovingResult {
    MOVING_CONT,
    MOVING_FINISH
};

static const char* const JOINT_LOCATOR[] = {
    "PRM:/r1/c1-Joint2:j1",      // TILT
    "PRM:/r1/c1/c2-Joint2:j2",   // PAN
    "PRM:/r1/c1/c2/c3-Joint2:j3" // ROLL
};

class MovingHead : public OObject {
public:
    MovingHead();
    virtual ~MovingHead() {}

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
    MovingResult  MoveToZeroPos();
    MovingResult  SwingHead();

    RCRegion* FindFreeRegion();
    void SetJointValue(RCRegion* rgn, int idx, double start, double end);
    void SetJointValue(RCRegion* rgn, int idx, int phase);

    static const size_t NUM_COMMAND_VECTOR = 2;
    static const size_t NUM_JOINTS         = 3;
    static const int    TILT_INDEX         = 0;
    static const int    PAN_INDEX          = 1;
    static const int    ROLL_INDEX         = 2;

    static const word   TILT_PGAIN         = 0x000a;
    static const word   TILT_IGAIN         = 0x0008;
    static const word   TILT_DGAIN         = 0x000c;

    static const word   PAN_PGAIN          = 0x000d;
    static const word   PAN_IGAIN          = 0x0008;
    static const word   PAN_DGAIN          = 0x000b;

    static const word   ROLL_PGAIN         = 0x000a;
    static const word   ROLL_IGAIN         = 0x0008;
    static const word   ROLL_DGAIN         = 0x000c;

    static const word   PSHIFT             = 0x000e;
    static const word   ISHIFT             = 0x0002;
    static const word   DSHIFT             = 0x000f;

    static const int ZERO_POS_MAX_COUNTER  =  16; // 128ms * 16 = 2048ms
    static const int MAX_PHASE             = 128; // phase [0:127]
    
    MovingHeadState  movingHeadState;
    OPrimitiveID     jointID[NUM_JOINTS];
    RCRegion*        region[NUM_COMMAND_VECTOR];
};

#endif // MovingHead_h_DEFINED
