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
#include "NeutralAgent7.h"

static double BROADBASE_ANGLE[] = {
    0,    // TILT1
    0,    // PAN
    40,   // TILT2

    120,  // RFLEG J1
    90,   // RFLEG J2
    30,   // RFLEG J3

    120,  // LFLEG J1
    90,   // LFLEG J2
    30,   // LFLEG J3
    
    -120, // RRLEG J1
    70,   // RRLEG J2
    30,   // RRLEG J3

    -120, // LRLEG J1
    70,   // LRLEG J2
    30,   // LRLEG J3

    0,    // TAIL TILT
    0,    // TAIL PAN
    0     // MOUTH
};

static double SLEEPING_ANGLE[] = {
    0,    // TILT1
    0,    // PAN
    0,    // TILT2

    59,   // RFLEG J1
    0,    // RFLEG J2
    30,   // RFLEG J3

    59,   // LFLEG J1
    0,    // LFLEG J2
    30,   // LFLEG J3

    -119, // RRLEG J1
    4,    // RRLEG J2
    122,  // RRLEG J3

    -119, // LRLEG J1
    4,    // LRLEG J2
    122,  // LRLEG J3

    0,    // TAIL TILT
    0,    // TAIL PAN
    0     // MOUTH
};

NeutralAgent7::NeutralAgent7() : neutralAgentState(NAS7_IDLE)
{
}

void
NeutralAgent7::Init(ODA* oda)
{
}

bool
NeutralAgent7::AreYou(MoNetAgentID agent)
{
    OSYSDEBUG(("NeutralAgent7::AreYou(%d)\n", agent));
    return (agent == monetagentNEUTRAL) ? true : false;
}

void
NeutralAgent7::NotifyCommand(const MoNetAgentCommand& command,
                            MoNetAgentResult* result)
{
    OSYSDEBUG(("NeutralAgent7::NotifyCommand()\n"));
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
    neutralAgentState = NAS7_ADJUSTING_DIFF_JOINT_VALUE;

    result->agent      = monetagentNEUTRAL;
    result->index      = monetagentNEUTRAL_NT2SLEEP;
    result->status     = monetSUCCESS;
    result->endPosture = monetpostureUNDEF;
}

