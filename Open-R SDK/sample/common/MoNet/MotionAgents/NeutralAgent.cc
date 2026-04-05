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
#include "NeutralAgent.h"

static double BROADBASE_ANGLE[] = {
    45,   // TILT
    0,    // PAN
    0,    // ROLL

    117,  // RFLEG J1
    90,   // RFLEG J2
    30,   // RFLEG J3

    117,  // LFLEG J1
    90,   // LFLEG J2
    30,   // LFLEG J3
    
    -117, // RRLEG J1
    70,   // RRLEG J2
    30,   // RRLEG J3

    -117, // LRLEG J1
    70,   // LRLEG J2
    30    // LRLEG J3
};

static double SLEEPING_ANGLE[] = {
    -10,  // TILT
    0,    // PAN
    0,    // ROLL

    60,   // RFLEG J1
    0,    // RFLEG J2
    30,   // RFLEG J3

    60,   // LFLEG J1
    0,    // LFLEG J2
    30,   // LFLEG J3

    -117, // RRLEG J1
    0,    // RRLEG J2
    147,  // RRLEG J3

    -117, // LRLEG J1
    0,    // LRLEG J2
    147   // LRLEG J3
};

NeutralAgent::NeutralAgent() : neutralAgentState(NAS_IDLE)
{
}

void
NeutralAgent::Init(ODA* oda)
{
}

bool
NeutralAgent::AreYou(MoNetAgentID agent)
{
    OSYSDEBUG(("NeutralAgent::AreYou(%d)\n", agent));
    return (agent == monetagentNEUTRAL) ? true : false;
}

void
NeutralAgent::NotifyCommand(const MoNetAgentCommand& command,
                            MoNetAgentResult* result)
{
    OSYSDEBUG(("NeutralAgent::NotifyCommand()\n"));
    OSYSDEBUG(("  agent        : %d\n",   command.agent));
    OSYSDEBUG(("  index        : %d\n",   command.index));
    OSYSDEBUG(("  syncKey      : %08x\n", command.syncKey));
    OSYSDEBUG(("  startPosture : %d\n",   command.startPosture));
    OSYSDEBUG(("  endPosture   : %d\n",   command.endPosture));

    if (command.agent != monetagentNEUTRAL ||
        command.index != monetagentNEUTRAL_NT2SLEEP) {

        result->agent      = command.agent;
        result->index      = command.index;
        result->status     = monetINVALID_ARG;
        result->endPosture = monetpostureUNDEF;
        return;
    }

    AdjustDiffJointValue(command.syncKey);
    neutralAgentState = NAS_ADJUSTING_DIFF_JOINT_VALUE;

    result->agent      = monetagentNEUTRAL;
    result->index      = monetagentNEUTRAL_NT2SLEEP;
    result->status     = monetSUCCESS;
    result->endPosture = monetpostureUNDEF;
}

void
NeutralAgent::ReadyEffector(const MoNetAgentCommand& command,
                            MoNetAgentResult* result)
{
    result->agent      = monetagentNEUTRAL;
    result->index      = monetagentNEUTRAL_NT2SLEEP;
    result->endPosture = monetpostureUNDEF;
    
    if (neutralAgentState == NAS_IDLE) {

        ; // do nothing

    } else if (neutralAgentState == NAS_ADJUSTING_DIFF_JOINT_VALUE) {

        OSYSDEBUG(("NAS_ADJUSTING_DIFF_JOINT_VALUE\n"));
        SetJointGain();
        MoNetStatus s = MoveToBroadBase();
        neutralAgentState = NAS_MOVING_TO_BROADBASE;
        result->status = monetSUCCESS;

    } else if (neutralAgentState == NAS_MOVING_TO_BROADBASE) {

        OSYSDEBUG(("NAS_MOVING_TO_BROADBASE\n"));
        MoNetStatus s = MoveToBroadBase();
        if (s == monetCOMPLETION) {
            neutralAgentState = NAS_MOVING_TO_SLEEPING;
        }
        result->status = monetSUCCESS;

    } else if (neutralAgentState == NAS_MOVING_TO_SLEEPING) {

        OSYSDEBUG(("NAS_MOVING_TO_SLEEPING\n"));
        MoNetStatus s = MoveToSleeping();
        if (s == monetCOMPLETION) {
            result->status     = monetCOMPLETION;
            result->endPosture = monetpostureSLEEP;
        } else {
            result->status = monetSUCCESS;
        }
    }
}

