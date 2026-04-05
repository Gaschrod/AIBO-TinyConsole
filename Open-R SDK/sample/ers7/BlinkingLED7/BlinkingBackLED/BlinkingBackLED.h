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

#ifndef BlinkingBackLED_h_DEFINED
#define BlinkingBackLED_h_DEFINED

#include <OPENR/OObject.h>
#include <OPENR/OSubject.h>
#include <OPENR/OObserver.h>
#include "def.h"

enum BlinkingBackLEDState {
    BBLS_IDLE,
    BBLS_START,
    BBLS_WHITE0to255_COLOR0,
    BBLS_WHITE255_COLOR0to255,
    BBLS_WHITE255to0_COLOR255,
    BBLS_WHITE0_COLOR255to0,
    BBLS_WHITE0_COLOR0
};

enum LED3Result {
    LED3_CONT,
    LED3_FINISH
};

static const char* const LED3_LOCATOR[] = {
    "PRM:/lu-LED3:lu", // Back light (front,  color)
    "PRM:/lv-LED3:lv", // Back light (front,  white)
    "PRM:/lw-LED3:lw", // Back light (middle, color)
    "PRM:/lx-LED3:lx", // Back light (middle, white)
    "PRM:/ly-LED3:ly", // Back light (rear,   color)
    "PRM:/lz-LED3:lz"  // Back light (rear,   white) 
};

class BlinkingBackLED : public OObject {
public:
    BlinkingBackLED();
    virtual ~BlinkingBackLED() {}

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

    LED3Result White0to255_Color0();
    LED3Result White255_Color0to255();
    LED3Result White255to0_Color255();
    LED3Result White0_Color255to0();
    LED3Result White0_Color0();
    void SetWhiteLED3Value(RCRegion* rgn, int level, int slope);
    void SetColorLED3Value(RCRegion* rgn, int level, int slope);
    void SetWhiteLED3Value(RCRegion* rgn, sword intensity);
    void SetColorLED3Value(RCRegion* rgn, sword intensity);
    RCRegion* FindFreeRegion();

    static const size_t NUM_LEDS           = 6;
    static const size_t NUM_COMMAND_VECTOR = 2;
    static const int    MAX_LEVEL          = 15;
    static const int    POSITIVE_SLOPE     = 1;
    static const int    NEGATIVE_SLOPE     = -1;

    BlinkingBackLEDState  blinkingLEDState;
    OPrimitiveID          ledID[NUM_LEDS];
    RCRegion*             region[NUM_COMMAND_VECTOR];
};

#endif // BlinkingBackLED_h_DEFINED