void
NeutralAgent7::ReadyEffector(const MoNetAgentCommand& command,
                            MoNetAgentResult* result)
{
    result->agent      = monetagentNEUTRAL;
    result->index      = monetagentNEUTRAL_NT2SLEEP;
    result->endPosture = monetpostureUNDEF;
    
    if (neutralAgentState == NAS7_IDLE) {

        ; // do nothing

    } else if (neutralAgentState == NAS7_ADJUSTING_DIFF_JOINT_VALUE) {

        OSYSDEBUG(("NAS7_ADJUSTING_DIFF_JOINT_VALUE\n"));
        SetJointGain();
        MoNetStatus s = MoveToBroadBase();
        neutralAgentState = NAS7_MOVING_TO_BROADBASE;
        result->status = monetSUCCESS;

    } else if (neutralAgentState == NAS7_MOVING_TO_BROADBASE) {

        OSYSDEBUG(("NAS7_MOVING_TO_BROADBASE\n"));
        MoNetStatus s = MoveToBroadBase();
        if (s == monetCOMPLETION) {
            neutralAgentState = NAS7_MOVING_TO_SLEEPING;
        }
        result->status = monetSUCCESS;

    } else if (neutralAgentState == NAS7_MOVING_TO_SLEEPING) {

        OSYSDEBUG(("NAS7_MOVING_TO_SLEEPING\n"));
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
NeutralAgent7::AdjustDiffJointValue(OVRSyncKey syncKey)
{
    OJointValue current[DRX1000_NUM_JOINT2];
    RCRegion* rgn = moNetAgentManager->FindFreeCommandRegion();
    OCommandVectorData* cmdVecData = (OCommandVectorData*)rgn->Base();
    cmdVecData->vectorInfo.syncKey = syncKey;

    for (int i = 0; i < DRX1000_NUM_JOINT2; i++) {
        OJointValue current;
        OPENR::GetJointValue(moNetAgentManager->PrimitiveID(i), &current);
        SetJointValue(rgn, i,
                      degrees(current.value/1000000.0),
                      degrees(current.value/1000000.0));
    }

    cmdVecData->SetNumData(DRX1000_NUM_JOINT2);

    moNetAgentManager->Effector()->SetData(rgn);
    moNetAgentManager->Effector()->NotifyObservers();

    return monetCOMPLETION;
}

MoNetStatus
NeutralAgent7::MoveToBroadBase()
{
    static int counter = -1;
    static double start[DRX1000_NUM_JOINT2];
    static double delta[DRX1000_NUM_JOINT2];
    double ndiv = (double)BROADBASE_MAX_COUNTER;

    if (counter == -1) {

        for (int i = 0; i < DRX1000_NUM_JOINT2; i++) {
            OJointValue current;
            OPrimitiveID jointID = moNetAgentManager->PrimitiveID(i);
            OPENR::GetJointValue(jointID, &current);
            start[i] = degrees(current.value/1000000.0);
            delta[i] = (BROADBASE_ANGLE[i] - start[i]) / ndiv;
        }

        counter = 0;

        RCRegion* rgn = moNetAgentManager->FindFreeCommandRegion();
        for (int i = 0; i < DRX1000_NUM_JOINT2; i++) {
            SetJointValue(rgn, i, start[i], start[i] + delta[i]);
            start[i] += delta[i];
        }

        OCommandVectorData* cmdVecData = (OCommandVectorData*)rgn->Base();
        cmdVecData->SetNumData(DRX1000_NUM_JOINT2);

        moNetAgentManager->Effector()->SetData(rgn);
        counter ++;
    }

    RCRegion* rgn = moNetAgentManager->FindFreeCommandRegion();
    for (int i = 0; i < DRX1000_NUM_JOINT2; i++) {
        SetJointValue(rgn, i, start[i], start[i] + delta[i]);
        start[i] += delta[i];
    }

    OCommandVectorData* cmdVecData = (OCommandVectorData*)rgn->Base();
    cmdVecData->SetNumData(DRX1000_NUM_JOINT2);

    moNetAgentManager->Effector()->SetData(rgn);
    moNetAgentManager->Effector()->NotifyObservers();

    counter++;
    return (counter == BROADBASE_MAX_COUNTER) ? monetCOMPLETION : monetSUCCESS;
}

MoNetStatus
NeutralAgent7::MoveToSleeping()
{
    static int counter = -1;
    static double start[DRX1000_NUM_JOINT2];
    static double delta[DRX1000_NUM_JOINT2];
    double ndiv = (double)SLEEPING_MAX_COUNTER;

    if (counter == -1) {

        for (int i = 0; i < DRX1000_NUM_JOINT2; i++) {
            start[i] = BROADBASE_ANGLE[i];
            delta[i] = (SLEEPING_ANGLE[i] - start[i]) / ndiv;
        }

        counter = 0;
    }

    RCRegion* rgn = moNetAgentManager->FindFreeCommandRegion();
    for (int i = 0; i < DRX1000_NUM_JOINT2; i++) {
        SetJointValue(rgn, i, start[i], start[i] + delta[i]);
        start[i] += delta[i];
    }

    OCommandVectorData* cmdVecData = (OCommandVectorData*)rgn->Base();
    cmdVecData->SetNumData(DRX1000_NUM_JOINT2);

    moNetAgentManager->Effector()->SetData(rgn);
    moNetAgentManager->Effector()->NotifyObservers();

    counter++;
    return (counter == SLEEPING_MAX_COUNTER) ? monetCOMPLETION : monetSUCCESS;
}

void
NeutralAgent7::SetJointGain()
{
    OSYSDEBUG(("NeutralAgent7::SetJointGain()\n"));

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

void
NeutralAgent7::SetJointValue(RCRegion* rgn, int idx, double start, double end)
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
