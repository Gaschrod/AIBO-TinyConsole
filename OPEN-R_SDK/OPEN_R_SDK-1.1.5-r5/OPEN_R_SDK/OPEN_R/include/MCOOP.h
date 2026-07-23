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
 * $Date: Mon Feb 25 16:33:26 JST 2002 $
 * $Revision: /main/tokyo/AIBO9/3 $
 */

#ifndef _MCOOP_h_DEFINED
#define _MCOOP_h_DEFINED

#include <Types.h>
#include <SystemError.h>
#include <SystemResource.h>
#include <OID.h>
#include <TID.h>
#include <SystemTime.h>
#include <TimeEventInfo.h>
#include <EventID.h>
#include "MCOOPError.h"
#include "MCOOPTypes.h"

#ifdef  __cplusplus
extern "C" {
#endif  /* __cplusplus */
    sError  Send (OID dest, Selector meth, void* msg, size_t sizeOfMsg);
    sError  SendWithRBox (OID dest, Selector meth, void* msg, size_t sizeOfMsg,
        RID* rBoxID);
#ifdef  __cplusplus
    sError  Receive (RID rBoxID, void* msg = 0, size_t sizeOfMsg = 0,
    size_t* sizeOfReceivedMsg = NULL, RelativeTime* timeout = NULL);
#else   /* __cplusplus */
    sError  Receive (RID rBoxID, void* msg, size_t sizeOfMsg,
    size_t* sizeOfReceivedMsg, RelativeTime* timeout);
#endif	/* __cplusplus */
#ifdef	__cplusplus
    sError  Call (OID dest, Selector meth, void* msg, size_t sizeOfMsg,
        void* replyMsg = 0, size_t sizeOfReplyMsg = 0);
#else	/* __cplusplus */
    sError  Call (OID dest, Selector meth, void* msg, size_t sizeOfMsg,
        void* replyMsg, size_t sizeOfReplyMsg);
#endif	/* __cplusplus */
#ifdef	__cplusplus
    sError  Reply (void* msg = 0, size_t sizeOfMsg = 0);
#else	/* __cplusplus */
    sError  Reply (void* msg, size_t sizeOfMsg);
#endif	/* __cplusplus */
    sError  Delegate (OID dest, Selector meth, void* msg, size_t sizeOfMsg);
    sError  CleanRBox (RID rBoxID);
#ifdef	__cplusplus
    void    Return (void* msg = 0, size_t sizeOfMsg = 0);
#else	/* __cplusplus */
    void    Return (void* msg, size_t sizeOfMsg);
#endif	/* __cplusplus */
    void    Exit ();
    void    ExitFromPrologue ();
    sError  GetSystemTime (SystemTime* sysTime);
    sError  SetReplyEvent (RID rBoxID, OID dest, Selector meth, void* msg, 
		size_t sizeOfMsg, EventID*  eventID);
    sError  SetTimeEvent (TimeEventInfo* info, OID dest, Selector meth,
		void* msg, size_t sizeOfMsg, EventID* eventID);
    sError  CancelEvent (EventID eventID);
    sError  SetResourceValue (sResource res, slongword value);
#ifdef	__cplusplus
    sError  GetResourceValue (sResource res, slongword* cur,
		slongword* max = NULL, slongword* min = NULL);
#else	/* __cplusplus */
    sError  GetResourceValue (sResource res, slongword* cur,
		slongword* max, slongword* min);
#endif	/* __cplusplus */
    sError  NewRegion (size_t size, void** pBaseAddress);
    sError  DeleteRegion (void* pBaseAddress);
    sError  New (OID cls, OID* instance);
    sError  Delete ();
    sError  Load (char* moduleName, OID* instance);
    sError  LoadAndExecMAV (const char* pathName, longword arg1, longword arg2, longword priority, size_t stack, size_t heap);
    sError  Unload (OID instance);
    sError  WhoAmI (OID* object);
    sError  Find (char*, OID* object);
	sError	FindSelector (char* name, Selector* meth);
    sError  Register (OID object, char* name);
    sError  Unregister (char* name);
    sError  SetEntryTable (size_t numberOfEntries, EntryTable*);
    sError  NewSharedMemoryRegion (size_t size, MemoryProtectionInfo info,
		MemoryRegionID* memRegionID, void** baseAddr);
    sError  DeleteSharedMemoryRegion (MemoryRegionID memRegionID);
    sError  AllowToAttachMemoryRegion (MemoryRegionID memRegionID,
		MemoryProtectionInfo info, OID destObjID,
 		bool givingOwnership);
    sError  AttachMemoryRegion (MemoryRegionID memRegionID, 
		MemoryProtectionInfo info,  void** baseAddr);
    sError  DetachMemoryRegion (MemoryRegionID memRegionID);
    sError  ChangeProtectionMemoryRegion (MemoryRegionID memRegionID,
		MemoryProtectionInfo info);
    sError  GrowMemoryRegion (MemoryRegionID memRegionID, size_t size);
    sError  ShrinkMemoryRegion (MemoryRegionID memRegionID,
		size_t size);
    sError  GetPageSize (size_t* size);
    sError  GetFreeMemorySize (size_t* size);

    sError  Wait (longword nanosec);
#ifdef  __cplusplus
}
#endif  /* __cplusplus */

#ifdef  __cplusplus
/* as aliases */
inline sError
Send (OID dest, Selector meth, void* msg, size_t sizeOfMsg, RID* rBoxID)
{
    return SendWithRBox (dest, meth, msg, sizeOfMsg, rBoxID);
}

inline sError
Find (char* name, Selector* meth)
{
	return FindSelector (name, meth);
}

/* for backward compatibility */
inline sError
Receive (RID rBoxID, void* msg, size_t sizeOfMsg, RelativeTime* timeout)
{
    return Receive (rBoxID, msg, sizeOfMsg, NULL, timeout);
}
#endif  /* __cplusplus */

#endif  /* _MCOOP_h_DEFINED */
