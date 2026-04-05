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

#ifndef _SE_CmdList_h_DEFINED
#define _SE_CmdList_h_DEFINED

typedef unsigned int  SE_COMMAND;

const SE_COMMAND  TAIL_HOME     = 0x0001;
const SE_COMMAND  TAIL_MOVE     = 0x0002;
const SE_COMMAND  TAIL_SWING    = 0x0003;

#endif // _SE_CmdList_h_DEFINED
