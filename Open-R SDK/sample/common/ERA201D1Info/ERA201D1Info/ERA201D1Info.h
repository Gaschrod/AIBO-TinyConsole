//
// Copyright 2002,2003 Sony Corporation 
//
// Permission to use, copy, modify, and redistribute this software for
// non-commercial use is hereby granted.
//
// This software is provided "as is" without warranty of any kind,
// either expressed or implied, including but not limited to the
// implied warranties of fitness for a particular purpose.
//

#ifndef ERA201D1Info_h_DEFINED
#define ERA201D1Info_h_DEFINED

#include <ERA201D1.h>
#include <OPENR/OObject.h>
#include <OPENR/OSubject.h>
#include <OPENR/OObserver.h>
#include "def.h"

class ERA201D1Info : public OObject {
public:
    ERA201D1Info();
    virtual ~ERA201D1Info() {}

    OSubject*   subject[numOfSubject];
    OObserver*	observer[numOfObserver];     

    virtual OStatus DoInit   (const OSystemEvent& event);
    virtual OStatus DoStart  (const OSystemEvent& event);
    virtual OStatus DoStop   (const OSystemEvent& event);
    virtual OStatus DoDestroy(const OSystemEvent& event);

    void PrintMACAddress(const EtherDriverGetMACAddressMsg& msg);
    void PrintEtherStatistics(const EtherDriverGetStatisticsMsg& msg);
    void PrintWLANSettings(const EtherDriverGetWLANSettingsMsg& msg);
    void PrintWLANStatistics(const EtherDriverGetWLANStatisticsMsg& msg);
};

#endif // ERA201D1Info_h_DEFINED
