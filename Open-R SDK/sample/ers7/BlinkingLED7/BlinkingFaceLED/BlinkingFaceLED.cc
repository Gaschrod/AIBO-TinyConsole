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
#include "BlinkingFaceLED.h"

BlinkingFaceLED::BlinkingFaceLED() : blinkingLEDState(BFLS_IDLE)
{
    for (int i = 0; i < NUM_LEDS; i++) ledID[i] = oprimitiveID_UNDEF;
    for (int i = 0; i < NUM_COMMAND_VECTOR; i++) region[i] = 0;
}

OStatus
BlinkingFaceLED::DoInit(const OSystemEvent& event)
{
    OSYSDEBUG(("BlinkingFaceLED::DoInit()\n"));

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
BlinkingFaceLED::DoStart(const OSystemEvent& event)
{
    OSYSDEBUG(("BlinkingFaceLED::DoStart()\n"));

    if (subject[sbjBlink]->IsReady() == true) {
        blinkingLEDState = BFLS_MODE_A_0to255;
        ModeA_0to255();
    } else {
        blinkingLEDState = BFLS_START;
    }

    ENABLE_ALL_SUBJECT;
    ASSERT_READY_TO_ALL_OBSERVER;

    return oSUCCESS;
}

OStatus
BlinkingFaceLED::DoStop(const OSystemEvent& event)
{
    OSYSDEBUG(("BlinkingFaceLED::DoStop()\n"));

    blinkingLEDState = BFLS_IDLE;

    DISABLE_ALL_SUBJECT;    
    DEASSERT_READY_TO_ALL_OBSERVER;

    return oSUCCESS;
}

OStatus
BlinkingFaceLED::DoDestroy(const OSystemEvent& event)
{
    DELETE_ALL_SUBJECT_AND_OBSERVER;
    return oSUCCESS;
}

void
BlinkingFaceLED::Ready(const OReadyEvent& event)
{
    OSYSDEBUG(("BlinkingFaceLED::Ready()\n"));

    if (blinkingLEDState == BFLS_IDLE) {

        OSYSDEBUG(("BFLS_IDLE\n"));
        ; // do nothing

    } else if (blinkingLEDState == BFLS_START) {

        OSYSDEBUG(("BFLS_START\n"));
        blinkingLEDState = BFLS_MODE_A_0to255;
        ModeA_0to255();

    } else if (blinkingLEDState == BFLS_MODE_A_0to255) {

        OSYSDEBUG(("BFLS_MODE_A_0to255\n"));
        LED3Result r = ModeA_0to255();
        if (r == LED3_FINISH) {
            blinkingLEDState = BFLS_MODE_A_255to0;
        }

    } else if (blinkingLEDState == BFLS_MODE_A_255to0) {

        OSYSDEBUG(("BFLS_MODE_A_255to0\n"));
        LED3Result r = ModeA_255to0();
        if (r == LED3_FINISH) {
            blinkingLEDState = BFLS_MODE_A_0;
        }

    } else if (blinkingLEDState == BFLS_MODE_A_0) {

        OSYSDEBUG(("BFLS_MODE_A_0\n"));
        LED3Result r = ModeA_0();
        if (r == LED3_FINISH) {
            blinkingLEDState = BFLS_MODE_B_0to255;
        }
        
    } else if (blinkingLEDState == BFLS_MODE_B_0to255) {

        OSYSDEBUG(("BFLS_MODE_B_0to255\n"));
        LED3Result r = ModeB_0to255();
        if (r == LED3_FINISH) {
            blinkingLEDState = BFLS_MODE_B_255to0;
        }

    } else if (blinkingLEDState == BFLS_MODE_B_255to0) {

        OSYSDEBUG(("BFLS_MODE_B_255to0\n"));
        LED3Result r = ModeB_255to0();
        if (r == LED3_FINISH) {
            blinkingLEDState = BFLS_MODE_B_0;
        }

    } else { // blinkingLEDState == BFLS_MODE_B_0

        OSYSDEBUG(("BFLS_MODE_B_0\n"));
        LED3Result r = ModeB_0();
        if (r == LED3_FINISH) {
            blinkingLEDState = BFLS_MODE_A_0to255;
        }
    }
}

void
BlinkingFaceLED::OpenPrimitives()
{
    for (int i = 0; i < NUM_LEDS; i++) {
        OStatus result = OPENR::OpenPrimitive(LED3_LOCATOR[i], &ledID[i]);
        if (result != oSUCCESS) {
            OSYSLOG1((osyslogERROR, "%s : %s %d",
                      "BlinkingFaceLED::OpenPrimitives()",
                      "OPENR::OpenPrimitive() FAILED", result));
        }
    }
}

void
BlinkingFaceLED::NewCommandVectorData()
{
    OStatus result;
    MemoryRegionID      cmdVecDataID;
    OCommandVectorData* cmdVecData;

    for (int i = 0; i < NUM_COMMAND_VECTOR; i++) {

        result = OPENR::NewCommandVectorData(NUM_LEDS, 
                                             &cmdVecDataID, &cmdVecData);
        if (result != oSUCCESS) {
            OSYSLOG1((osyslogERROR, "%s : %s %d",
                      "BlinkingFaceLED::NewCommandVectorData()",
                      "OPENR::NewCommandVectorData() FAILED", result));
        }

        region[i] = new RCRegion(cmdVecData->vectorInfo.memRegionID,
                                 cmdVecData->vectorInfo.offset,
                                 (void*)cmdVecData,
                                 cmdVecData->vectorInfo.totalSize);

        cmdVecData->SetNumData(NUM_LEDS);

        for (int j = 0; j < NUM_LEDS; j++) {
            OCommandInfo* info = cmdVecData->GetInfo(j);
            info->Set(odataLED_COMMAND3, ledID[j], ocommandMAX_FRAMES);
        }
    }
}

LED3Result
BlinkingFaceLED::ModeA_0to255()
{
    static int level = -1;

    if (level == -1) {
        level = 0;
        RCRegion* rgn = FindFreeRegion();
        SetLED3Value(rgn, level, POSITIVE_SLOPE, oled3_MODE_A);
        subject[sbjBlink]->SetData(rgn);
        level++;
    } 

    RCRegion* rgn = FindFreeRegion();
    SetLED3Value(rgn, level, POSITIVE_SLOPE, oled3_MODE_A);
    subject[sbjBlink]->SetData(rgn);
    subject[sbjBlink]->NotifyObservers();
    level++;

    if (level > MAX_LEVEL) {
        level = 0;
        return LED3_FINISH;
    } else {
        return LED3_CONT;
    }
}

LED3Result
BlinkingFaceLED::ModeA_255to0()
{
    static int level = MAX_LEVEL;

    RCRegion* rgn = FindFreeRegion();
    SetLED3Value(rgn, level, NEGATIVE_SLOPE, oled3_MODE_A);
    subject[sbjBlink]->SetData(rgn);
    subject[sbjBlink]->NotifyObservers();
    level--;

    if (level < 0) {
        level = MAX_LEVEL;
        return LED3_FINISH;
    } else {
        return LED3_CONT;
    }
}

LED3Result
BlinkingFaceLED::ModeA_0()
{
    static int level = 0;

    RCRegion* rgn = FindFreeRegion();
    SetLED3Value(rgn, 0, oled3_MODE_A);
    subject[sbjBlink]->SetData(rgn);
    subject[sbjBlink]->NotifyObservers();
    level++;

    if (level > MAX_LEVEL) {
        level = 0;
        return LED3_FINISH;
    } else {
        return LED3_CONT;
    }
}

LED3Result
BlinkingFaceLED::ModeB_0to255()
{
    static int level = 0;
    
    RCRegion* rgn = FindFreeRegion();
    SetLED3Value(rgn, level, POSITIVE_SLOPE, oled3_MODE_B);
    subject[sbjBlink]->SetData(rgn);
    subject[sbjBlink]->NotifyObservers();
    level++;

    if (level > MAX_LEVEL) {
        level = 0;
        return LED3_FINISH;
    } else {
        return LED3_CONT;
    }
}

LED3Result
BlinkingFaceLED::ModeB_255to0()
{
    static int level = MAX_LEVEL;

    RCRegion* rgn = FindFreeRegion();
    SetLED3Value(rgn, level, NEGATIVE_SLOPE, oled3_MODE_B);
    subject[sbjBlink]->SetData(rgn);
    subject[sbjBlink]->NotifyObservers();
    level--;

    if (level < 0) {
        level = MAX_LEVEL;
        return LED3_FINISH;
    } else {
        return LED3_CONT;
    }
}

LED3Result
BlinkingFaceLED::ModeB_0()
{
    static int level = 0;

    RCRegion* rgn = FindFreeRegion();
    SetLED3Value(rgn, 0, oled3_MODE_B);
    subject[sbjBlink]->SetData(rgn);
    subject[sbjBlink]->NotifyObservers();
    level++;

    if (level > MAX_LEVEL) {
        level = 0;
        return LED3_FINISH;
    } else {
        return LED3_CONT;
    }
}

void
BlinkingFaceLED::SetLED3Value(RCRegion* rgn, 
                              int level, int slope, OLED3Mode mode)
{
    OCommandVectorData* cmdVecData = (OCommandVectorData*)rgn->Base();

    for (int i = 0; i < NUM_LEDS; i++) {
        OCommandData* data = cmdVecData->GetData(i);
        OLEDCommandValue3* val = (OLEDCommandValue3*)data->value;
        for (int j = 0; j < ocommandMAX_FRAMES; j++) {
            if (slope == POSITIVE_SLOPE) {
                val[j].intensity = 16 * level + j;
            } else {
                val[j].intensity = 16 * level + 15 - j;
            }
            val[j].mode      = mode;
            val[j].period    = 1;
        }
    }
}

void
BlinkingFaceLED::SetLED3Value(RCRegion* rgn, sword intensity, OLED3Mode mode)
{
    OCommandVectorData* cmdVecData = (OCommandVectorData*)rgn->Base();

    for (int i = 0; i < NUM_LEDS; i++) {
        OCommandData* data = cmdVecData->GetData(i);
        OLEDCommandValue3* val = (OLEDCommandValue3*)data->value;
        for (int j = 0; j < ocommandMAX_FRAMES; j++) {
            val[j].intensity = intensity;
            val[j].mode      = mode;
            val[j].period    = 1;
        }
    }
}

RCRegion*
BlinkingFaceLED::FindFreeRegion()
{
    for (int i = 0; i < NUM_COMMAND_VECTOR; i++) {
        if (region[i]->NumberOfReference() == 1) return region[i];
    }

    return 0;
}
