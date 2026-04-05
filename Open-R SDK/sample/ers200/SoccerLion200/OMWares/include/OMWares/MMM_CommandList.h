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

#ifndef _MMM_CommandList_h_DEFINED
#define _MMM_CommandList_h_DEFINED

#include <OMWares/OMoNetMessage.h>
#include <OMWares/LE2_S_CmdList.h>
#include <OMWares/TE_CmdList.h>
#include <OMWares/SE_CmdList.h>

typedef unsigned int MMM_MainCmd;
typedef unsigned int MMM_SubCmd;

//
//  MMM Main Command
//
//  << MMM Main Command to oMoNet >>
const MMM_MainCmd  mmmmcMoveNow                = (MMM_MainCmd)oMoveNow;
const MMM_MainCmd  mmmmcMoveAfter              = (MMM_MainCmd)oMoveAfter;
const MMM_MainCmd  mmmmcMoveStartPosNow        = (MMM_MainCmd)oMoveStartPosNow;
const MMM_MainCmd  mmmmcMoveAtertPosAfter      = (MMM_MainCmd)oMoveStartPosAfter;
const MMM_MainCmd  mmmmcStopMoveNormal         = (MMM_MainCmd)oStopMoveNormal;
const MMM_MainCmd  mmmmcStopMoveEmergency      = (MMM_MainCmd)oStopMoveEmergency;
const MMM_MainCmd  mmmmcReportPosture          = (MMM_MainCmd)oReportPosture;
//  << MMM Main Command to MoNetAgent >>
const MMM_MainCmd  mmmmcStartMotion            = (MMM_MainCmd)oStartMotion;
const MMM_MainCmd  mmmmcStopMotion             = (MMM_MainCmd)oStopMotion;
const MMM_MainCmd  mmmmcKillMotion             = (MMM_MainCmd)oKillMotion;

