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

#ifndef NeutralAgent_h_DEFINED
#define NeutralAgent_h_DEFINED

#include <MoNetData.h>
#include "MoNetAgent.h"

enum NeutralAgentState {
    NAS_IDLE,
    NAS_ADJUSTING_DIFF_JOINT_VALUE,
    NAS_MOVING_TO_BROADBASE,
    NAS_MOVING_TO_SLEEPING
};

class NeutralAgent : public MoNetAgent {
public:
    NeutralAgent();
    virtual ~NeutralAgent() {}

    virtual bool AreYou(MoNetAgentID agent);
    virtual void Init(ODA* oda);
    virtual void NotifyCommand(const MoNetAgentCommand& command,
                               MoNetAgentResult* result);
    virtual void ReadyEffector(const MoNetAgentCommand& command,
                               MoNetAgentResult* result);
    virtual void SetJointGain();

private:
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

    static const int BROADBASE_MAX_COUNTER = 192; // 32ms * 192 = 6144ms
    static const int SLEEPING_MAX_COUNTER  = 192; // 32ms * 192 = 6144ms

    MoNetStatus AdjustDiffJointValue(OVRSyncKey syncKey);
    MoNetStatus MoveToBroadBase();
    MoNetStatus MoveToSleeping();

    void SetJointValue(RCRegion* rgn, int idx, double start, double end);

    NeutralAgentState neutralAgentState;
};

#endif // NeutralAgent_h_DEFINED
