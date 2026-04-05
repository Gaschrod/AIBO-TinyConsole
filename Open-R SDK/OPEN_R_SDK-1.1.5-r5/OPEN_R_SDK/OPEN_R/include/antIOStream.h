/*
 *  Copyright 2002 Sony Corporation
 */
#ifndef _ANTIOSTREAM_H_DEFINED_
#define _ANTIOSTREAM_H_DEFINED_


#ifdef __GNUC__

#include <iostream>
#define OSTREAM std::ostream
#define COUT std::cout
#define CIN std::cin

#else /* __GNUC__ */

#ifdef USE_LSTREAM

#include <lstream.h>
#include <iostream.h> 
#define OSTREAM lstream
#define COUT lout
#define CIN cin
// to prevent accidental use
#ifdef PARANOIA
#define cout Please_use_the_COUT_macro_in_stread_of_cout
#define ostream Please_use_the_OSTREAM_macro_in_stead_of_ostream
#endif

#else  // USE_LSTREAM

#include <iostream.h>
#define OSTREAM ostream
#define COUT cout
#define CIN cin

#endif // USE_LSTREAM

#endif /* __GNUC__ */

#endif // _ANTIOSTREAM_H_DEFINED_
