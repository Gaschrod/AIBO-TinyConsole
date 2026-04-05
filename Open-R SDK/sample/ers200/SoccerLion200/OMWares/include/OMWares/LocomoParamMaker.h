//
// Copyright 1999,2002 Sony Corporation 
//
// Permission to use, copy, modify, and redistribute this software for
// non-commercial use is hereby granted.
//
// This software is provided "as is" without warranty of any kind,
// either expressed or implied, including but not limited to the
// implied warranties of fitness for a particular purpose.
//

#ifndef _LocomoParamMaker_h_DEFINED
#define _LocomoParamMaker_h_DEFINED

#include <OPENR/OStatus.h>
#include <OMWares/LE2_S_CmdList.h>

struct LocomoParamStruct;

class LocomoParamMaker
{
private:
    LocomoParamStruct*  param;
public:
    LocomoParamMaker();
    ~LocomoParamMaker();

    OStatus MakeParam(double     transvel,  // Translation Velocity  [mm/sec]
                      double     transdir,  // Translation Direction [radians]
                      double     rotvel,    // Rotation Velocity     [rad/sec]
                      double     rotcntY,   // Rotation Center of Y-axis [mm]
                      double     rotcntZ,   // Rotation Center of Z-axis [mm]
                      WALK_STYLE style);    // Walking Style (Look LE2_S_CmdList.h)
    //  These Walking Style is available.
    //     const WALK_STYLE  DEFAULT_WALKING
    //     const WALK_STYLE   STABLE_WALKING
    //     const WALK_STYLE     SLOW_WALKING
    //     const WALK_STYLE     FAST_WALKING
    //     const WALK_STYLE  OFFLOAD_WALKING
    //     const WALK_STYLE     BABY_WALKING
    //     const WALK_STYLE   LIZARD_WALKING
    //     const WALK_STYLE    CREEP_WALKING
    //     const WALK_STYLE     TROT_WALKING
    //     const WALK_STYLE    RIGHT_TURN   
    //     const WALK_STYLE     LEFT_TURN   
    //     const WALK_STYLE     SKIP_BACK   

    LocomoParamStruct*  ProvideLocomoParam(void);
};

#endif  // _LocomoParamMaker_h_
