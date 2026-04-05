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

#include <MCOOP.h>
#include <ObjectEntryTable.h>
#include <apsys.h>
#include "Crash.h"

Crash Self;

//
//  Stub Function Definitions
//
extern "C" void
InitStub(OReasonMessage* reasonMsg)
{
    Self.Init( OSystemEvent(reasonMsg->reason,
			    reasonMsg->param, reasonMsg->paramSize) );
    Return();
}

extern "C" void
StartStub(OReasonMessage* reasonMsg)
{
    Self.Start( OSystemEvent(reasonMsg->reason,
			     reasonMsg->param, reasonMsg->paramSize) );
    Return();
}

extern "C" void
StopStub(OReasonMessage* reasonMsg)
{
    Self.Stop( OSystemEvent(reasonMsg->reason,
			    reasonMsg->param, reasonMsg->paramSize) );
    Return();
}

extern "C" void
DestroyStub(OReasonMessage* reasonMsg)
{
    Self.Destroy( OSystemEvent(reasonMsg->reason,
			       reasonMsg->param, reasonMsg->paramSize) );
    Return();
}

//
// Entry Table
//
GEN_ENTRY(InitEntry, InitStub);
GEN_ENTRY(StartEntry, StartStub);
GEN_ENTRY(StopEntry, StopStub);
GEN_ENTRY(DestroyEntry, DestroyStub);
GEN_ENTRY(PrologueEntry, Prologue);

ObjectEntry	ObjectEntryTable[] = {
    {oentryINIT,    (Entry)InitEntry},
    {oentrySTART,   (Entry)StartEntry},
    {oentrySTOP, 	(Entry)StopEntry},
    {oentryDESTROY, (Entry)DestroyEntry},
    {UNDEF,         (Entry) ENTRY_UNDEF}
};
