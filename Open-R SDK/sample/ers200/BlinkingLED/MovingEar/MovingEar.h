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

#ifndef MovingEar_h_DEFINED
#define MovingEar_h_DEFINED

#include <OPENR/OObject.h>
#include <OPENR/OSubject.h>
#include <OPENR/OObserver.h>
#include "def.h"

enum MovingEarState {
    MES_IDLE,
    MES_START
};

static const char* const EAR_LOCATOR[] = {
    "PRM:/r1/c1/c2/c3/e1-Joint3:j5",
    "PRM:/r1/c1/c2/c3/e2-Joint3:j6",
};

class MovingEar : public OObject {
public:
    MovingEar();
    virtual ~MovingEar() {}

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
    void MoveEar();

    static const size_t NUM_COMMAND_VECTOR = 2;
    static const size_t NUM_EARS           = 2;

    MovingEarState  movingEarState;
    OPrimitiveID    earID[NUM_EARS];
    RCRegion*       region[NUM_COMMAND_VECTOR];
};

#endif // MovingEar_h_DEFINED
