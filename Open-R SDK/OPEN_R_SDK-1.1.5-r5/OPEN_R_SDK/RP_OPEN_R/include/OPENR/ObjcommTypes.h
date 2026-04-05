//
// Copyright (C) 2002 Sony Corporation
// All Rights Reserved.
//

#ifndef ObjcommTypes_h_DEFINED
#define ObjcommTypes_h_DEFINED

#if defined(__GNUC__)
#include <iostream>
using std::cout;
using std::endl;
using std::ostream;
#else
#include <iostream.h>
#endif
#include <string.h>
#include <list>
using std::list;
#include <OPENR/OPENR.h>

// ID
typedef OServiceEntry         SubjectID;
typedef OServiceEntry         ObserverID;

// OServiceEntry to ID
inline SubjectID  ENTRY2SUBJECTID (const OServiceEntry& e) { return e; }
inline ObserverID ENTRY2OBSERVERID(const OServiceEntry& e) { return e; }

//  For DEBUG
ostream& operator << (ostream& os, const ObserverID& observer);
ostream& operator << (ostream& os, const SubjectID& id);

/*  Message type definition in ObjectComm  */
typedef longword ObjcommMessageType;
const ObjcommMessageType  objcommMSG_UNDEF   = 0;
const ObjcommMessageType  objcommMSG_NOTIFY  = 1;
const ObjcommMessageType  objcommMSG_CONTROL = 2;
const ObjcommMessageType  objcommMSG_READY   = 3;

/*  Type of data transmition by shared memory or message  */
typedef longword ObjcommDataType;
const ObjcommDataType     objcommDATA_TYPE_UNDEF = 0;
const ObjcommDataType     objcommDATA_TYPE_MSG   = 1;
const ObjcommDataType     objcommDATA_TYPE_SHM   = 2;

/*  Error code  */
typedef int ObjcommStatus;
const ObjcommStatus  objcommSUCCESS              = 0;
const ObjcommStatus  objcommNOTIFY_UNDEF         = 1;
const ObjcommStatus  objcommFRAMENUMBER_UNDEF    = 2;
const ObjcommStatus  objcommADDOBSERVER_FAIL     = 3;
const ObjcommStatus  objcommREMOVEOBSERVER_FAIL  = 4;
const ObjcommStatus  objcommCOMMAND_UNDEF        = 5;

/*  Command type  */
typedef int ObjcommCommand;
/*  AddObserver/RemoveObserverの指定  */
const ObjcommCommand objcommCMD_UNDEF               = 0;
const ObjcommCommand objcommCMD_ADDOBSERVER         = 1;
const ObjcommCommand objcommCMD_REMOVEOBSERVER      = 2;
/*  AssertReady/DeassertReadyの指定  */
const ObjcommCommand objcommCMD_ASSERT_READY        = 3;
const ObjcommCommand objcommCMD_DEASSERT_READY      = 4;
/*  Connection to myself(no need to reply)  */
const ObjcommCommand objcommCMD_ADDOBSERVER_SELF    = 5;
const ObjcommCommand objcommCMD_REMOVEOBSERVER_SELF = 6;
/*  Version 2.2のAddObserver/RemoveObserverの指定  */
const ObjcommCommand objcommCMD_ADDOBSERVER_V22     = 7;
const ObjcommCommand objcommCMD_REMOVEOBSERVER_V22  = 8;

/*   Version of libObjectComm  */
typedef unsigned int ObjcommVersion;
const ObjcommVersion objcommVERSION10 = 1;
const ObjcommVersion objcommVERSION20 = 2;
const ObjcommVersion objcommVERSION21 = 3;
const ObjcommVersion objcommVERSION22 = 4;
/*  UNDEF should be same as MemoryRegionID's UNDEF. */
const ObjcommVersion objcommVERSION_UNDEF = 0xffffffff;

#endif // _ObjcommTypes_h_DEFINED
