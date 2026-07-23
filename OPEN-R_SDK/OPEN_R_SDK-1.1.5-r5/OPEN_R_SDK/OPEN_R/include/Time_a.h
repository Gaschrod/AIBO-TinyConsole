/*
 * The Aperios Software Platform
 *
 * This program is subject to copyright protection in accordance with the
 * applicable law. It must not, except where allowed by law, by any means or
 * in any form be reproduced, distributed or lent. Moreover, no part of the
 * program may be used, viewed, printed, disassembled or otherwise interfered
 * with in any form, except where allowed by law, without the express written
 * consent of the copyright holder.
 *
 * Copyright 1988-2002 Sony Corporation
 * All Rights Reserved.
 *
 */
/*
 * $Date: Mon Feb 25 16:33:21 JST 2002 $
 * $Revision: /main/tokyo/AIBO9/1 $
 */

// This library provides a collection of functions to handle time 

#ifndef _Time_a_h_DEFINED
#define _Time_a_h_DEFINED

#include <Types.h>
#include <SystemTime.h>

#ifndef	__cplusplus
typedef
#endif
struct RelativeTime {
	longword	sec;
	longword	msec;

#ifdef	__cplusplus
	RelativeTime () : sec (0), msec (0) {};
	RelativeTime (longword sc) : sec (sc), msec (0) {};
	RelativeTime (longword sc, longword msc) : sec (sc), msec (msc) {};
#endif	/* __cplusplus */
}
#ifndef	__cplusplus
RelativeTime
#endif
;

#ifdef	__cplusplus
struct InternalTime {
  int day;	/* Days since 1900/1/1    [0-] */
  longword sec;	/* Seconds since 00:00:00 [0-86399] */
  int msec;	/* Milli-Seconds          [0-999] */
};

// operators on RelativeTime

inline int
operator== (const RelativeTime & time1, const RelativeTime & time2)
{
    return (time1.sec == time2.sec && time1.msec == time2.msec);
}


// conversion functions
RelativeTime
ConvertToRelativeTime(const SystemTime& time);

SystemTime
ConvertToSystemTime(const RelativeTime & time);
#endif	/* __cplusplus */

#endif	/* _Time_a_h_DEFINED */
