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
 * $Date: Mon Feb 25 16:33:44 JST 2002 $
 * $Revision: /main/tokyo/AIBO9/2 $
 */

#ifndef _MAVTypes_h_DEFINED_
#define _MAVTypes_h_DEFINED_

#include <Types.h>
#include <TID.h>

/*
 * basic types
 */

typedef uint8	Bool;		/* Boolean, TRUE or FALSE */
#define MAV_TRUE	1
#define MAV_FALSE	0

typedef int32	    Status;	/* OK or ERROR */
#define OK	    0		/* Completely success */
#define ERROR	    (-1)	/* failure; caller should check errno */
#define NOSUPPORT   (-2)	/* this facility is not suporrted */

/* General */
#define TIMEOUT		101	/* timeout happen */
#define TIMEOUT2	102	/* 2nd timeout happen */

/* MemorySpace, ExecutionSpace, Thread and Module */
#define EBADSCHEDCLASS	(-1001)	/* specified SchedClass is wrong */
#define ENOTFOUNDMODULE	(-1002)	/* specified Module is not found */
#define EBADSCHEDPRIORITY (-1003) /* specified SchedPriority is wrong */
#define EFAILINSTANTIATION	(-1004)	/* instantiating is failed */
#define ENOEXECUTIONSPACE	(-1005)	/* there are no ExecutionSpace */
#define ENOMEMORYSPACE	(-1006)	/* there are no MemorySpace */
#define ENOTHREAD	(-1007)	/* there are no Thread */
#define EBADEXECUTIONSPACE	(-1008)	/* specified ExecutionSpace is wrong */
#define EBADMEMORYSPACE	(-1009)	/* specified MemorySpace is wrong */
#define EBADTHREAD	(-1010)	/* specified Thread is wrong */
#define EDISPTHREAD	(-1011)	/* specified Thread is Dispatcher's Thread */

/* Dispatcher and MessageQueue */
#define EBADDISPATCHER	(-2001)	/* specified Dispatcher is wrong */
#define EBADREPLYID	(-2002)	/* specified ReplyID is wrong */
#define EBADSELECTOR	(-2003)	/* specified Selector is wrong */
#define EBADNUMBER	(-2004)	/* specified maxThreads is wrong */
#define EBADMSGQUEUE	(-2005)	/* specified MessageQueue is wrong */
#define ECREATEMSGQUEUE	(-2006)	/* creating new MessageQueue is fail */
#define ENOEVENT	(-2007)	/* there are no event to be set */
#define EMSGSIZELARGE	(-2008)	/* specified message size is exceeded */
#define ENOTENOUGHBUFSIZE	(-2009)	/* specified buffer size is not enough */
#define ENOTREPLIER	(-2010)	/* the caller Thread is not the replyer */
#define EALREADYDONE	(-2011)	/* reply or postpone is already done for specified id */
#define EBADRECEIVEORDER	(-2012)	/* specified receive order is wrong */
#define EBADSENDORDER	(-2013)	/* specified send order is wrong */
#define EFULLQUEUE	(-2014) /* the queue of specified dispatcher is overflown */
#define EBADDISPRSC	(-2015) /* specified resource type is wrong */
#define EBADVALUE	(-2016) /* specified value exceeds the alterable range */
#define ENOREPLYID	(-2017) /* upper limit of the reply id is exceeded */
#define EFAILINTERMETASPACECOM	(-2030)	/* inter metaspace communication is failed */

/* Mutex, ShareLock and CondVar */
#define EBADMUTEX	(-3001)	/* specified Mutex is wrong */
#define EBADCONDVAR	(-3002)	/* specified CondVar is wrong */
#define EBADSHARELOCK	(-3003)	/* specified ShareLock is wrong */
#define EBADPOLICY	(-3004)	/* specified policy is wrong */
#define EBADOWNER	(-3005)	/* specified owner option is wrong */
#define ENOCONDVAR	(-3006) /* there are no CondVar */
#define ENOMUTEX	(-3007)	/* there are no Mutex */
#define ENOSHARELOCK	(-3008)	/* there are no ShareLock */
#define ENOTLOCKED	(-3009)	/* the caller Thread does not have the Lock */
#define EDESTROYED	(-3010)	/* the target object is destroyed */
#define EMANYLOCKED	(-3011) /* the caller Thread has locked the mutex more than once */
#define EALREADYLOCKED	(-3012)	/* the caller thread has already locked the sharelock */

