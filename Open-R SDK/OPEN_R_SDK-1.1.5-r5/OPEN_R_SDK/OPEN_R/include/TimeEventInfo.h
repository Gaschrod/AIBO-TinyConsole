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
 * $Date: Mon Feb 25 16:34:02 JST 2002 $
 * $Revision: /main/tokyo/AIBO9/1 $
 */

#ifndef _TimeEventInfo_h_DEFINED
#define _TimeEventInfo_h_DEFINED

#include <Types.h>
#include <Time_a.h>

#ifndef __cplusplus
/*
 * EventType of TimeEventInfo for Language C
 */
typedef enum {
	Periodic,
	NonPeriodic
} EventType;

/*
 * EventAttr of TimeEventInfo for Language C
 * These values have to be same as WatchDogInfo::ActivationType.
 */
typedef enum {
	ExceedStartImmediate		= 0x00000000,
	ExceedStartError		= 0x00010000
} EventAttr;
#endif  /* __cplusplus */


/*
 * TimeEventInfo
 *
 *	EventType	periodic or non-periodic event
 *	bool		start time is relative or calender time
 *	EventAttr	event attribute
 *	RelativeTime	periodic time used for periodic event
 */

#ifndef  __cplusplus
typedef
#endif
struct TimeEventInfo {
#ifdef  __cplusplus
        typedef enum {
                Periodic,
                NonPeriodic
        } EventType;

        /*
         * These values have to be same as WatchDogInfo::ActivationType.
         */
        typedef enum {
                ExceedStartImmediate            = 0x00000000,
                ExceedStartError                = 0x00010000
        } EventAttr;
#endif  /* --cplusplus */

	RelativeTime	relativeTime;
	RelativeTime	periodicTime;
}
#ifndef  __cplusplus
TimeEventInfo
#endif
;


#ifdef	__cplusplus

/*
 * TimeEventInfoWithRelativeTime
 */

struct TimeEventInfoWithRelativeTime : public TimeEventInfo {
	bool		isRelativeTime;
	EventType	eventType;
	longword	eventAttr;

	TimeEventInfoWithRelativeTime (RelativeTime, RelativeTime = 0,
		longword eventAttr = TimeEventInfo::ExceedStartImmediate);
	void	Set (RelativeTime, RelativeTime = 0,
		longword eventAttr = TimeEventInfo::ExceedStartImmediate);

	/* OLD method for backward compatibility */
	TimeEventInfoWithRelativeTime (EventType, RelativeTime);
	void    Set (EventType, RelativeTime);
};

inline
TimeEventInfoWithRelativeTime::TimeEventInfoWithRelativeTime (
	RelativeTime relTime,
	RelativeTime perTime,
	longword attr) : eventAttr (attr)
{
	relativeTime = relTime;
	if (perTime == 0) {
		eventType = TimeEventInfo::NonPeriodic;
	} else {
		eventType = TimeEventInfo::Periodic;
	}
	isRelativeTime = true;
	periodicTime = perTime;
}

inline void
TimeEventInfoWithRelativeTime::Set (
	RelativeTime relTime,
	RelativeTime perTime,
	longword attr)
{
	if (perTime == 0) {
		eventType = TimeEventInfo::NonPeriodic;
	} else {
		eventType = TimeEventInfo::Periodic;
	}
	isRelativeTime = true;
	relativeTime = relTime;
	periodicTime = perTime;
	eventAttr = attr;
}

/* OLD method for backward compatibility */
inline
TimeEventInfoWithRelativeTime::TimeEventInfoWithRelativeTime
	(EventType eType, RelativeTime relTime)
{
	isRelativeTime = true;
	eventType = eType;
	eventAttr = TimeEventInfo::ExceedStartImmediate;
	relativeTime = relTime;
	if (eventType == TimeEventInfo::NonPeriodic)
		periodicTime = 0;
	else
		periodicTime = relTime;
}

inline void
TimeEventInfoWithRelativeTime::Set (EventType eType, RelativeTime relTime)
{
	isRelativeTime = true;
	eventType = eType;
	eventAttr = TimeEventInfo::ExceedStartImmediate;
	relativeTime = relTime;
	if (eventType == TimeEventInfo::NonPeriodic)
		periodicTime = 0;
	else
		periodicTime = relTime;
}

#else	/* __cplusplus */

struct TimeEventInfoWithRelativeTime {
	struct	TimeEventInfo	timeEventInfo;
	bool			isRelativeTime;
	EventType		eventType;
	longword		eventAttr;
};

#endif	/* __cplusplus */

#endif  /* _TimeEventInfo_h_DEFINED */
