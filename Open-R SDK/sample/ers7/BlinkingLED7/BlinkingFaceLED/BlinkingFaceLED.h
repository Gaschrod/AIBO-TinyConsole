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

#ifndef BlinkingFaceLED_h_DEFINED
#define BlinkingFaceLED_h_DEFINED

#include <OPENR/OObject.h>
#include <OPENR/OSubject.h>
#include <OPENR/OObserver.h>
#include "def.h"

enum BlinkingFaceLEDState {
    BFLS_IDLE,
    BFLS_START,
    BFLS_MODE_A_0to255,
    BFLS_MODE_A_255to0,
    BFLS_MODE_A_0,
    BFLS_MODE_B_0to255,
    BFLS_MODE_B_255to0,
    BFLS_MODE_B_0
};

enum LED3Result {
    LED3_CONT,
    LED3_FINISH
};

static const char* const LED3_LOCATOR[] = {
    "PRM:/r1/c1/c2/c3/la-LED3:la", // Face light1
    "PRM:/r1/c1/c2/c3/lb-LED3:lb", // Face light2
    "PRM:/r1/c1/c2/c3/lc-LED3:lc", // Face light3
    "PRM:/r1/c1/c2/c3/ld-LED3:ld", // Face light4
    "PRM:/r1/c1/c2/c3/le-LED3:le", // Face light5
    "PRM:/r1/c1/c2/c3/lf-LED3:lf", // Face light6
    "PRM:/r1/c1/c2/c3/lg-LED3:lg", // Face light7
    "PRM:/r1/c1/c2/c3/lh-LED3:lh", // Face light8
    "PRM:/r1/c1/c2/c3/li-LED3:li", // Face light9
    "PRM:/r1/c1/c2/c3/lj-LED3:lj", // Face light10
    "PRM:/r1/c1/c2/c3/lk-LED3:lk", // Face light11
    "PRM:/r1/c1/c2/c3/ll-LED3:ll", // Face light12
    "PRM:/r1/c1/c2/c3/lm-LED3:lm", // Face light13
    "PRM:/r1/c1/c2/c3/ln-LED3:ln"  // Face light14
};

class BlinkingFaceLED : public OObject {
public:
    BlinkingFaceLED();
    virtual ~BlinkingFaceLED() {}

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

    LED3Result ModeA_0to255();
    LED3Result ModeA_255to0();
    LED3Result ModeA_0();
    LED3Result ModeB_0to255();
    LED3Result ModeB_255to0();
    LED3Result ModeB_0();
    void SetLED3Value(RCRegion* rgn, int level, int slope, OLED3Mode mode);
    void SetLED3Value(RCRegion* rgn, sword intensity, OLED3Mode mode);
    RCRegion* FindFreeRegion();

    static const size_t NUM_LEDS           = 14;
    static const size_t NUM_COMMAND_VECTOR = 2;
    static const int    MAX_LEVEL          = 15;
    static const int    POSITIVE_SLOPE     = 1;
    static const int    NEGATIVE_SLOPE     = -1;

    BlinkingFaceLEDState  blinkingLEDState;
    OPrimitiveID          ledID[NUM_LEDS];
    RCRegion*             region[NUM_COMMAND_VECTOR];
};

#endif // BlinkingFaceLED_h_DEFINED
