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

#ifndef _OMPsdRH_h_DEFINED
#define _OMPsdRH_h_DEFINED

#include <OPENR/OPENR.h>
#include <OPENR/OObserver.h>

struct PsdResults;

class OMPsdRH 
{
private:
    PsdResults* psdResults;
    int         flag;

public:
    OMPsdRH();
    ~OMPsdRH();
    
    OStatus InportResult(void** msg);
    int     GetStatus(void);
    int     GetRangeToObstacle(void);
    int     GetTimeStamp(void);
};

#endif  //  _OMPsdRH_h_DEFINED
