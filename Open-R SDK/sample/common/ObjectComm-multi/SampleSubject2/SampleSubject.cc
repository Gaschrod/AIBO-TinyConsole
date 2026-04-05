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

#include <string.h>
#include <OPENR/OSyslog.h>
#include <OPENR/core_macro.h>
#include "SampleSubject.h"

SampleSubject::SampleSubject()
{
}

OStatus
SampleSubject::DoInit(const OSystemEvent& event)
{
    NEW_ALL_SUBJECT_AND_OBSERVER;
    REGISTER_ALL_ENTRY;
    SET_ALL_READY_AND_NOTIFY_ENTRY;
    return oSUCCESS;
}

OStatus
SampleSubject::DoStart(const OSystemEvent& event)
{
    ENABLE_ALL_SUBJECT;
    ASSERT_READY_TO_ALL_OBSERVER;
    return oSUCCESS;
}    

OStatus
SampleSubject::DoStop(const OSystemEvent& event)
{
    DISABLE_ALL_SUBJECT;
    DEASSERT_READY_TO_ALL_OBSERVER;
    return oSUCCESS;
}

OStatus
SampleSubject::DoDestroy(const OSystemEvent& event)
{
    DELETE_ALL_SUBJECT_AND_OBSERVER;
    return oSUCCESS;
}

void 
SampleSubject::Ready(const OReadyEvent& event)
{ 
    OSYSPRINT(("SampleSubject2::Ready() : %s\n",
               event.IsAssert() ? "ASSERT READY" : "DEASSERT READY"));

    char str[32];
    strcpy(str, "!!! Goodbye world !!!");
    subject[sbjSendString]->SetData(str, sizeof(str));  
    subject[sbjSendString]->NotifyObservers();  
}
