//
// Copyright 2003 Sony Corporation 
//
// Permission to use, copy, modify, and redistribute this software for
// non-commercial use is hereby granted.
//
// This software is provided "as is" without warranty of any kind,
// either expressed or implied, including but not limited to the
// implied warranties of fitness for a particular purpose.
//

#include <OPENR/OPENRAPI.h>
#include <OPENR/OSyslog.h>
#include <OPENR/core_macro.h>
#include "MovingEar7.h"

MovingEar7::MovingEar7() : movingEarState(MES_IDLE)
{
    for (int i = 0; i < NUM_EARS; i++) earID[i] = oprimitiveID_UNDEF;
    for (int i = 0; i < NUM_COMMAND_VECTOR; i++) region[i] = 0;
}

OStatus
MovingEar7::DoInit(const OSystemEvent& event)
{
    OSYSDEBUG(("MovingEar7::DoInit()\n"));

    NEW_ALL_SUBJECT_AND_OBSERVER;
    REGISTER_ALL_ENTRY;
    SET_ALL_READY_AND_NOTIFY_ENTRY;

    OpenPrimitives();
    NewCommandVectorData();

    // 
    // OPENR::SetMotorPower(opowerON) is executed in blinkingLED.
    // So, it isn't necessary here.
    //
    
    return oSUCCESS;
}

OStatus
MovingEar7::DoStart(const OSystemEvent& event)
{
    OSYSDEBUG(("MovingEar7::DoStart()\n"));

    ENABLE_ALL_SUBJECT;
    ASSERT_READY_TO_ALL_OBSERVER;

    MoveEar();
    movingEarState = MES_START;

    return oSUCCESS;
}

OStatus
MovingEar7::DoStop(const OSystemEvent& event)
{
    OSYSDEBUG(("MovingEar7::DoStop()\n"));

    movingEarState = MES_IDLE;

    DISABLE_ALL_SUBJECT;
    DEASSERT_READY_TO_ALL_OBSERVER;

    return oSUCCESS;
}

OStatus
MovingEar7::DoDestroy(const OSystemEvent& event)
{
    DELETE_ALL_SUBJECT_AND_OBSERVER;
    return oSUCCESS;
}

void
MovingEar7::Ready(const OReadyEvent& event)
{
    OSYSDEBUG(("MovingEar7::Ready()\n"));

    if (movingEarState == MES_START) {
        OSYSDEBUG(("MES_START\n"));
        MoveEar();
    }
}

void
MovingEar7::OpenPrimitives()
{
    for (int i = 0; i < NUM_EARS; i++) {
        OStatus result = OPENR::OpenPrimitive(EAR_LOCATOR[i], &earID[i]);
        if (result != oSUCCESS) {
            OSYSLOG1((osyslogERROR, "%s : %s %d",
                      "MovingEar7::OpenPrimitives()",
                      "OPENR::OpenPrimitive() FAILED", result));
        }
    }
}

void
MovingEar7::NewCommandVectorData()
{
    OStatus result;
    MemoryRegionID      cmdVecDataID;
    OCommandVectorData* cmdVecData;

    for (int i = 0; i < NUM_COMMAND_VECTOR; i++) {

        result = OPENR::NewCommandVectorData(NUM_EARS, 
                                             &cmdVecDataID, &cmdVecData);
        if (result != oSUCCESS) {
            OSYSLOG1((osyslogERROR, "%s : %s %d",
                      "MovingEar7::NewCommandVectorData()",
                      "OPENR::NewCommandVectorData() FAILED", result));
        }

        region[i] = new RCRegion(cmdVecData->vectorInfo.memRegionID,
                                 cmdVecData->vectorInfo.offset,
                                 (void*)cmdVecData,
                                 cmdVecData->vectorInfo.totalSize);

        cmdVecData->SetNumData(NUM_EARS);

        // 
        // Left ear  : earID[0] PRM:/r1/c1/c2/c3/e5-Joint4:15
        // Right ear : earID[1] PRM:/r1/c1/c2/c3/e6-Joint4:16
        //
        //                    Left ear        Right ear
        // -------------------------------------------------
        // region[0] val[0] : ojoint4_STATE1  ojoint4_STATE0
        //           val[1] : ojoint4_STATE0  ojoint4_STATE0
        //           val[2] : ojoint4_STATE0  ojoint4_STATE0
        //           val[3] : ojoint4_STATE0  ojoint4_STATE0
        //           val[4] : ojoint4_STATE0  ojoint4_STATE0
        //           val[5] : ojoint4_STATE0  ojoint4_STATE0
        //           val[6] : ojoint4_STATE0  ojoint4_STATE0
        //           val[7] : ojoint4_STATE0  ojoint4_STATE0
        // region[1] val[0] : ojoint4_STATE0  ojoint4_STATE1
        //           val[1] : ojoint4_STATE0  ojoint4_STATE0
        //           val[2] : ojoint4_STATE0  ojoint4_STATE0
        //           val[3] : ojoint4_STATE0  ojoint4_STATE0
        //           val[4] : ojoint4_STATE0  ojoint4_STATE0
        //           val[5] : ojoint4_STATE0  ojoint4_STATE0
        //           val[6] : ojoint4_STATE0  ojoint4_STATE0
        //           val[7] : ojoint4_STATE0  ojoint4_STATE0
        //
        {
            // Left ear
            OCommandInfo* info = cmdVecData->GetInfo(0);
            info->Set(odataJOINT_COMMAND4, earID[0], ocommandMAX_FRAMES);

            OCommandData* data = cmdVecData->GetData(0);
            OJointCommandValue4* val = (OJointCommandValue4*)data->value;
            for (int k = 0; k < ocommandMAX_FRAMES; k++) {
                if (i == 0 && k == 0) {
                    val[k].value = ojoint4_STATE1;
                } else {
                    val[k].value = ojoint4_STATE0;
                }
                val[k].period = 16;
            }
        }

        {
            // Right ear
            OCommandInfo* info = cmdVecData->GetInfo(1);
            info->Set(odataJOINT_COMMAND4, earID[1], ocommandMAX_FRAMES);

            OCommandData* data = cmdVecData->GetData(1);
            OJointCommandValue4* val = (OJointCommandValue4*)data->value;
            for (int k = 0; k < ocommandMAX_FRAMES; k++) {
                if (i == 1 && k == 0) {
                    val[k].value = ojoint4_STATE1;
                } else {
                    val[k].value = ojoint4_STATE0;
                }
                val[k].period = 16;
            }
        }
    }
}

void
MovingEar7::MoveEar()
{
    static int index = -1;

    if (index == -1) { // MoveEar() is called first time.
        index = 0;
        subject[sbjMove]->SetData(region[index]);
        index++;
    } 

    subject[sbjMove]->SetData(region[index]);
    subject[sbjMove]->NotifyObservers();

    index++;
    index = index % NUM_COMMAND_VECTOR;
}
