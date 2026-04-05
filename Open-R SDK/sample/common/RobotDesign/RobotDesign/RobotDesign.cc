//
// Copyright 2002,2004 Sony Corporation 
//
// Permission to use, copy, modify, and redistribute this software for
// non-commercial use is hereby granted.
//
// This software is provided "as is" without warranty of any kind,
// either expressed or implied, including but not limited to the
// implied warranties of fitness for a particular purpose.
//

#include <OPENR/OSyslog.h>
#include <OPENR/OPENRAPI.h>
#include "RobotDesign.h"
#include "entry.h"

RobotDesign::RobotDesign()
{
}

OStatus
RobotDesign::DoInit(const OSystemEvent& event)
{
    OSYSDEBUG(("RobotDesign::DoInit()\n"));
    return oSUCCESS;
}

OStatus
RobotDesign::DoStart(const OSystemEvent& event)
{
    OSYSDEBUG(("RobotDesign::DoStart()\n"));

    OStatus result;
    char design[orobotdesignNAME_MAX+1];
    ODesignDataID dataID;
    byte* addr;
    size_t size = 0;

    result = OPENR::GetRobotDesign(design);
    if (result != oSUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s %d",
                  "RobotDesign::DoStart()",
                  "OPENR::GetRobotDesign() FAILED", result));
        return oFAIL;
    }

    OSYSPRINT(("OPENR::GetRobotDesign() : %s\n", design));
    
    result= OPENR::FindDesignData("SAMPLE_DATA", &dataID, &addr, &size);
    if (result != oSUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s %d",
                  "RobotDesign::DoStart()",
                  "OPENR::FindDesignData() FAILED", result));
        return oFAIL;
    }

    memcpy(design, addr, size);
    design[size] = '\0';
    OSYSPRINT(("%s", design));

    result= OPENR::DeleteDesignData(dataID);
    if (result != oSUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s %d",
                  "RobotDesign::DoStart()",
                  "OPENR::DeleteDesignData() FAILED", result));
        return oFAIL;
    }

    return oSUCCESS;
}    

OStatus
RobotDesign::DoStop(const OSystemEvent& event)
{
    OSYSDEBUG(("RobotDesign::DoStop()\n"));
    return oSUCCESS;
}

OStatus
RobotDesign::DoDestroy(const OSystemEvent& event)
{
    return oSUCCESS;
}
