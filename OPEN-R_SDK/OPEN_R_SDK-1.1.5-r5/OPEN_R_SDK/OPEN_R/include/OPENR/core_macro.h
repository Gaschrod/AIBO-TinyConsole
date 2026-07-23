//
// core_macro.h
//
// Copyright 1999-2002 Sony Corporation
//

#ifndef New_core_macro_h_DEFINED
#define New_core_macro_h_DEFINED

#include "entry.h"
#include <OPENR/OPENR.h>

#define ASSERT_READY_TO_ALL_OBSERVER \
{ \
    for ( int i = 0; i < numOfObserver; ++i ) \
	observer[ i ]->AssertReady(); \
}

#define DEASSERT_READY_TO_ALL_OBSERVER \
{ \
    for ( int i = 0; i < numOfObserver; ++i ) \
	observer[ i ]->DeassertReady(); \
}

#define REGISTER_ALL_ENTRY \
{ \
    for ( int i = 0; i < numOfSubject; ++i ) { \
        RegisterServiceEntry(OServiceEntry( myOID_, \
                             Entry_Control[ i ] ), \
                             subjectService[ i ] ); \
    } \
\
    for ( int i = 0; i < numOfObserver; ++i ) { \
        RegisterServiceEntry(OServiceEntry( myOID_, \
                             Entry_Connect[ i ] ), \
                             observerService[ i ] ); \
    } \
}

#define SET_ALL_READY_AND_NOTIFY_ENTRY \
{ \
    OServiceEntry entry; \
\
    for ( int i = 0; i < numOfSubject; ++i ) {  \
	entry.Set( myOID_, Entry_Ready[ i ]);  \
	subject[ i ]->SetReadyEntry( entry );  \
    }  \
 \
    for ( int i = 0; i < numOfObserver; ++i ) {  \
	entry.Set( myOID_, Entry_Notify[ i ] );  \
	observer[ i ]->SetNotifyEntry( entry );  \
    }  \
}

#define NEW_ALL_SUBJECT_AND_OBSERVER \
{ \
    for ( int i = 0; i < numOfSubject; ++i ) \
	subject[ i ] = new OSubject(); \
\
    for ( int i = 0; i < numOfObserver; ++i ) \
	observer[ i ] = new OObserver(); \
}

// NOT YET
#define DELETE_ALL_SUBJECT_AND_OBSERVER

// NOT YET
#define ENABLE_ALL_SUBJECT

#define DISABLE_ALL_SUBJECT \
{ \
    for (int i = 0; i < numOfSubject; ++i) \
    subject[i]->ClearBuffer(); \
}

//
// support for old define
//
#define ASSERT_TO_ALL_OBSERVER \
{ \
    for ( int i = 0; i < numOfObserver; ++i ) \
	observer[ i ]->AssertReady(); \
}

#define DEASSERT_TO_ALL_OBSERVER \
{ \
    for ( int i = 0; i < numOfObserver; ++i ) \
	observer[ i ]->DeassertReady(); \
}

#define NewComData \
NEW_ALL_SUBJECT_AND_OBSERVER

#define DeleteComData \
DELETE_ALL_SUBJECT_AND_OBSERVER

#define SetComData \
REGISTER_ALL_ENTRY \
SET_ALL_READY_AND_NOTIFY_ENTRY

#endif // _core_macro_h_DEFINED
