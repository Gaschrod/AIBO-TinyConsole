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

#ifndef _MoNetMessageMaker_h_DEFINED
#define _MoNetMessageMaker_h_DEFINED

#include <OPENR/OStatus.h>
#include <OMWares/OMoNetMessage.h>
#include <OMWares/MMM_CommandList.h>

class MoNetMessageMaker
{
private:
    OMoNetMessage  msg_;

public:
    MoNetMessageMaker();
    ~MoNetMessageMaker();

    OStatus MakePlayBackCommand(MMM_MainCmd main, MMM_SubCmd sub, char* motion);
    OStatus MakePlayBackCommand(MMM_MainCmd main, MMM_SubCmd sub, char* motion, char* start, char* end);

    OStatus MakeStepWalkingCommand(MMM_MainCmd main, MMM_SubCmd sub, unsigned int times=1);
    OStatus MakeWalkingCommand(MMM_MainCmd main, MMM_SubCmd sub, unsigned int times=1);
    OStatus MakeKickingCommand(MMM_MainCmd main, MMM_SubCmd sub, double dir);

    OStatus MakeHeadCommand(MMM_MainCmd main, MMM_SubCmd sub);
    OStatus MakeHeadCommand(MMM_MainCmd main, MMM_SubCmd sub, unsigned int time);
    OStatus MakeHeadCommand(MMM_MainCmd main, MMM_SubCmd sub, double tilt, double pan, double roll);
    OStatus MakeHeadCommand(MMM_MainCmd main, MMM_SubCmd sub, double tilt, double pan, double roll, unsigned int time);

    OStatus MakeTailCommand(MMM_MainCmd main, MMM_SubCmd sub);
    OStatus MakeTailCommand(MMM_MainCmd main, MMM_SubCmd sub, unsigned int time);
    OStatus MakeTailCommand(MMM_MainCmd main, MMM_SubCmd sub, double tilt, double pan);
    OStatus MakeTailCommand(MMM_MainCmd main, MMM_SubCmd sub, double tilt, double pan, unsigned int time);

    OMoNetMessage*  ProvideMoNetMessage(void);
};

#endif  // _MoNetMessageMaker_h_
