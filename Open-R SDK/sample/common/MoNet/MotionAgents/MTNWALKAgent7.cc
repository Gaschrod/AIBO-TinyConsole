//
// Copyright 2004 Sony Corporation 
//
// Permission to use, copy, modify, and redistribute this software for
// non-commercial use is hereby granted.
//
// This software is provided "as is" without warranty of any kind,
// either expressed or implied, including but not limited to the
// implied warranties of fitness for a particular purpose.
//

#include <OPENR/OPENRAPI.h>
#include "MTNWALKAgent7.h"

bool
MTNWALKAgent7::AreYou(MoNetAgentID agent)
{
    OSYSDEBUG(("MTNWALKAgent7::AreYou(%d)\n", agent));
    return (agent == monetagentMTNWALK) ? true : false;
}

void
MTNWALKAgent7::SetJointGain()
{
    OSYSDEBUG(("MTNWALKAgent7::SetJointGain()\n"));

    OPrimitiveID tilt1ID = moNetAgentManager->PrimitiveID(HEAD_TILT1);
    OPrimitiveID panID   = moNetAgentManager->PrimitiveID(HEAD_PAN);
    OPrimitiveID tilt2ID = moNetAgentManager->PrimitiveID(HEAD_TILT2);

    OPENR::EnableJointGain(tilt1ID);
    OPENR::SetJointGain(tilt1ID,
                        TILT1_PGAIN, TILT1_IGAIN, TILT1_DGAIN,
                        PSHIFT, ISHIFT, DSHIFT);
    
    OPENR::EnableJointGain(panID);
    OPENR::SetJointGain(panID,
                        PAN_PGAIN, PAN_IGAIN, PAN_DGAIN,
                        PSHIFT, ISHIFT, DSHIFT);

    OPENR::EnableJointGain(tilt2ID);
    OPENR::SetJointGain(tilt2ID,
                        TILT2_PGAIN, TILT2_IGAIN, TILT2_DGAIN,
                        PSHIFT, ISHIFT, DSHIFT);

    int base = RFLEG_J1;
    for (int i = 0; i < 4; i++) {

        OPrimitiveID j1ID = moNetAgentManager->PrimitiveID(base + 3 * i);
        OPrimitiveID j2ID = moNetAgentManager->PrimitiveID(base + 3 * i + 1);
        OPrimitiveID j3ID = moNetAgentManager->PrimitiveID(base + 3 * i + 2);
        
        OPENR::EnableJointGain(j1ID);        
        OPENR::SetJointGain(j1ID,
                            J1_PGAIN, J1_IGAIN, J1_DGAIN,
                            PSHIFT, ISHIFT, DSHIFT);

        OPENR::EnableJointGain(j2ID);
        OPENR::SetJointGain(j2ID,
                            J2_PGAIN, J2_IGAIN, J2_DGAIN,
                            PSHIFT, ISHIFT, DSHIFT);

        OPENR::EnableJointGain(j3ID);
        OPENR::SetJointGain(j3ID,
                            J3_PGAIN, J3_IGAIN, J3_DGAIN,
                            PSHIFT, ISHIFT, DSHIFT);
    }

    OPrimitiveID tailtiltID = moNetAgentManager->PrimitiveID(TAIL_TILT);
    OPrimitiveID tailpanID  = moNetAgentManager->PrimitiveID(TAIL_PAN);

    OPENR::EnableJointGain(tailtiltID);
    OPENR::SetJointGain(tailtiltID,
                        TAIL_PGAIN, TAIL_IGAIN, TAIL_DGAIN,
                        PSHIFT, ISHIFT, DSHIFT);

    OPENR::EnableJointGain(tailpanID);
    OPENR::SetJointGain(tailpanID,
                        TAIL_PGAIN, TAIL_IGAIN, TAIL_DGAIN,
                        PSHIFT, ISHIFT, DSHIFT);
}
