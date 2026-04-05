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

#ifndef _TE_CmdList_h_DEFINED
#define _TE_CmdList_h_DEFINED

typedef unsigned int  TE_COMMAND;

const TE_COMMAND  MOVE_TO_HOME    = 0x0001;
const TE_COMMAND  COLOR_TRACKING  = 0x0002;
const TE_COMMAND  MOTION_TRACKING = 0x0003;
const TE_COMMAND  MOVE_TO_POINT   = 0x0004;
const TE_COMMAND  MOVE_WITH_DIFF  = 0x0005;

#endif // _TE_CmdList_h_DEFINED
