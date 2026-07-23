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
 * $Date: Mon Feb 25 16:35:30 JST 2002 $
 * $Revision: /main/er/AIBO9/2 $
 */

#ifndef _AnalyzerAPI_h_DEFINED
#define _AnalyzerAPI_h_DEFINED

#include <Types.h>
#include <AnalyzerTypes.h>
#include <AnalyzerError.h>
#include <OID.h>

#include <ExecSpaceInfo.h>
#include <ThreadInfo.h>
#include <ContextInfo.h>
#include <SegmentInfo.h>
#include <EventInfo.h>

class MAV_Dispatcher;
class MAV_MessageQueue;
class MAV_Mutex;
class MAV_ShareLock;
class MAV_CondVar;

#ifdef  __cplusplus
extern "C" {
#endif  /* __cplusplus */
    azrError  AnalyzerGetExecSpaces (longword numOfBuf, ExecSpaceID* buffer, longword* numOfExecSpaces);
    azrError  AnalyzerGetThreadsByExecSpace (ExecSpaceID execSpace, longword numOfBuf, ThreadID* buffer, longword* numOfThreads);
    azrError  AnalyzerGetExecSpaceByThread (ThreadID thread, ExecSpaceID* execSpace);
    azrError  AnalyzerGetContextByThread (ThreadID thread, CName* context);
    azrError  AnalyzerGetThreadByContext (CName context, ThreadID* thread);
    azrError  AnalyzerGetContentsOfExecSpace (ExecSpaceID execSpace, ExecSpaceInfo* execSpaceInfo);
    azrError  AnalyzerGetContentsOfThread (ThreadID thread, ThreadInfo* threadInfo);
    azrError  AnalyzerGetContentsOfContext (CName cName, ContextInfo* contextInfo);
    azrError  AnalyzerGetLengthOfMessageQueue (ExecSpaceID execSpace, int* length);
    azrError  AnalyzerGetSizeOfUsedStack (CName cName, size_t* size);
    azrError  AnalyzerGetSizeOfFreeMemory (size_t* sizeOfFreeMemory);
    azrError  AnalyzerGetNumOfTimeEvents (longword* length, longword* max);
    azrError  AnalyzerGetSegmentInfo (ExecSpaceID execSpace, longword numOfBuf, SegmentInfo* buffer, longword* numOfSegments);
    azrError  AnalyzerGetPAddrFromVAddr (void* pAddr, void** vAddr);
    // Analysis mAV APIs
    azrError AnalyzerGetDispatchers(longword num, MAV_Dispatcher **dispBuf, longword* numOfDispatchers);
    azrError AnalyzerGetLengthOfDispatcherMsgQueue(MAV_Dispatcher *dispatcher, longword *length);
    azrError AnalyzerGetLengthOfDispatcherReqQueue(MAV_Dispatcher *dispatcher, longword *length);
    azrError AnalyzerGetMAVMessageQueues(longword num, MAV_MessageQueue **msgqBuf, longword *numOfMsgQueues);
    azrError AnalyzerGetLengthOfMAVMessageQueue(MAV_MessageQueue *msgq, longword *length);
    azrError AnalyzerGetMutexes(longword num, MAV_Mutex **mutexBuf, longword *numOfMutexes);
    azrError AnalyzerGetKeeperOfMutex(MAV_Mutex *mutex, ThreadID *thread);
    azrError AnalyzerGetWaitersOfMutex(MAV_Mutex *mutex, longword num, ThreadID *threadBuf, longword *numOfWaiters);
    azrError AnalyzerGetShareLocks(longword num, MAV_ShareLock **lockBuf, longword *numOfShareLocks);
    azrError AnalyzerGetKeeperOfShareLockForWrite(MAV_ShareLock *lock, ThreadID *thread);
    azrError AnalyzerGetKeepersOfShareLockForRead(MAV_ShareLock *lock, longword num, ThreadID *threadBuf, longword *numOfKeepers);
    azrError AnalyzerGetWaitersOfShareLock(MAV_ShareLock *lock, longword num, ThreadID *threadBuf, longword *numOfWaiters);
    azrError AnalyzerGetEventInfo(ThreadID thread, EventInfo *eventInfo);
    azrError AnalyzerGetCondVars(longword num, MAV_CondVar **condVarBuf, longword *numOfCondVars);
    azrError AnalyzerGetWaitersOfCondVar(MAV_CondVar *condVar, longword num, ThreadID *threadBuf, longword *numOfWaiters);
#ifdef  __cplusplus
}
#endif  /* __cplusplus */

#endif  /* _AnalyzerAPI_h_DEFINED */
