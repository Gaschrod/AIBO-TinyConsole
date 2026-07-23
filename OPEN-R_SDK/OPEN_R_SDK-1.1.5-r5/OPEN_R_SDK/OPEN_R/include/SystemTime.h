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
 * $Date: Mon Feb 25 16:33:33 JST 2002 $
 * $Revision: /main/tokyo/AIBO9/1 $
 */


#ifndef	_SystemTime_h_DEFINED
#define	_SystemTime_h_DEFINED

#include <Types.h> 

struct structSystemTime {
	longword	seconds;
	longword	useconds;
};

#ifdef	__cplusplus
struct SystemTime : structSystemTime{
#else	/* __cplusplus */
typedef struct {
	longword	seconds;
	longword	useconds;
#endif	/* __cplusplus */

#ifdef	__cplusplus
	SystemTime () {
		seconds = 0;
		useconds = 0;
	};
	SystemTime (longword sec, longword usec) {
        	seconds = sec;
		useconds = usec;
	};

	inline Boolean	IsValid () {
		return !((seconds == 0xffffffff) && (useconds == 0xffffffff));};

	void	MakeInvalid () {
		seconds = 0xffffffff;
		useconds = 0xffffffff;
	};
#endif	/* __cplusplus */
#ifdef	__cplusplus
};
#else	/* __cplusplus */
} SystemTime;
#endif	/* __cplusplus */

#ifdef	__cplusplus
inline int
operator< (const SystemTime& time1, const SystemTime& time2)
{
	return (time1.seconds < time2.seconds) || ((time1.seconds == time2.seconds) && (time1.useconds < time2.useconds));
}

inline SystemTime
operator+ (const SystemTime& time1, const SystemTime& time2)
{
	SystemTime	time;

	time.seconds = time1.seconds + time2.seconds;
	if ((time.useconds = time1.useconds + time2.useconds) >= 1000000) {
		time.seconds++;
		time.useconds -= 1000000;
	}
	return time;
}

inline SystemTime
operator- (const SystemTime& time1, const SystemTime& time2)
{
	SystemTime	time;

	// assume time1 > time2

	time.seconds = time1.seconds - time2.seconds;
	if (time1.useconds < time2.useconds) {
		time.useconds = 1000000 + time1.useconds - time2.useconds;
		time.seconds--;
	} else {
		time.useconds = time1.useconds - time2.useconds;
	}
	return time;
}
#endif	/* __cplusplus */

#endif	/* _SystemTime_h_DEFINED */
