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
#include "BlinkingBackLED.h"

BlinkingBackLED::BlinkingBackLED() : blinkingLEDState(BBLS_IDLE)
{
    for (int i = 0; i < NUM_LEDS; i++) ledID[i] = oprimitiveID_UNDEF;
    for (int i = 0; i < NUM_COMMAND_VECTOR; i++) region[i] = 0;
}

OStatus
BlinkingBackLED::DoInit(const OSystemEvent& event)
{
    OSYSDEBUG(("BlinkingBackLED::DoInit()\n"));

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
BlinkingBackLED::DoStart(const OSystemEvent& event)
{
    OSYSDEBUG(("BlinkingBackLED::DoStart()\n"));

    if (subject[sbjBlink]->IsReady() == true) {
        blinkingLEDState = BBLS_WHITE0to255_COLOR0;
        White0to255_Color0();
    } else {
        blinkingLEDState = BBLS_START;
    }

    ENABLE_ALL_SUBJECT;
    ASSERT_READY_TO_ALL_OBSERVER;

    return oSUCCESS;
}

OStatus
BlinkingBackLED::DoStop(const OSystemEvent& event)
{
    OSYSDEBUG(("BlinkingBackLED::DoStop()\n"));

    blinkingLEDState = BBLS_IDLE;

    DISABLE_ALL_SUBJECT;    
    DEASSERT_READY_TO_ALL_OBSERVER;

    return oSUCCESS;
}

OStatus
BlinkingBackLED::DoDestroy(const OSystemEvent& event)
{
    DELETE_ALL_SUBJECT_AND_OBSERVER;
    return oSUCCESS;
}

void
BlinkingBackLED::Ready(const OReadyEvent& event)
{
    OSYSDEBUG(("BlinkingBackLED::Ready()\n"));

    if (blinkingLEDState == BBLS_IDLE) {

        OSYSDEBUG(("BBLS_IDLE\n"));
        ; // do nothing

    } else if (blinkingLEDState == BBLS_START) {

        OSYSDEBUG(("BBLS_START\n"));
        blinkingLEDState = BBLS_WHITE0to255_COLOR0;
        White0to255_Color0();

    } else if (blinkingLEDState == BBLS_WHITE0to255_COLOR0) {

        OSYSDEBUG(("BBLS_WHITE0to255_COLOR0\n"));
        LED3Result r = White0to255_Color0();
        if (r == LED3_FINISH) {
            blinkingLEDState = BBLS_WHITE255_COLOR0to255;
        }

    } else if (blinkingLEDState == BBLS_WHITE255_COLOR0to255) {

        OSYSDEBUG(("BBLS_WHITE255_COLOR0to255\n"));
        LED3Result r = White255_Color0to255();
        if (r == LED3_FINISH) {
            blinkingLEDState = BBLS_WHITE255to0_COLOR255;
        }

    } else if (blinkingLEDState == BBLS_WHITE255to0_COLOR255) {

        OSYSDEBUG(("BBLS_WHITE255to0_COLOR255\n"));
        LED3Result r = White255to0_Color255();
        if (r == LED3_FINISH) {
            blinkingLEDState = BBLS_WHITE0_COLOR255to0;
        }

    } else if (blinkingLEDState == BBLS_WHITE0_COLOR255to0) {

        OSYSDEBUG(("BBLS_WHITE0_COLOR255to0\n"));
        LED3Result r = White0_Color255to0();
        if (r == LED3_FINISH) {
            blinkingLEDState = BBLS_WHITE0_COLOR0;
        }

    } else { // blinkingLEDState == BBLS_WHITE0_COLOR0

        OSYSDEBUG(("BBLS_WHITE0_COLOR0\n"));
        LED3Result r = White0_Color0();
        if (r == LED3_FINISH) {
            blinkingLEDState = BBLS_WHITE0to255_COLOR0;
        }

    }
}

void
BlinkingBackLED::OpenPrimitives()
{
    for (int i = 0; i < NUM_LEDS; i++) {
        OStatus result = OPENR::OpenPrimitive(LED3_LOCATOR[i], &ledID[i]);
        if (result != oSUCCESS) {
            OSYSLOG1((osyslogERROR, "%s : %s %d",
                      "BlinkingBackLED::OpenPrimitives()",
                      "OPENR::OpenPrimitive() FAILED", result));
        }
    }
}

void
BlinkingBackLED::NewCommandVectorData()
{
    OStatus result;
    MemoryRegionID      cmdVecDataID;
    OCommandVectorData* cmdVecData;

    for (int i = 0; i < NUM_COMMAND_VECTOR; i++) {

        result = OPENR::NewCommandVectorData(NUM_LEDS, 
                                             &cmdVecDataID, &cmdVecData);
        if (result != oSUCCESS) {
            OSYSLOG1((osyslogERROR, "%s : %s %d",
                      "BlinkingBackLED::NewCommandVectorData()",
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
BlinkingBackLED::White0to255_Color0()
{
    static int level = -1;

    if (level == -1) {
        level = 0;
        RCRegion* rgn = FindFreeRegion();
        SetWhiteLED3Value(rgn, level, POSITIVE_SLOPE);
        SetColorLED3Value(rgn, 0);
        subject[sbjBlink]->SetData(rgn);
        level++;
    } 

    RCRegion* rgn = FindFreeRegion();
    SetWhiteLED3Value(rgn, level, POSITIVE_SLOPE);
    SetColorLED3Value(rgn, 0);
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
BlinkingBackLED::White255_Color0to255()
{
    static int level = 0;

    RCRegion* rgn = FindFreeRegion();
    SetWhiteLED3Value(rgn, 255);
    SetColorLED3Value(rgn, level, POSITIVE_SLOPE);
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
BlinkingBackLED::White255to0_Color255()
{
    static int level = MAX_LEVEL;

    RCRegion* rgn = FindFreeRegion();
    SetWhiteLED3Value(rgn, level, NEGATIVE_SLOPE);
    SetColorLED3Value(rgn, 255);
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
BlinkingBackLED::White0_Color255to0()
{
    static int level = MAX_LEVEL;

    RCRegion* rgn = FindFreeRegion();
    SetWhiteLED3Value(rgn, 0);
    SetColorLED3Value(rgn, level, NEGATIVE_SLOPE);
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
BlinkingBackLED::White0_Color0()
{
    static int level = 0;

    RCRegion* rgn = FindFreeRegion();
    SetWhiteLED3Value(rgn, 0);
    SetColorLED3Value(rgn, 0);
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
BlinkingBackLED::SetWhiteLED3Value(RCRegion* rgn, int level, int slope)
{
    OCommandVectorData* cmdVecData = (OCommandVectorData*)rgn->Base();

    for (int i = 0; i < NUM_LEDS/2; i++) {
        OCommandData* wdata = cmdVecData->GetData(2*i+1);
        OLEDCommandValue3* wval3 = (OLEDCommandValue3*)wdata->value;
        for (int j = 0; j < ocommandMAX_FRAMES; j++) {
            if (slope == POSITIVE_SLOPE) {
                wval3[j].intensity = 16 * level + j;
            } else {
                wval3[j].intensity = 16 * level + 15 - j;
            }
            wval3[j].mode      = oled3_MODE_UNDEF;
            wval3[j].period    = 1;
        }
    }
}

void
BlinkingBackLED::SetColorLED3Value(RCRegion* rgn, int level, int slope)
{
    OCommandVectorData* cmdVecData = (OCommandVectorData*)rgn->Base();

    for (int i = 0; i < NUM_LEDS/2; i++) {
        OCommandData* cdata = cmdVecData->GetData(2*i);
        OLEDCommandValue3* cval3 = (OLEDCommandValue3*)cdata->value;
        for (int j = 0; j < ocommandMAX_FRAMES; j++) {
            if (slope == POSITIVE_SLOPE) {
                cval3[j].intensity = 16 * level + j;                
            } else {
                cval3[j].intensity = 16 * level + 15 - j;
            }
            cval3[j].mode      = oled3_MODE_UNDEF;
            cval3[j].period    = 1;
        }
    }
}

void
BlinkingBackLED::SetWhiteLED3Value(RCRegion* rgn, sword intensity)
{
    OCommandVectorData* cmdVecData = (OCommandVectorData*)rgn->Base();

    for (int i = 0; i < NUM_LEDS/2; i++) {
        OCommandData* wdata = cmdVecData->GetData(2*i+1);
        OLEDCommandValue3* wval3 = (OLEDCommandValue3*)wdata->value;
        for (int j = 0; j < ocommandMAX_FRAMES; j++) {
            wval3[j].intensity = intensity;
            wval3[j].mode      = oled3_MODE_UNDEF;
            wval3[j].period    = 1;
        }
    }
}

void
BlinkingBackLED::SetColorLED3Value(RCRegion* rgn, sword intensity)
{
    OCommandVectorData* cmdVecData = (OCommandVectorData*)rgn->Base();

    for (int i = 0; i < NUM_LEDS/2; i++) {
        OCommandData* cdata = cmdVecData->GetData(2*i);
        OLEDCommandValue3* cval3 = (OLEDCommandValue3*)cdata->value;
        for (int j = 0; j < ocommandMAX_FRAMES; j++) {
            cval3[j].intensity = intensity;
            cval3[j].mode      = oled3_MODE_UNDEF;
            cval3[j].period    = 1;
        }
    }
}

RCRegion*
BlinkingBackLED::FindFreeRegion()
{
    for (int i = 0; i < NUM_COMMAND_VECTOR; i++) {
        if (region[i]->NumberOfReference() == 1) return region[i];
    }

    return 0;
}
