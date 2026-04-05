//
// Copyright 2002,2004 Sony Corporation 
//
// Permission to use, copy, modify, and redistribute this software for
// non-commercial use is hereby granted.
//
// This software is provided "as is" without warranty of any kind,
// either expressed or implied, including but not limited to the
// implied warranties of fitness for a particular purpose.
//

#ifndef MoNetData_h
#define MoNetData_h

#include <OPENR/OPENR.h>

static const char* const MONET_MOTION_KEYWORD = "MONET_MOTION";
static const char* const MONET_SOUND_KEYWORD  = "MONET_SOUND";
static const char* const MONET_CONFIG     = "/MS/OPEN-R/MW/CONF/ERS-200/MONET.CFG";
static const char* const MONETCMD_CONFIG  = "/MS/OPEN-R/MW/CONF/ERS-200/MONETCMD.CFG";
static const char* const MONET7_CONFIG    = "/MS/OPEN-R/MW/CONF/ERS-7/MONET.CFG";
static const char* const MONETCMD7_CONFIG = "/MS/OPEN-R/MW/CONF/ERS-7/MONETCMD.CFG";
const size_t MONET_AGENT_COMMAND_NAME_MAX = 31;
const size_t MONET_AGENT_NAME_MAX         = 31;

typedef int MoNetCommandID;
const MoNetCommandID monetcommandID_UNDEF = -1;

typedef int MoNetPosture;
const MoNetPosture monetpostureUNDEF    = -1;
const MoNetPosture monetpostureANY      =  0; // "any"
const MoNetPosture monetpostureNT       =  1; // "nt"
const MoNetPosture monetpostureSTAND    =  2; // "stand"
const MoNetPosture monetpostureSIT      =  3; // "sit"
const MoNetPosture monetpostureSLEEP    =  4; // "sleep"
const MoNetPosture monetpostureWALK     =  5; // "walk"

typedef int MoNetAgentID;
const MoNetAgentID monetagentUNDEF   = -1;
const MoNetAgentID monetagentNEUTRAL =  0;
const MoNetAgentID monetagentMTN     =  1;
const MoNetAgentID monetagentSOUND   =  2;
const MoNetAgentID monetagentMTNWALK =  3;

static const char* const monetagentNEUTRAL_STRNAME = "monetagentNEUTRAL";
static const char* const monetagentMTN_STRNAME     = "monetagentMTN";
static const char* const monetagentSOUND_STRNAME   = "monetagentSOUND";
static const char* const monetagentMTNWALK_STRNAME = "monetagentMTNWALK";

//
// monetagentNEUTRAL command index
//
const int monetagentNEUTRAL_NT2SLEEP = 0;

//
// monetagentMTN command index
//
const int monetagentMTN_NULL_MOTION = -1;

typedef int MoNetStatus;
const MoNetStatus monetUNDEF        = -1;
const MoNetStatus monetSUCCESS      =  0; // also means CONTINUATION
const MoNetStatus monetCOMPLETION   =  1;
const MoNetStatus monetINCOMPLETION =  2;
const MoNetStatus monetBUSY         =  3;
const MoNetStatus monetINVALID_ARG  =  4;
const MoNetStatus monetINVALID_WAV  =  5;

struct MoNetCommand {
    MoNetCommandID  commandID;

    MoNetCommand(MoNetCommandID id) { commandID = id; }
};

struct MoNetResult {
    MoNetCommandID  commandID;
    MoNetStatus     status;
    MoNetPosture    posture;

    MoNetResult(MoNetCommandID id, MoNetStatus st, MoNetPosture pos) {
        commandID = id;
        status    = st;
        posture   = pos;
    }
};

struct MoNetAgentCommand {
    MoNetAgentID  agent;
    int           index;
    OVRSyncKey    syncKey;
    MoNetPosture  startPosture;
    MoNetPosture  endPosture;

    MoNetAgentCommand() { Clear(); }
    void Clear() {
        agent        = monetagentUNDEF;
        index        = -1;
        syncKey      = ovrsynckeyUNDEF;
        startPosture = monetpostureUNDEF;
        endPosture   = monetpostureUNDEF;
    }
};

struct MoNetAgentResult {
    MoNetAgentID  agent;
    int           index;
    MoNetStatus   status;
    MoNetPosture  endPosture;

    MoNetAgentResult() {
        agent      = monetagentUNDEF;
        index      = -1;
        status     = monetUNDEF;
        endPosture = monetpostureUNDEF;
    }
    MoNetAgentResult(MoNetAgentID a, int idx, MoNetStatus s, MoNetPosture p) {
        agent   = a;
        index   = idx;
        status  = s;
        endPosture = p;
    }
};

#endif // MoNetData_h
