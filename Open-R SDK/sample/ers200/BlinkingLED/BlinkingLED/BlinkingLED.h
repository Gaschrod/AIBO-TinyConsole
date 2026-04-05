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

#ifndef BlinkingLED_h_DEFINED
#define BlinkingLED_h_DEFINED

#include <OPENR/OObject.h>
#include <OPENR/OSubject.h>
#include <OPENR/OObserver.h>
#include "def.h"

enum BlinkingLEDState {
    BLS_IDLE,
    BLS_START
};

static const char* const LED_LOCATOR[] = {
    "PRM:/r1/c1/c2/c3/l1-LED2:l1",
    "PRM:/r1/c1/c2/c3/l2-LED2:l2",
    "PRM:/r1/c1/c2/c3/l3-LED2:l3",
    "PRM:/r1/c1/c2/c3/l4-LED2:l4",
    "PRM:/r1/c1/c2/c3/l5-LED2:l5",
    "PRM:/r1/c1/c2/c3/l6-LED2:l6",
    "PRM:/r1/c1/c2/c3/l7-LED2:l7",
};   

class BlinkingLED : public OObject {
public:
    BlinkingLED();
    virtual ~BlinkingLED() {}

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
    static const size_t NUM_LEDS           = 7;

    BlinkingLEDState  blinkingLEDState;
    OPrimitiveID      ledID[NUM_LEDS];
    RCRegion*         region[NUM_COMMAND_VECTOR];
};

#endif // BlinkingLED_h_DEFINED
