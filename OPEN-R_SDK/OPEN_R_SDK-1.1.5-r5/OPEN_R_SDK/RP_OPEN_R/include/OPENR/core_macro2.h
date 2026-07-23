//
// core_macro2.h
//
// Copyright 2001 Sony Corporation
//

#ifndef New_core_macro2_h_DEFINED
#define New_core_macro2_h_DEFINED

#include <string>
#include <OPENR/core_macro.h>

#undef REGISTER_ALL_ENTRY

// assuming that all the subjectService and observerService
// has the right format.(specifically, include '.' in it)
#define REGISTER_ALL_ENTRY  \
{ \
    OStatus result = oSUCCESS; \
\
    for ( int i = 0; i < numOfSubject; ++i ) { \
        std::string s( subjectService[i] ); \
        s.replace( 0, s.find('.'), myname ); \
	OStatus result2 = \
	    RegisterServiceEntry( OServiceEntry( myOID_, \
						 Entry_Control[ i ] ), \
				  s.c_str() ); \
	if ( result2 != oSUCCESS )  result = result2; \
    } \
\
    for ( int i = 0; i < numOfObserver; ++i ) { \
        std::string s( observerService[i] ); \
        s.replace( 0, s.find('.'), myname ); \
	OStatus result2 = \
	    RegisterServiceEntry( OServiceEntry( myOID_, \
						 Entry_Connect[ i ] ), \
				  s.c_str() ); \
	if ( result2 != oSUCCESS )  result = result2; \
    } \
}

#endif // _core_macro2_h_DEFINED
