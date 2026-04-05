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

#ifndef MTNWALKAgent7_h_DEFINED
#define MTNWALKAgent7_h_DEFINED

#include "MTNAgent7.h"

class MTNWALKAgent7 : public MTNAgent7 {
public:
    MTNWALKAgent7() {}
    virtual ~MTNWALKAgent7() {}

    virtual bool AreYou(MoNetAgentID agent);
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

    static const word   J1_PGAIN    = 0x001c;
    static const word   J1_IGAIN    = 0x0008;
    static const word   J1_DGAIN    = 0x0001;

    static const word   J2_PGAIN    = 0x0014;
    static const word   J2_IGAIN    = 0x0004;
    static const word   J2_DGAIN    = 0x0001;

    static const word   J3_PGAIN    = 0x001c;
    static const word   J3_IGAIN    = 0x0008;
    static const word   J3_DGAIN    = 0x0001;

    static const word   TAIL_PGAIN  = 0x000a;
    static const word   TAIL_IGAIN  = 0x0004;
    static const word   TAIL_DGAIN  = 0x0004;

    static const word   PSHIFT      = 0x000e;
    static const word   ISHIFT      = 0x0002;
    static const word   DSHIFT      = 0x000f;
};

#endif // MTNWALKAgent7_h_DEFINED
