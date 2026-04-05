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
#include "BlinkingHeadLED.h"

BlinkingHeadLED::BlinkingHeadLED() : blinkingLEDState(BHLS_IDLE)
{
    for (int i = 0; i < NUM_LEDS; i++) ledID[i] = oprimitiveID_UNDEF;
    for (int i = 0; i < NUM_COMMAND_VECTOR; i++) region[i] = 0;
}

OStatus
BlinkingHeadLED::DoInit(const OSystemEvent& event)
{
    OSYSDEBUG(("BlinkingHeadLED::DoInit()\n"));

    NEW_ALL_SUBJECT_AND_OBSERVER;
    REGISTER_ALL_ENTRY;
    SET_ALL_READY_AND_NOTIFY_ENTRY;

    OpenPrimitives();
    NewCommandVectorData();
    OStatus st = OPENR::SetMotorPower(opowerON);
    OSYSDEBUG(("OPENR::SetMotorPower(opowerON) %d\n", st));
    
    return oSUCCESS;
}

OStatus
BlinkingHeadLED::DoStart(const OSystemEvent& event)
{
    OSYSDEBUG(("BlinkingHeadLED::DoStart()\n"));

    BlinkLED();
    blinkingLEDState = BHLS_START;

    ENABLE_ALL_SUBJECT;
    ASSERT_READY_TO_ALL_OBSERVER;

    return oSUCCESS;
}

OStatus
BlinkingHeadLED::DoStop(const OSystemEvent& event)
{
    OSYSDEBUG(("BlinkingHeadLED::DoStop()\n"));

    blinkingLEDState = BHLS_IDLE;

    DISABLE_ALL_SUBJECT;    
    DEASSERT_READY_TO_ALL_OBSERVER;

    return oSUCCESS;
}

OStatus
BlinkingHeadLED::DoDestroy(const OSystemEvent& event)
{
    DELETE_ALL_SUBJECT_AND_OBSERVER;
    return oSUCCESS;
}

void
BlinkingHeadLED::Ready(const OReadyEvent& event)
{
    OSYSDEBUG(("BlinkingHeadLED::Ready()\n"));

    if (blinkingLEDState == BHLS_START) {
        OSYSDEBUG(("BHLS_START\n"));
        BlinkLED();
    }
}

void
BlinkingHeadLED::OpenPrimitives()
{
    for (int i = 0; i < NUM_LEDS; i++) {
        OStatus result = OPENR::OpenPrimitive(LED2_LOCATOR[i], &ledID[i]);
        if (result != oSUCCESS) {
            OSYSLOG1((osyslogERROR, "%s : %s %d",
                      "BlinkingHeadLED::OpenPrimitives()",
                      "OPENR::OpenPrimitive() FAILED", result));
        }
    }
}

void
BlinkingHeadLED::NewCommandVectorData()
{
    OStatus result;
    MemoryRegionID      cmdVecDataID;
    OCommandVectorData* cmdVecData;

    for (int i = 0; i < NUM_COMMAND_VECTOR; i++) {

        result = OPENR::NewCommandVectorData(NUM_LEDS, 
                                             &cmdVecDataID, &cmdVecData);
        if (result != oSUCCESS) {
            OSYSLOG1((osyslogERROR, "%s : %s %d",
                      "BlinkingHeadLED::NewCommandVectorData()",
                      "OPENR::NewCommandVectorData() FAILED", result));
        }

        region[i] = new RCRegion(cmdVecData->vectorInfo.memRegionID,
                                 cmdVecData->vectorInfo.offset,
                                 (void*)cmdVecData,
                                 cmdVecData->vectorInfo.totalSize);

        cmdVecData->SetNumData(NUM_LEDS);


        for (int j = 0; j < NUM_LEDS; j++) {
            OCommandInfo* info = cmdVecData->GetInfo(j);
            info->Set(odataLED_COMMAND2, ledID[j], NUM_FRAMES);

            OCommandData* data = cmdVecData->GetData(j);
            OLEDCommandValue2* val = (OLEDCommandValue2*)data->value;
            for (int k = 0; k < NUM_FRAMES; k++) {
                val[k].led    = LED2_VALUE[j][k];
                val[k].period = 64; // 8ms * 64 = 512ms
            }
        }
    }
}

void
BlinkingHeadLED::BlinkLED()
{
    static int index = -1;

    if (index == -1) { // BlinkLED() is called first time.
        index = 0;
        subject[sbjBlink]->SetData(region[index]);
        index++;
    } 

    subject[sbjBlink]->SetData(region[index]);
    subject[sbjBlink]->NotifyObservers();

    index++;
    index = index % NUM_COMMAND_VECTOR;
    OSYSDEBUG(("index %d\n", index));
}
