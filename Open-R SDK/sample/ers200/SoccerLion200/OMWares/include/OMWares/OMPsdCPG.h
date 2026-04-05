//
// Copyright 1999,2002 Sony Corporation 
//
// Permission to use, copy, modify, and redistribute this software for
// non-commercial use is hereby granted.
//
// This software is provided "as is" without warranty of any kind,
// either expressed or implied, including but not limited to the
// implied warranties of fitness for a particular purpose.
//

#ifndef _OMPsdCPG_h_DEFINED
#define _OMPsdCPG_h_DEFINED

#include <OPENR/OPENR.h>
#include <OPENR/OObserver.h>

struct PsdCntrl;

class OMPsdCPG 
{
private:
    PsdCntrl* psdCntrl;
    int       flag;

public:
    OMPsdCPG();
    ~OMPsdCPG();

    OStatus ProvideOMPsdControlParameter(void*& ompsdCP, size_t& sizeData);

    OStatus Generate_NotDefined(ObserverID client);
    OStatus Generate_CloserThan(ObserverID client, int threshold);
    OStatus Generate_FartherThan(ObserverID client, int threshold);
    OStatus Generate_Within(ObserverID client, int threshold1, int threshold2);
    OStatus Generate_NotWithin(ObserverID client, int threshold1, int threshold2);
    OStatus Generate_NoObstacle(ObserverID client);
    OStatus Generate_Reset(ObserverID client);
};

#endif  //  _OMPsdCPG_h_DEFINED