//
//  MMM Sub Command
//
//  << MMM Sub Command for PlayBack >>
const MMM_SubCmd  mmmscPlayBackAll             = (MMM_SubCmd)oAll;
const MMM_SubCmd  mmmscPlayBackHead            = (MMM_SubCmd)oHead;
const MMM_SubCmd  mmmscPlayBackLegs            = (MMM_SubCmd)oLegs;
const MMM_SubCmd  mmmscPlayBackTail            = (MMM_SubCmd)oTail;
//  << MMM Sub Command for Walking >>
const MMM_SubCmd  mmmscWalkStop                = (MMM_SubCmd)0x00010000;
//    Spetial 
const MMM_SubCmd  mmmscWalkCustom              = (MMM_SubCmd)(mmmscWalkStop + CUSTOM_WALKING);
//    Kick Command
const MMM_SubCmd  mmmscRightKick               = (MMM_SubCmd)(mmmscWalkStop + RIGHT_KICK);
const MMM_SubCmd  mmmscLeftKick                = (MMM_SubCmd)(mmmscWalkStop + LEFT_KICK);
//    Turn Command
const MMM_SubCmd  mmmscWalkTurnRight           = (MMM_SubCmd)(mmmscWalkStop + RIGHT_TURN);
const MMM_SubCmd  mmmscWalkTurnLeft            = (MMM_SubCmd)(mmmscWalkStop + LEFT_TURN);
const MMM_SubCmd  mmmscSkipBack                = (MMM_SubCmd)(mmmscWalkStop + SKIP_BACK);
//    Default Walking Command
const MMM_SubCmd  mmmscWalkForward             = (MMM_SubCmd)(mmmscWalkStop + DEFAULT_WALKING + WALK_FORWARD);
const MMM_SubCmd  mmmscWalkRightForward        = (MMM_SubCmd)(mmmscWalkStop + DEFAULT_WALKING + WALK_RIGHTFORWARD);
const MMM_SubCmd  mmmscWalkLeftForward         = (MMM_SubCmd)(mmmscWalkStop + DEFAULT_WALKING + WALK_LEFTFORWARD);
const MMM_SubCmd  mmmscWalkRight               = (MMM_SubCmd)(mmmscWalkStop + DEFAULT_WALKING + WALK_RIGHT);
const MMM_SubCmd  mmmscWalkLeft                = (MMM_SubCmd)(mmmscWalkStop + DEFAULT_WALKING + WALK_LEFT);
const MMM_SubCmd  mmmscWalkBackward            = (MMM_SubCmd)(mmmscWalkStop + DEFAULT_WALKING + WALK_BACKWARD);
//    Stable Walking Command
const MMM_SubCmd  mmmscStableWalkForward       = (MMM_SubCmd)(mmmscWalkStop + STABLE_WALKING + WALK_FORWARD);
const MMM_SubCmd  mmmscStableWalkRightForward  = (MMM_SubCmd)(mmmscWalkStop + STABLE_WALKING + WALK_RIGHTFORWARD);
const MMM_SubCmd  mmmscStableWalkLeftForward   = (MMM_SubCmd)(mmmscWalkStop + STABLE_WALKING + WALK_LEFTFORWARD);
const MMM_SubCmd  mmmscStableWalkRight         = (MMM_SubCmd)(mmmscWalkStop + STABLE_WALKING + WALK_RIGHT);
const MMM_SubCmd  mmmscStableWalkLeft          = (MMM_SubCmd)(mmmscWalkStop + STABLE_WALKING + WALK_LEFT);
const MMM_SubCmd  mmmscStableWalkBackward      = (MMM_SubCmd)(mmmscWalkStop + STABLE_WALKING + WALK_BACKWARD);
//    Slow Walking Command
const MMM_SubCmd  mmmscSlowWalkForward         = (MMM_SubCmd)(mmmscWalkStop + SLOW_WALKING + WALK_FORWARD);
const MMM_SubCmd  mmmscSlowWalkRightForward    = (MMM_SubCmd)(mmmscWalkStop + SLOW_WALKING + WALK_RIGHTFORWARD);
const MMM_SubCmd  mmmscSlowWalkLeftForward     = (MMM_SubCmd)(mmmscWalkStop + SLOW_WALKING + WALK_LEFTFORWARD);
const MMM_SubCmd  mmmscSlowWalkRight           = (MMM_SubCmd)(mmmscWalkStop + SLOW_WALKING + WALK_RIGHT);
const MMM_SubCmd  mmmscSlowWalkLeft            = (MMM_SubCmd)(mmmscWalkStop + SLOW_WALKING + WALK_LEFT);
const MMM_SubCmd  mmmscSlowWalkBackward        = (MMM_SubCmd)(mmmscWalkStop + SLOW_WALKING + WALK_BACKWARD);
//    OffLoad Walking Command
const MMM_SubCmd  mmmscOffLoadWalkForward      = (MMM_SubCmd)(mmmscWalkStop + OFFLOAD_WALKING + WALK_FORWARD);
const MMM_SubCmd  mmmscOffLoadWalkRightForward = (MMM_SubCmd)(mmmscWalkStop + OFFLOAD_WALKING + WALK_RIGHTFORWARD);
const MMM_SubCmd  mmmscOffLoadWalkLeftForward  = (MMM_SubCmd)(mmmscWalkStop + OFFLOAD_WALKING + WALK_LEFTFORWARD);
const MMM_SubCmd  mmmscOffLoadWalkRight        = (MMM_SubCmd)(mmmscWalkStop + OFFLOAD_WALKING + WALK_RIGHT);
const MMM_SubCmd  mmmscOffLoadWalkLeft         = (MMM_SubCmd)(mmmscWalkStop + OFFLOAD_WALKING + WALK_LEFT);
const MMM_SubCmd  mmmscOffLoadWalkBackward     = (MMM_SubCmd)(mmmscWalkStop + OFFLOAD_WALKING + WALK_BACKWARD);
//    Fast Walking Command
const MMM_SubCmd  mmmscFastWalkForward         = (MMM_SubCmd)(mmmscWalkStop + FAST_WALKING + WALK_FORWARD);
const MMM_SubCmd  mmmscFastWalkRightForward    = (MMM_SubCmd)(mmmscWalkStop + FAST_WALKING + WALK_RIGHTFORWARD);
const MMM_SubCmd  mmmscFastWalkLeftForward     = (MMM_SubCmd)(mmmscWalkStop + FAST_WALKING + WALK_LEFTFORWARD);
const MMM_SubCmd  mmmscFastWalkRight           = (MMM_SubCmd)(mmmscWalkStop + FAST_WALKING + WALK_RIGHT);
const MMM_SubCmd  mmmscFastWalkLeft            = (MMM_SubCmd)(mmmscWalkStop + FAST_WALKING + WALK_LEFT);
const MMM_SubCmd  mmmscFastWalkBackward        = (MMM_SubCmd)(mmmscWalkStop + FAST_WALKING + WALK_BACKWARD);
//    Lizard Walking Command 
const MMM_SubCmd  mmmscLizardWalkForward       = (MMM_SubCmd)(mmmscWalkStop + LIZARD_WALKING + WALK_FORWARD);
const MMM_SubCmd  mmmscLizardWalkRightForward  = (MMM_SubCmd)(mmmscWalkStop + LIZARD_WALKING + WALK_RIGHTFORWARD);
const MMM_SubCmd  mmmscLizardWalkLeftForward   = (MMM_SubCmd)(mmmscWalkStop + LIZARD_WALKING + WALK_LEFTFORWARD);
const MMM_SubCmd  mmmscLizardWalkRight         = (MMM_SubCmd)(mmmscWalkStop + LIZARD_WALKING + WALK_RIGHT);
const MMM_SubCmd  mmmscLizardWalkLeft          = (MMM_SubCmd)(mmmscWalkStop + LIZARD_WALKING + WALK_LEFT);
const MMM_SubCmd  mmmscLizardWalkBackward      = (MMM_SubCmd)(mmmscWalkStop + LIZARD_WALKING + WALK_BACKWARD);
//    Baby Walking Command 
const MMM_SubCmd  mmmscBabyWalkForward         = (MMM_SubCmd)(mmmscWalkStop + BABY_WALKING + WALK_FORWARD);
const MMM_SubCmd  mmmscBabyWalkRightForward    = (MMM_SubCmd)(mmmscWalkStop + BABY_WALKING + WALK_RIGHTFORWARD);
const MMM_SubCmd  mmmscBabyWalkLeftForward     = (MMM_SubCmd)(mmmscWalkStop + BABY_WALKING + WALK_LEFTFORWARD);
const MMM_SubCmd  mmmscBabyWalkRight           = (MMM_SubCmd)(mmmscWalkStop + BABY_WALKING + WALK_RIGHT);
const MMM_SubCmd  mmmscBabyWalkLeft            = (MMM_SubCmd)(mmmscWalkStop + BABY_WALKING + WALK_LEFT);
const MMM_SubCmd  mmmscBabyWalkBackward        = (MMM_SubCmd)(mmmscWalkStop + BABY_WALKING + WALK_BACKWARD);
//    Creep Walking Command 
const MMM_SubCmd  mmmscCreepWalkForward        = (MMM_SubCmd)(mmmscWalkStop + CREEP_WALKING + WALK_FORWARD);
const MMM_SubCmd  mmmscCreepWalkRightForward   = (MMM_SubCmd)(mmmscWalkStop + CREEP_WALKING + WALK_RIGHTFORWARD);
const MMM_SubCmd  mmmscCreepWalkLeftForward    = (MMM_SubCmd)(mmmscWalkStop + CREEP_WALKING + WALK_LEFTFORWARD);
const MMM_SubCmd  mmmscCreepWalkRight          = (MMM_SubCmd)(mmmscWalkStop + CREEP_WALKING + WALK_RIGHT);
const MMM_SubCmd  mmmscCreepWalkLeft           = (MMM_SubCmd)(mmmscWalkStop + CREEP_WALKING + WALK_LEFT);
const MMM_SubCmd  mmmscCreepWalkBackward       = (MMM_SubCmd)(mmmscWalkStop + CREEP_WALKING + WALK_BACKWARD);
//    Trot Walking Command 
const MMM_SubCmd  mmmscTrotWalkForward        = (MMM_SubCmd)(mmmscWalkStop  + TROT_WALKING + WALK_FORWARD);
const MMM_SubCmd  mmmscTrotWalkRightForward   = (MMM_SubCmd)(mmmscWalkStop  + TROT_WALKING + WALK_RIGHTFORWARD);
const MMM_SubCmd  mmmscTrotWalkLeftForward    = (MMM_SubCmd)(mmmscWalkStop  + TROT_WALKING + WALK_LEFTFORWARD);
const MMM_SubCmd  mmmscTrotWalkRight          = (MMM_SubCmd)(mmmscWalkStop  + TROT_WALKING + WALK_RIGHT);
const MMM_SubCmd  mmmscTrotWalkLeft           = (MMM_SubCmd)(mmmscWalkStop  + TROT_WALKING + WALK_LEFT);
const MMM_SubCmd  mmmscTrotWalkBackward       = (MMM_SubCmd)(mmmscWalkStop  + TROT_WALKING + WALK_BACKWARD);
//    Adaptive Walking Command 
const MMM_SubCmd  mmmscAdaptiveWalkForward        = (MMM_SubCmd)(mmmscWalkStop + ADAPTIVE_WALKING + WALK_FORWARD);
const MMM_SubCmd  mmmscAdaptiveWalkRightForward   = (MMM_SubCmd)(mmmscWalkStop + ADAPTIVE_WALKING + WALK_RIGHTFORWARD);
const MMM_SubCmd  mmmscAdaptiveWalkLeftForward    = (MMM_SubCmd)(mmmscWalkStop + ADAPTIVE_WALKING + WALK_LEFTFORWARD);
const MMM_SubCmd  mmmscAdaptiveWalkRight          = (MMM_SubCmd)(mmmscWalkStop + ADAPTIVE_WALKING + WALK_RIGHT);
const MMM_SubCmd  mmmscAdaptiveWalkLeft           = (MMM_SubCmd)(mmmscWalkStop + ADAPTIVE_WALKING + WALK_LEFT);
const MMM_SubCmd  mmmscAdaptiveWalkBackward       = (MMM_SubCmd)(mmmscWalkStop + ADAPTIVE_WALKING + WALK_BACKWARD);
//  << MMM Sub Command for Tracking >>
const MMM_SubCmd  mmmscHeadHome                = (MMM_SubCmd)0x00020000 + (MMM_SubCmd)MOVE_TO_HOME;
const MMM_SubCmd  mmmscHeadCTracking           = (MMM_SubCmd)0x00020000 + (MMM_SubCmd)COLOR_TRACKING;
const MMM_SubCmd  mmmscHeadMTracking           = (MMM_SubCmd)0x00020000 + (MMM_SubCmd)MOTION_TRACKING;
const MMM_SubCmd  mmmscHeadMove                = (MMM_SubCmd)0x00020000 + (MMM_SubCmd)MOVE_TO_POINT;
const MMM_SubCmd  mmmscHeadMoveDiff            = (MMM_SubCmd)0x00020000 + (MMM_SubCmd)MOVE_WITH_DIFF;
//  << MMM Sub Command for Tail >>
const MMM_SubCmd  mmmscTailHome                = (MMM_SubCmd)0x00030000 + (MMM_SubCmd)TAIL_HOME;
const MMM_SubCmd  mmmscTailMove                = (MMM_SubCmd)0x00030000 + (MMM_SubCmd)TAIL_MOVE;
const MMM_SubCmd  mmmscTailSwing               = (MMM_SubCmd)0x00030000 + (MMM_SubCmd)TAIL_SWING;

#endif  //  _MMM_CommandList_h_DEFINED
