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

#ifndef _OMLE2Commander_h_DEFINED
#define _OMLE2Commander_h_DEFINED

#include <OPENR/OPENR.h>
#include <OMWares/LE2CmdList.h>

struct LE2Command;

class OMLE2Commander 
{
private:
    LE2Command* le2cmd;
    int         flag;

public:
    OMLE2Commander();
    ~OMLE2Commander();

    OStatus ProvideCommand(void* omLE2Cmd);
    OStatus Generate_Command(int command, int times);
};

#endif  //  _OMLE2Commander_h_DEFINED
