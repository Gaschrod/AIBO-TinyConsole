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
#include "MTNAgent7.h"

static const int DRX1000_INDEX[] = {
    HEAD_TILT1,
    HEAD_PAN,
    HEAD_TILT2,
    MOUTH,
    LEFT_EAR,
    RIGHT_EAR,
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
    RRLEG_J3,
    TAIL_TILT,
    TAIL_PAN
};    

MTNAgent7::MTNAgent7() : motionODA(0), mtn()
{
}

void
MTNAgent7::Init(ODA* oda)
{
    motionODA = oda;
}

bool
MTNAgent7::AreYou(MoNetAgentID agent)
{
    OSYSDEBUG(("MTNAgent7::AreYou(%d)\n", agent));
    return (agent == monetagentMTN) ? true : false;
}

void
MTNAgent7::NotifyCommand(const MoNetAgentCommand& command,
                        MoNetAgentResult* result)
{
    OSYSDEBUG(("MTNAgent7::NotifyCommand()\n"));
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
MTNAgent7::ReadyEffector(const MoNetAgentCommand& command,
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
MTNAgent7::Move(OVRSyncKey syncKey)
{
    RCRegion* rgn = moNetAgentManager->FindFreeCommandRegion();
    OCommandVectorData* cmdVecData = (OCommandVectorData*)rgn->Base();
    cmdVecData->vectorInfo.syncKey = syncKey;

    MoNetStatus st = SetPrimitiveID(cmdVecData, mtn.GetRobotDesign());
    if (st != monetSUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s %s",
                  "MTNAgent7::Move()",
                  "SetPrimitiveID() FAILED", mtn.GetName()));
        return monetINVALID_ARG;
    }

    int nframes = mtn.InterpolateCommandVectorData(cmdVecData, NUM_FRAMES);
    if (nframes == -1) {
        OSYSLOG1((osyslogERROR, "%s : %s %s",
                  "MTNAgent7::Move()",
                  "mtn.InterpolateCommandVectorData() FAILED", mtn.GetName()));
        return monetINVALID_ARG;
    }

    mtn.Next(nframes);

    moNetAgentManager->Effector()->SetData(rgn);
    moNetAgentManager->Effector()->NotifyObservers();

    return (mtn.More() == true) ? monetSUCCESS : monetCOMPLETION;
}

MoNetStatus
MTNAgent7::SetPrimitiveID(OCommandVectorData* cmdVec, char* robotDesign)
{
    if (strcmp(robotDesign, "DRX-1000") == 0) {

        if (cmdVec->vectorInfo.maxNumData < DRX1000_NUM_JOINTS) {
            return monetINVALID_ARG;
        }

        for (int i = 0; i < DRX1000_NUM_JOINTS; i++) {
            OCommandInfo* info = cmdVec->GetInfo(i);
            OPrimitiveID id = moNetAgentManager->PrimitiveID(DRX1000_INDEX[i]);
            info->Set(odataJOINT_COMMAND2, id, 0);
        }

        cmdVec->SetNumData(DRX1000_NUM_JOINTS);

        return monetSUCCESS;

    } else {

        return monetINVALID_ARG;

    }
}

void
MTNAgent7::SetJointGain()
{
    OSYSDEBUG(("MTNAgent7::SetJointGain()\n"));

    OPrimitiveID tilt1ID = moNetAgentManager->PrimitiveID(HEAD_TILT1);
    OPrimitiveID panID   = moNetAgentManager->PrimitiveID(HEAD_PAN);
    OPrimitiveID tilt2ID = moNetAgentManager->PrimitiveID(HEAD_TILT2);

    OPENR::EnableJointGain(tilt1ID);
    OPENR::SetJointGain(tilt1ID,
                        TILT1_PGAIN, TILT1_IGAIN, TILT1_DGAIN,
                        PSHIFT, ISHIFT, DSHIFT);
    
    OPENR::EnableJointGain(panID);
    OPENR::SetJointGain(panID,
                        PAN_PGAIN, PAN_IGAIN, PAN_DGAIN,
                        PSHIFT, ISHIFT, DSHIFT);

    OPENR::EnableJointGain(tilt2ID);
    OPENR::SetJointGain(tilt2ID,
                        TILT2_PGAIN, TILT2_IGAIN, TILT2_DGAIN,
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

    OPrimitiveID tailtiltID = moNetAgentManager->PrimitiveID(TAIL_TILT);
    OPrimitiveID tailpanID  = moNetAgentManager->PrimitiveID(TAIL_PAN);

    OPENR::EnableJointGain(tailtiltID);
    OPENR::SetJointGain(tailtiltID,
                        TAIL_PGAIN, TAIL_IGAIN, TAIL_DGAIN,
                        PSHIFT, ISHIFT, DSHIFT);

    OPENR::EnableJointGain(tailpanID);
    OPENR::SetJointGain(tailpanID,
                        TAIL_PGAIN, TAIL_IGAIN, TAIL_DGAIN,
                        PSHIFT, ISHIFT, DSHIFT);
}
