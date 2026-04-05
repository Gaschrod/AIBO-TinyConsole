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

#ifndef BallTrackingHeadData_h
#define BallTrackingHeadData_h

#include <OPENR/OPENR.h>

enum BallTrackingHeadStatus {
    BTH_SUCCESS,
    BTH_FAIL,
    BTH_BUSY,
    BTH_INVALID_ARG
};

enum BallTrackingHeadCommandType {
    BTHCMD_MOVE_TO_ZERO_POS,
    BTHCMD_MOVE_TO_SLEEPING,
    BTHCMD_PLAY_FOUND_SOUND,
    BTHCMD_PLAY_LOST_SOUND
};

struct BallTrackingHeadCommand {
    BallTrackingHeadCommandType type;
    
    BallTrackingHeadCommand(BallTrackingHeadCommandType t) : type(t) {}
};

struct BallTrackingHeadResult {
    BallTrackingHeadStatus      status;

    BallTrackingHeadResult(BallTrackingHeadStatus s) : status(s) {}
};

#endif // BallTrackingHeadData_h

