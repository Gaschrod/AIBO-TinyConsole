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

#include <OPENR/OPENRAPI.h>
#include "MTNAgent.h"

static const int DRX900_INDEX[] = {
    HEAD_TILT,
    HEAD_PAN,
    HEAD_ROLL,
    LFLEG_J1,
    LFLEG_J2,
    LFLEG_J3,
    LRLEG_J1,
    LRLEG_J2,
    LRLEG_J3,
    RFLEG_J1,
    RFLEG_J2,
    RFLEG_J3,
    RRLEG_J1,
    RRLEG_J2,
    RRLEG_J3
};    

MTNAgent::MTNAgent() : motionODA(0), mtn()
{
}

void
MTNAgent::Init(ODA* oda)
{
    motionODA = oda;
}

bool
MTNAgent::AreYou(MoNetAgentID agent)
{
    OSYSDEBUG(("MTNAgent::AreYou(%d)\n", agent));
    return (agent == monetagentMTN) ? true : false;
}

void
MTNAgent::NotifyCommand(const MoNetAgentCommand& command,
                        MoNetAgentResult* result)
{
    OSYSDEBUG(("MTNAgent::NotifyCommand()\n"));
    OSYSDEBUG(("  agent        : %d\n",   command.agent));
    OSYSDEBUG(("  index        : %d\n",   command.index));
    OSYSDEBUG(("  syncKey      : %08x\n", command.syncKey));
    OSYSDEBUG(("  startPosture : %d\n",   command.startPosture));
    OSYSDEBUG(("  endPosture   : %d\n",   command.endPosture));

    if (command.index == monetagentMTN_NULL_MOTION) {
        result->agent      = command.agent;
        result->index      = command.index;
        result->status     = monetCOMPLETION;
        result->endPosture = command.endPosture;
        return;
    }

    MTNFile* mtnfile = (MTNFile*)motionODA->GetData(command.index);
    if (mtnfile == 0) {
        result->agent      = command.agent;
        result->index      = command.index;
        result->status     = monetINVALID_ARG;
        result->endPosture = command.startPosture;
        return;
    }
    
    mtn.Set(mtnfile);
    MoNetStatus status = Move(command.syncKey);

    result->agent      = command.agent;
    result->index      = command.index;
    result->status     = status;
    result->endPosture = monetpostureUNDEF;
}

void
MTNAgent::ReadyEffector(const MoNetAgentCommand& command,
                        MoNetAgentResult* result)
{
    result->agent      = command.agent;
    result->index      = command.index;
    result->endPosture = monetpostureUNDEF;

    MoNetStatus status = Move(ovrsynckeyUNDEF);
    if (status == monetCOMPLETION) {
        result->endPosture = command.endPosture;
    }

    result->status = status;
}

MoNetStatus
MTNAgent::Move(OVRSyncKey syncKey)
{
    RCRegion* rgn = moNetAgentManager->FindFreeCommandRegion();
    OCommandVectorData* cmdVecData = (OCommandVectorData*)rgn->Base();
    cmdVecData->vectorInfo.syncKey = syncKey;

    MoNetStatus st = SetPrimitiveID(cmdVecData, mtn.GetRobotDesign());
    if (st != monetSUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s %s",
                  "MTNAgent::Move()",
                  "SetPrimitiveID() FAILED", mtn.GetName()));
        return monetINVALID_ARG;
    }

    int nframes = mtn.InterpolateCommandVectorData(cmdVecData, NUM_FRAMES);
    if (nframes == -1) {
        OSYSLOG1((osyslogERROR, "%s : %s %s",
                  "MTNAgent::Move()",
                  "mtn.InterpolateCommandVectorData() FAILED", mtn.GetName()));
        return monetINVALID_ARG;
    }

    mtn.Next(nframes);

    moNetAgentManager->Effector()->SetData(rgn);
    moNetAgentManager->Effector()->NotifyObservers();

    return (mtn.More() == true) ? monetSUCCESS : monetCOMPLETION;
}

MoNetStatus
MTNAgent::SetPrimitiveID(OCommandVectorData* cmdVec, char* robotDesign)
{
    if (strcmp(robotDesign, "DRX-900") == 0) {

        if (cmdVec->vectorInfo.maxNumData < DRX900_NUM_JOINTS) {
            return monetINVALID_ARG;
        }

        for (int i = 0; i < DRX900_NUM_JOINTS; i++) {
            OCommandInfo* info = cmdVec->GetInfo(i);
            OPrimitiveID id = moNetAgentManager->PrimitiveID(DRX900_INDEX[i]);
            info->Set(odataJOINT_COMMAND2, id, 0);
        }
        
        return monetSUCCESS;

    } else {

        return monetINVALID_ARG;

    }
}

void
MTNAgent::SetJointGain()
{
    OPrimitiveID tiltID = moNetAgentManager->PrimitiveID(HEAD_TILT);
    OPrimitiveID panID  = moNetAgentManager->PrimitiveID(HEAD_PAN);
    OPrimitiveID rollID = moNetAgentManager->PrimitiveID(HEAD_ROLL);

    OPENR::EnableJointGain(tiltID);
    OPENR::SetJointGain(tiltID,
                        TILT_PGAIN, TILT_IGAIN, TILT_DGAIN,
                        PSHIFT, ISHIFT, DSHIFT);
    
    OPENR::EnableJointGain(panID);
    OPENR::SetJointGain(panID,
                        PAN_PGAIN, PAN_IGAIN, PAN_DGAIN,
                        PSHIFT, ISHIFT, DSHIFT);

    OPENR::EnableJointGain(rollID);
    OPENR::SetJointGain(rollID,
                        ROLL_PGAIN, ROLL_IGAIN, ROLL_DGAIN,
                        PSHIFT, ISHIFT, DSHIFT);

    int base = RFLEG_J1;
    for (int i = 0; i < 4; i++) {

        OPrimitiveID j1ID = moNetAgentManager->PrimitiveID(base + 3 * i);
        OPrimitiveID j2ID = moNetAgentManager->PrimitiveID(base + 3 * i + 1);
        OPrimitiveID j3ID = moNetAgentManager->PrimitiveID(base + 3 * i + 2);
        
        OPENR::EnableJointGain(j1ID);        
        OPENR::SetJointGain(j1ID,
                            J1_PGAIN, J1_IGAIN, J1_DGAIN,
                            PSHIFT, ISHIFT, DSHIFT);

        OPENR::EnableJointGain(j2ID);
        OPENR::SetJointGain(j2ID,
                            J2_PGAIN, J2_IGAIN, J2_DGAIN,
                            PSHIFT, ISHIFT, DSHIFT);

        OPENR::EnableJointGain(j3ID);
        OPENR::SetJointGain(j3ID,
                            J3_PGAIN, J3_IGAIN, J3_DGAIN,
                            PSHIFT, ISHIFT, DSHIFT);
    }
}