MoNetStatus
NeutralAgent::AdjustDiffJointValue(OVRSyncKey syncKey)
{
    OJointValue current[DRX900_NUM_JOINTS];
    RCRegion* rgn = moNetAgentManager->FindFreeCommandRegion();
    OCommandVectorData* cmdVecData = (OCommandVectorData*)rgn->Base();
    cmdVecData->vectorInfo.syncKey = syncKey;

    for (int i = 0; i < DRX900_NUM_JOINTS; i++) {
        OJointValue current;
        OPENR::GetJointValue(moNetAgentManager->PrimitiveID(i), &current);
        SetJointValue(rgn, i,
                      degrees(current.value/1000000.0),
                      degrees(current.value/1000000.0));
    }

    moNetAgentManager->Effector()->SetData(rgn);
    moNetAgentManager->Effector()->NotifyObservers();

    return monetCOMPLETION;
}

MoNetStatus
NeutralAgent::MoveToBroadBase()
{
    static int counter = -1;
    static double start[DRX900_NUM_JOINTS];
    static double delta[DRX900_NUM_JOINTS];
    double ndiv = (double)BROADBASE_MAX_COUNTER;

    if (counter == -1) {

        for (int i = 0; i < DRX900_NUM_JOINTS; i++) {
            OJointValue current;
            OPrimitiveID jointID = moNetAgentManager->PrimitiveID(i);
            OPENR::GetJointValue(jointID, &current);
            start[i] = degrees(current.value/1000000.0);
            delta[i] = (BROADBASE_ANGLE[i] - start[i]) / ndiv;
        }

        counter = 0;

        RCRegion* rgn = moNetAgentManager->FindFreeCommandRegion();
        for (int i = 0; i < DRX900_NUM_JOINTS; i++) {
            SetJointValue(rgn, i, start[i], start[i] + delta[i]);
            start[i] += delta[i];
        }

        moNetAgentManager->Effector()->SetData(rgn);
        counter ++;
    }

    RCRegion* rgn = moNetAgentManager->FindFreeCommandRegion();
    for (int i = 0; i < DRX900_NUM_JOINTS; i++) {
        SetJointValue(rgn, i, start[i], start[i] + delta[i]);
        start[i] += delta[i];
    }

    moNetAgentManager->Effector()->SetData(rgn);
    moNetAgentManager->Effector()->NotifyObservers();

    counter++;
    return (counter == BROADBASE_MAX_COUNTER) ? monetCOMPLETION : monetSUCCESS;
}

MoNetStatus
NeutralAgent::MoveToSleeping()
{
    static int counter = -1;
    static double start[DRX900_NUM_JOINTS];
    static double delta[DRX900_NUM_JOINTS];
    double ndiv = (double)SLEEPING_MAX_COUNTER;

    if (counter == -1) {

        for (int i = 0; i < DRX900_NUM_JOINTS; i++) {
            start[i] = BROADBASE_ANGLE[i];
            delta[i] = (SLEEPING_ANGLE[i] - start[i]) / ndiv;
        }

        counter = 0;
    }

    RCRegion* rgn = moNetAgentManager->FindFreeCommandRegion();
    for (int i = 0; i < DRX900_NUM_JOINTS; i++) {
        SetJointValue(rgn, i, start[i], start[i] + delta[i]);
        start[i] += delta[i];
    }

    moNetAgentManager->Effector()->SetData(rgn);
    moNetAgentManager->Effector()->NotifyObservers();

    counter++;
    return (counter == SLEEPING_MAX_COUNTER) ? monetCOMPLETION : monetSUCCESS;
}

void
NeutralAgent::SetJointGain()
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

void
NeutralAgent::SetJointValue(RCRegion* rgn, int idx, double start, double end)
{
    OCommandVectorData* cmdVecData = (OCommandVectorData*)rgn->Base();

    OPrimitiveID jointID = moNetAgentManager->PrimitiveID(idx);
    OCommandInfo* info = cmdVecData->GetInfo(idx);
    info->Set(odataJOINT_COMMAND2, jointID, NUM_FRAMES);

    OCommandData* data = cmdVecData->GetData(idx);
    OJointCommandValue2* jval = (OJointCommandValue2*)data->value;

    double delta = end - start;
    for (int i = 0; i < NUM_FRAMES; i++) {
        double dval = start + (delta * i) / (double)NUM_FRAMES;
        jval[i].value = oradians(dval);
    }
}