/* Registry */
#define EBADNAME	(-4001)	/* specified Name is wrong */
#define EBADPREFIX	(-4002)	/* specified prefix is wrong */
#define EBADKEY		(-4003)	/* specified key is wrong */
#define EBADLENGTH	(-4004)	/* specified valueLen is wrong */
#define EBADTYPE    	(-4005)	/* specified type is wrong */
#define ENOTREGISTERED	(-4007)	/* specified entry name is not registered */
#define ERANGEOVER	(-4008)	/* list counter is out of range */
#define EALREADYREGISTERED (-4009) /* element is already registered */
#define EEXCEEDMAXENTRIES  (-4010) /* exceed the upper limit of element */

/* Event, Clock and Timer */
#define EBADTIMER	(-5001)	/* specified Timer is wrong */
#define ERESERVED	(-5002)	/* specified evet is already reserved */
#define ENOTIMERENTRY	(-5003)	/* there are no entry to set timeout */
#define ENOTRESERVED	(-5004)	/* specified evet is not reserved */
#define ESYSEVENT	(-5005)	/* specified evet is reserved by system */
#define ENOTARMED	(-5006)	/* specified Timer is not set Aram */
#define ENOTIMER	(-5007)	/* there are no Timer */

/* Region */
#define EBADREGION	(-6001)	/* specified Region is wrong */
#define EBADPROTMODE	(-6002)	/* specified protection mode is wrong */
#define EBADSIZE	(-6003)	/* specified size is wrong */
#define EBADMAPPING	(-6004)	/* specified map is wrong */
#define EALREADYMAPPED	(-6005)	/* specified Region is already mapped */
#define ENOTALLOWED	(-6006)	/* this API does not allow to caller */
#define ENOTOWNER	(-6007)	/* the caller Thread is not owner */
#define ENOMEMORY   	(-6008)	/* there are no memory */
#define EBADMEMREGIONID	(-6009)	/* specified MemoryRegionID is wrong */

/* FaultHandler */
#define EBADFAULT	(-7001)	/* specified fault handler is wrong */

#ifdef MAV_USE_OBSOLETE_API
// For backward compatibility
#define ENOENT      2		/* entry not found */
#define ESMALLBUF   3		/* buffer is too small */
#define EBADARG     4		/* invalid argument */
#define ETOOBIG     5		/* size is too big */
#define ETIMEOUT    6		/* time out happens */
#define EILLEGAL    8		/* illegal operation */
#define EDESTROY    10		/* mutex or sharelock is destroyed */
#define ETOOSMALL	(-4006)	/* specified buffer is not enough size */
#endif /* MAV_USE_OBSOLETE_API */

#define WAIT_FOREVER	0xffffffff

typedef uint32	SystemTick;
typedef uint32	EventMask;
typedef uint32	MemOffset;
typedef BaseTID	ReplyID;

typedef void (*MAV_DispatcherEntry) (void *message, size_t size);
typedef void (*MAV_ThreadEntry) (void *arg1, void *arg2);
typedef void (*MAV_ModuleEntry) (void);

enum {
    /* options for schedule policy */
    Q_FIFO          = 0x01, /* queue waiters FIFO */
    Q_PRIORITY      = 0x02, /* queue waiters by priority */

    /* belong place */
    U_EXECSPACE     = 0x10, /* only used in exec space */
    U_GLOBAL        = 0x20  /* used every space */
};

#endif /* _MAVTypes_h_DEFINED_ */
