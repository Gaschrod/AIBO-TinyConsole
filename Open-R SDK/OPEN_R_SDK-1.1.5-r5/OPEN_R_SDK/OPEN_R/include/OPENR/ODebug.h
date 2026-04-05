//
// ODebug.h
//
// Copyright 1997,1998,1999,2000,2002 Sony Corporation
//

#ifndef _ODebug_h_DEFINED
#define _ODebug_h_DEFINED

#include <stdio.h>

#ifdef OPENR_NO_DEBUG
#undef OPENR_DEBUG
#endif

#define PRINTF(x)       (printf x)

#ifdef OPENR_DEBUG
#define ODEBUG(x)       (x)
#define DPRINTF(x)      (printf x)
#define DPRINTF2(x,y)   { if (x) (printf y); }
#else
#define ODEBUG(x)
#define DPRINTF(x)
#define DPRINTF2(x,y)
#endif /* OPENR_DEBUG */

#endif /* _ODebug_h_DEFINED */
