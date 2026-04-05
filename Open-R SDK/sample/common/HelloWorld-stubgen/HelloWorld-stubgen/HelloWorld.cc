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

#include <OPENR/OSyslog.h>
#include "HelloWorld.h"

HelloWorld::HelloWorld ()
{
    OSYSDEBUG(("HelloWorld::HelloWorld()\n"));
}

OStatus
HelloWorld::DoInit(const OSystemEvent& event)
{
    OSYSDEBUG(("HelloWorld::DoInit()\n"));
    return oSUCCESS;
}

OStatus
HelloWorld::DoStart(const OSystemEvent& event)
{
    OSYSDEBUG(("HelloWorld::DoStart()\n"));
    OSYSPRINT(("!!! Hello World !!!\n"));
    return oSUCCESS;
}    

OStatus
HelloWorld::DoStop(const OSystemEvent& event)
{
    OSYSDEBUG(("HelloWorld::DoStop()\n"));
    OSYSLOG1((osyslogERROR, "Bye Bye ..."));
    return oSUCCESS;
}

OStatus
HelloWorld::DoDestroy(const OSystemEvent& event)
{
    return oSUCCESS;
}
