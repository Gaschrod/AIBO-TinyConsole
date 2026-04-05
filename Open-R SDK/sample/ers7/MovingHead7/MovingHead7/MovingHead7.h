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

#ifndef MovingHead7_h_DEFINED
#define MovingHead7_h_DEFINED

#include <OPENR/OObject.h>
#include <OPENR/OSubject.h>
#include <OPENR/OObserver.h>
#include "def.h"

enum MovingHead7State {
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
    "PRM:/r1/c1-Joint2:11",      // TILT1
    "PRM:/r1/c1/c2-Joint2:12",   // PAN
    "PRM:/r1/c1/c2/c3-Joint2:13" // TILT2
};

class MovingHead7 : public OObject {
public:
    MovingHead7();
    virtual ~MovingHead7() {}

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
    static const int    TILT1_INDEX        = 0;
    static const int    PAN_INDEX          = 1;
    static const int    TILT2_INDEX        = 2;

    static const double TILT1_ZERO_POS     = 0.0;
    static const double PAN_ZERO_POS       = 0.0;
    static const double TILT2_ZERO_POS     = 0.0;
    static const double SWING_AMPLITUDE    = 80.0;

    static const word   TILT1_PGAIN        = 0x000a;
    static const word   TILT1_IGAIN        = 0x0004;
    static const word   TILT1_DGAIN        = 0x0002;

    static const word   PAN_PGAIN          = 0x0008;
    static const word   PAN_IGAIN          = 0x0002;
    static const word   PAN_DGAIN          = 0x0004;

    static const word   TILT2_PGAIN        = 0x0008;
    static const word   TILT2_IGAIN        = 0x0004;
    static const word   TILT2_DGAIN        = 0x0002;

    static const word   PSHIFT             = 0x000e;
    static const word   ISHIFT             = 0x0002;
    static const word   DSHIFT             = 0x000f;

    static const int ZERO_POS_MAX_COUNTER  =  16; // 128ms * 16 = 2048ms
    static const int MAX_PHASE             = 128; // phase [0:127]
    
    MovingHead7State  movingHeadState;
    OPrimitiveID      jointID[NUM_JOINTS];
    RCRegion*         region[NUM_COMMAND_VECTOR];
};

#endif // MovingHead7_h_DEFINED
