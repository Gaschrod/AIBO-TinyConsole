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

#ifndef _LE2_SCmdList_h_DEFINED
#define _LE2_SCmdList_h_DEFINED

typedef unsigned int  WALK_STYLE;
typedef unsigned int  WALK_DIR;

const WALK_STYLE     STOP_WALKING = 0x0000;
const WALK_STYLE  DEFAULT_WALKING = 0x0100;
const WALK_STYLE   STABLE_WALKING = 0x0200;
const WALK_STYLE     SLOW_WALKING = 0x0300;
const WALK_STYLE     FAST_WALKING = 0x0400;
const WALK_STYLE  OFFLOAD_WALKING = 0x0500;
const WALK_STYLE     BABY_WALKING = 0x0600;
const WALK_STYLE   LIZARD_WALKING = 0x0700;
const WALK_STYLE    CREEP_WALKING = 0x0800;
const WALK_STYLE     TROT_WALKING = 0x0900;
const WALK_STYLE    RIGHT_TURN    = 0xE100;
const WALK_STYLE     LEFT_TURN    = 0xE200;
const WALK_STYLE    RIGHT_KICK    = 0xF100;
const WALK_STYLE     LEFT_KICK    = 0xF200;
const WALK_STYLE     SKIP_BACK    = 0xF300;
const WALK_STYLE ADAPTIVE_WALKING = 0xFE00;
const WALK_STYLE   CUSTOM_WALKING = 0xFF00;

const WALK_DIR   WALK_FORWARD      = 0x0001;
const WALK_DIR   WALK_RIGHTFORWARD = 0x0002;
const WALK_DIR   WALK_LEFTFORWARD  = 0x0003;
const WALK_DIR   WALK_RIGHT        = 0x0004;
const WALK_DIR   WALK_LEFT         = 0x0005;
const WALK_DIR   WALK_BACKWARD     = 0x0006;

#endif // _LE2CmdList_h_DEFINED
