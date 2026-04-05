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

#ifndef NeutralAgent7_h_DEFINED
#define NeutralAgent7_h_DEFINED

#include <MoNetData.h>
#include "MoNetAgent.h"

enum NeutralAgent7State {
    NAS7_IDLE,
    NAS7_ADJUSTING_DIFF_JOINT_VALUE,
    NAS7_MOVING_TO_BROADBASE,
    NAS7_MOVING_TO_SLEEPING
};

class NeutralAgent7 : public MoNetAgent {
public:
    NeutralAgent7();
    virtual ~NeutralAgent7() {}

    virtual bool AreYou(MoNetAgentID agent);
    virtual void Init(ODA* oda);
    virtual void NotifyCommand(const MoNetAgentCommand& command,
                               MoNetAgentResult* result);
    virtual void ReadyEffector(const MoNetAgentCommand& command,
                               MoNetAgentResult* result);
    virtual void SetJointGain();

private:
    static const word   TILT1_PGAIN = 0x000a;
    static const word   TILT1_IGAIN = 0x0004;
    static const word   TILT1_DGAIN = 0x0002;

    static const word   PAN_PGAIN   = 0x0008;
    static const word   PAN_IGAIN   = 0x0002;
    static const word   PAN_DGAIN   = 0x0004;

    static const word   TILT2_PGAIN = 0x0008;
    static const word   TILT2_IGAIN = 0x0004;
    static const word   TILT2_DGAIN = 0x0002;

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

    static const int BROADBASE_MAX_COUNTER = 96; // 32ms * 96 = 3072ms
    static const int SLEEPING_MAX_COUNTER  = 96; // 32ms * 96 = 3072ms

    MoNetStatus AdjustDiffJointValue(OVRSyncKey syncKey);
    MoNetStatus MoveToBroadBase();
    MoNetStatus MoveToSleeping();

    void SetJointValue(RCRegion* rgn, int idx, double start, double end);

    NeutralAgent7State neutralAgentState;
};

#endif // NeutralAgent7_h_DEFINED
