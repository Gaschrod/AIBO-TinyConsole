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

#ifndef BlinkingHeadLED_h_DEFINED
#define BlinkingHeadLED_h_DEFINED

#include <OPENR/OObject.h>
#include <OPENR/OSubject.h>
#include <OPENR/OObserver.h>
#include "def.h"

enum BlinkingHeadLEDState {
    BHLS_IDLE,
    BHLS_START
};

static const char* const LED2_LOCATOR[] = {
    "PRM:/r1/c1/c2/c3/l1-LED2:l1", // Head light (color)
    "PRM:/r1/c1/c2/c3/l2-LED2:l2", // Head light (white)
    "PRM:/r1/c1/c2/c3/l3-LED2:l3", // Mode Indicator (red)
    "PRM:/r1/c1/c2/c3/l4-LED2:l4", // Mode Indicator (green)
    "PRM:/r1/c1/c2/c3/l5-LED2:l5", // Mode Indicator (blue)
    "PRM:/r1/c1/c2/c3/l6-LED2:l6", // Wireless light
};

const size_t NUM_LEDS   = 6;
const size_t NUM_FRAMES = 8;
const OLEDValue LED2_VALUE[NUM_LEDS][NUM_FRAMES] = {
    { oledON,  oledON,  oledOFF, oledOFF, oledON,  oledON,  oledOFF, oledOFF },
    { oledOFF, oledON,  oledON,  oledOFF, oledOFF, oledON,  oledON,  oledOFF },
    { oledON,  oledOFF, oledOFF, oledOFF, oledON,  oledON,  oledON,  oledOFF },
    { oledOFF, oledON,  oledOFF, oledON,  oledOFF, oledON,  oledON,  oledOFF },
    { oledOFF, oledOFF, oledON,  oledON,  oledON,  oledOFF, oledON,  oledOFF },
    { oledON,  oledOFF, oledON,  oledOFF, oledON,  oledOFF, oledON,  oledOFF }
};

class BlinkingHeadLED : public OObject {
public:
    BlinkingHeadLED();
    virtual ~BlinkingHeadLED() {}

    OSubject*  subject[numOfSubject];
    OObserver* observer[numOfObserver];

    virtual OStatus DoInit   (const OSystemEvent& event);
    virtual OStatus DoStart  (const OSystemEvent& event);
    virtual OStatus DoStop   (const OSystemEvent& event);
    virtual OStatus DoDestroy(const OSystemEvent& event);

    void Ready(const OReadyEvent& event);

private:
    void OpenPrimitives();
    void NewCommandVectorData();
    void BlinkLED();

    static const size_t NUM_COMMAND_VECTOR = 2;

    BlinkingHeadLEDState  blinkingLEDState;
    OPrimitiveID          ledID[NUM_LEDS];
    RCRegion*             region[NUM_COMMAND_VECTOR];
};

#endif // BlinkingHeadLED_h_DEFINED
