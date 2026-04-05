//
// Copyright 2002 Sony Corporation 
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
#include "MovingEar.h"

MovingEar::MovingEar() : movingEarState(MES_IDLE)
{
    for (int i = 0; i < NUM_EARS; i++) earID[i] = oprimitiveID_UNDEF;
    for (int i = 0; i < NUM_COMMAND_VECTOR; i++) region[i] = 0;
}

OStatus
MovingEar::DoInit(const OSystemEvent& event)
{
    OSYSDEBUG(("MovingEar::DoInit()\n"));

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
MovingEar::DoStart(const OSystemEvent& event)
{
    OSYSDEBUG(("MovingEar::DoStart()\n"));

    ENABLE_ALL_SUBJECT;
    ASSERT_READY_TO_ALL_OBSERVER;

    MoveEar();
    movingEarState = MES_START;

    return oSUCCESS;
}

OStatus
MovingEar::DoStop(const OSystemEvent& event)
{
    OSYSDEBUG(("MovingEar::DoStop()\n"));

    movingEarState = MES_IDLE;

    DISABLE_ALL_SUBJECT;
    DEASSERT_READY_TO_ALL_OBSERVER;

    return oSUCCESS;
}

OStatus
MovingEar::DoDestroy(const OSystemEvent& event)
{
    DELETE_ALL_SUBJECT_AND_OBSERVER;
    return oSUCCESS;
}

void
MovingEar::Ready(const OReadyEvent& event)
{
    OSYSDEBUG(("MovingEar::Ready()\n"));

    if (movingEarState == MES_START) {
        OSYSDEBUG(("MES_START\n"));
        MoveEar();
    }
}

void
MovingEar::OpenPrimitives()
{
    for (int i = 0; i < NUM_EARS; i++) {
        OStatus result = OPENR::OpenPrimitive(EAR_LOCATOR[i], &earID[i]);
        if (result != oSUCCESS) {
            OSYSLOG1((osyslogERROR, "%s : %s %d",
                      "MovingEar::OpenPrimitives()",
                      "OPENR::OpenPrimitive() FAILED", result));
        }
    }
}

void
MovingEar::NewCommandVectorData()
{
    OStatus result;
    MemoryRegionID      cmdVecDataID;
    OCommandVectorData* cmdVecData;

    for (int i = 0; i < NUM_COMMAND_VECTOR; i++) {

        result = OPENR::NewCommandVectorData(NUM_EARS, 
                                             &cmdVecDataID, &cmdVecData);
        if (result != oSUCCESS) {
            OSYSLOG1((osyslogERROR, "%s : %s %d",
                      "MovingEar::NewCommandVectorData()",
                      "OPENR::NewCommandVectorData() FAILED", result));
        }

        region[i] = new RCRegion(cmdVecData->vectorInfo.memRegionID,
                                 cmdVecData->vectorInfo.offset,
                                 (void*)cmdVecData,
                                 cmdVecData->vectorInfo.totalSize);

        cmdVecData->SetNumData(NUM_EARS);

        for (int j = 0; j < NUM_EARS; j++) {
            OCommandInfo* info = cmdVecData->GetInfo(j);
            info->Set(odataJOINT_COMMAND3, earID[j], ocommandMAX_FRAMES);

            OCommandData* data = cmdVecData->GetData(j);
            OJointCommandValue3* val = (OJointCommandValue3*)data->value;
            for (int k = 0; k < ocommandMAX_FRAMES; k++) {
                if (i % 2 == 0) {
                    val[k].value = ojoint3_STATE0;
                } else {
                    val[k].value = ojoint3_STATE1;
                }
            }
        }
    }
}

void
MovingEar::MoveEar()
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
