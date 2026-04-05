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
#include "BlinkingLED.h"

BlinkingLED::BlinkingLED() : blinkingLEDState(BLS_IDLE)
{
    for (int i = 0; i < NUM_LEDS; i++) ledID[i] = oprimitiveID_UNDEF;
    for (int i = 0; i < NUM_COMMAND_VECTOR; i++) region[i] = 0;
}

OStatus
BlinkingLED::DoInit(const OSystemEvent& event)
{
    OSYSDEBUG(("BlinkingLED::DoInit()\n"));

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
BlinkingLED::DoStart(const OSystemEvent& event)
{
    OSYSDEBUG(("BlinkingLED::DoStart()\n"));

    BlinkLED();
    blinkingLEDState = BLS_START;

    ENABLE_ALL_SUBJECT;
    ASSERT_READY_TO_ALL_OBSERVER;

    return oSUCCESS;
}

OStatus
BlinkingLED::DoStop(const OSystemEvent& event)
{
    OSYSDEBUG(("BlinkingLED::DoStop()\n"));

    blinkingLEDState = BLS_IDLE;

    DISABLE_ALL_SUBJECT;    
    DEASSERT_READY_TO_ALL_OBSERVER;

    return oSUCCESS;
}

OStatus
BlinkingLED::DoDestroy(const OSystemEvent& event)
{
    DELETE_ALL_SUBJECT_AND_OBSERVER;
    return oSUCCESS;
}

void
BlinkingLED::Ready(const OReadyEvent& event)
{
    OSYSDEBUG(("BlinkingLED::Ready()\n"));

    if (blinkingLEDState == BLS_START) {
        OSYSDEBUG(("BLS_START\n"));
        BlinkLED();
    }
}

void
BlinkingLED::OpenPrimitives()
{
    for (int i = 0; i < NUM_LEDS; i++) {
        OStatus result = OPENR::OpenPrimitive(LED_LOCATOR[i], &ledID[i]);
        if (result != oSUCCESS) {
            OSYSLOG1((osyslogERROR, "%s : %s %d",
                      "BlinkingLED::OpenPrimitives()",
                      "OPENR::OpenPrimitive() FAILED", result));
        }
    }
}

void
BlinkingLED::NewCommandVectorData()
{
    OStatus result;
    MemoryRegionID      cmdVecDataID;
    OCommandVectorData* cmdVecData;

    for (int i = 0; i < NUM_COMMAND_VECTOR; i++) {

        result = OPENR::NewCommandVectorData(NUM_LEDS, 
                                             &cmdVecDataID, &cmdVecData);
        if (result != oSUCCESS) {
            OSYSLOG1((osyslogERROR, "%s : %s %d",
                      "BlinkingLED::NewCommandVectorData()",
                      "OPENR::NewCommandVectorData() FAILED", result));
        }

        region[i] = new RCRegion(cmdVecData->vectorInfo.memRegionID,
                                 cmdVecData->vectorInfo.offset,
                                 (void*)cmdVecData,
                                 cmdVecData->vectorInfo.totalSize);

        cmdVecData->SetNumData(NUM_LEDS);

        for (int j = 0; j < NUM_LEDS; j++) {
            OCommandInfo* info = cmdVecData->GetInfo(j);
            info->Set(odataLED_COMMAND2, ledID[j], 1);

            OCommandData* data = cmdVecData->GetData(j);
            OLEDCommandValue2* val = (OLEDCommandValue2*)data->value;
            if (i % 2 == 0) {
                val[0].led = (j % 2 == 0) ? oledON : oledOFF;
            } else {
                val[0].led = (j % 2 == 0) ? oledOFF : oledON;
            }
            val[0].period = 64; // 8ms * 64 = 512ms
            OSYSDEBUG(("ledID[%d] %d\n", j, val[0].led));
        }
    }
}

void
BlinkingLED::BlinkLED()
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
