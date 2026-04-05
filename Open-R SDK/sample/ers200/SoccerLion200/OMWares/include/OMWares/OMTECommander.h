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

#ifndef _OMTECommander_h_DEFINED
#define _OMTECommander_h_DEFINED

#include <OPENR/OPENR.h>
#include <OMWares/TECmdList.h>

struct TECommand;

class OMTECommander 
{
private:
    TECommand*  tecmd;
    int         flag;

public:
    OMTECommander();
    ~OMTECommander();

    OStatus ProvideCommand(void* omTECmd);
    OStatus Generate_Command(int command, int time);
    OStatus Generate_Command(int command);
};

#endif  //  _OMTECommander_h_DEFINED
