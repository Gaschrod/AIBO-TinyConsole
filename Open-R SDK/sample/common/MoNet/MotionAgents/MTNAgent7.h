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

#ifndef MTNAgent7_h_DEFINED
#define MTNAgent7_h_DEFINED

#include <OPENR/ODataArchive.h>
#include <MoNetData.h>
#include "MoNetAgent.h"
#include "MTN.h"

class MTNAgent7 : public MoNetAgent {
public:
    MTNAgent7();
    virtual ~MTNAgent7() {}

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

    MoNetStatus SetPrimitiveID(OCommandVectorData* cmdVec, char* robotDesign);
    MoNetStatus Move(OVRSyncKey syncKey);

    ODA* motionODA;
    MTN  mtn;
};

#endif // MTNAgent7_h_DEFINED
