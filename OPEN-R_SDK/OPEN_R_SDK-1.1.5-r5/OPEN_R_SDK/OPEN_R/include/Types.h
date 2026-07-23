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
 * $Date: Mon Feb 25 16:33:08 JST 2002 $
 * $Revision: /main/tokyo/AIBO9/3 $
 */

#ifndef _Types_h_DEFINED
#define _Types_h_DEFINED

typedef unsigned char   byte;       /* must be 8-bit quantity */
typedef unsigned short  word;       /* must be 16-bit quantity */
typedef unsigned long   longword;   /* must be 32-bit quantity */
typedef char            sbyte;      /* must be signed 8-bit quantity */
typedef short           sword;      /* must be signed 16-bit quantity */
typedef long            slongword;  /* must be signed 32-bit quantity */
typedef byte*           pbyte;      /* must be 8-bit addressing unit */
typedef word*           pword;      /* must be 16-bit addressing unit */
typedef int*            plongword;  /* must be 32-bit addressing unit */

typedef unsigned long   magicword;
typedef unsigned char   u_char;
typedef unsigned short  u_short;
typedef unsigned int    u_int;
typedef unsigned long   u_long;

typedef char			int8;
typedef unsigned char		uint8;
typedef short			int16;
typedef unsigned short		uint16;
typedef long			int32;
typedef unsigned long		uint32;
typedef long			int64;	/* XXX */
typedef unsigned long		uint64;	/* XXX */

#ifndef _SIZE_T
#define _SIZE_T
typedef unsigned int    size_t;
#endif

#if defined(_BOOL) || defined(__GNUC__)
#define __BOOL_DEFINED
#endif

#ifndef __BOOL_DEFINED

#define bool    int
#define true    1
#define false   0
#define __BOOL_DEFINED

#endif

#ifndef NULL
#define NULL    0
#endif

#ifndef TRUE
#define TRUE    (! NULL)
#define FALSE   NULL
#endif

#if 0
typedef unsigned short  Boolean;
#else
typedef bool            Boolean;
#endif

typedef slongword sError;
typedef longword Selector;

typedef byte* VirtualAddress;
typedef void* PhysicalAddress;

/*
 * Temporary we defined UNDEF here.
 */
/*
 * ``UNDEF'' defines an entry meaning ``undefined.''
 */
#ifdef __cplusplus
const Selector	UNDEF	= 0xffffffff;
#else /* __cplusplus */
#define	UNDEF	0xffffffff
#endif /* __cplusplus */

/*
 * used to get a block which is aligned with the max size type.
 */
typedef long double AlignedBlock;
#define AlignedSize(size) (((size)+(sizeof(AlignedBlock)-1))/sizeof(AlignedBlock))

/*
 * Type definition of the entry point of an object
 */
typedef void    (*Entry) (...);

/*
 * NOTE: Following things could be defined in the other location?
 */
#ifndef	__cplusplus
typedef
#endif
struct  Context
#ifndef	__cplusplus
Context
#endif
;
typedef Context*        CName;

#ifndef	__cplusplus
typedef
#endif
struct  EntryTable
#ifndef	__cplusplus
EntryTable
#endif
;
typedef EntryTable*     EntryTableID;

#ifndef	__cplusplus
typedef
#endif
struct  Thread
#ifndef	__cplusplus
Thread
#endif
;
typedef Thread*         ThreadID;

#ifndef	__cplusplus
typedef
#endif
struct  Object
#ifndef	__cplusplus
Object
#endif
;
typedef Object*         ObjectPtr;

#define EXECSPACEID_UNDEF 0
#define EXECSPACEID_ALL   1
 
#ifndef __cplusplus
typedef
#endif
struct  ExecSpace
#ifndef __cplusplus
ExecSpace
#endif
;
typedef ExecSpace*      ExecSpaceID;
#define EXECSPACEID_ISVALID(x) ( (x) != EXECSPACEID_UNDEF )
#define EXECSPACEID_MAKEINVALID(x) ( (x) = EXECSPACEID_UNDEF )
/* NOTE: Instead, ExecSpace.h should be coped with both C and C++. */

/* types for shared memory */
typedef unsigned int MemoryRegionID;
typedef MemoryRegionID SharedMemoryRegionID;
typedef enum {
        MEMPROTINFO_INVALID = 0,
        MEMPROTINFO_R  = 1,
        MEMPROTINFO_RW = 2
} MemoryProtectionInfo;
typedef MemoryProtectionInfo SharedMemoryProtectionInfo;

#endif  /* _Types_h_DEFINED */
