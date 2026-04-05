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
 * $Date: Mon Feb 25 16:34:32 JST 2002 $
 * $Revision: /main/tokyo/AIBO9/2 $
 */

#ifndef FixedPtr_h_DEFINED
#define FixedPtr_h_DEFINED

/* In case of R4000 architecture, following Address ranges are reserved for
 * Fixed-Addressed-Structure.
 *
 * Area A: VCPU reserved.
 * 0x80000000-0x80001FFF: Assigned to VCPU and LowLevelModule (4Kbytes)
 *         This area is used for CPU exception vector.
 *         This area is NOT maped to 0x00000000.
 *         0x00000000 (NULL) causes address error exception.
 *         This exception makes debug easier.
 * 0x80000000-0x8000007F: reserved for TLB exception handler. (by VCPU)
 * 0x80000080-0x800000FF: reserved for TLB64 exception handler. (by VCPU)
 * 0x80000100-0x8000017F: reserved for future use. (by VCPU)
 * 0x80000180-0x80001FFF: reserved for General exception handler. (by VCPU)
 *
 *
 * Area B:
 * 0x80002000-0x80002FFF: Assigned to Fixed-Addressed-Structure.(4Kbytes)
 *                        This page is also maped to 0x00002000(read/write)
 *                        if CPU has TLB.
 *                        This area is used as 'debug purpose system variables'.
 *                        Take care! User mode objects can modify this page.
 *                        The purpose of this page is mainly debugging and
 *                        profiling.
 *                        DO NOT use this page as  inter-meta-object
 *                        shared memory.
 *
 *
 * Area C:
 * 0x80003000-0x80007FFF: Assigned to Fixed-Addressed-Structure.(16Kbytes)
 *                        This page is also maped to 0x00004000(read only)
 *                        if CPU has TLB.
 *                        'global constant values'.
 *                        'protected system global variables'.
 *                        'subroutines provided by VCPU'
 *
 * 0x80008000-0x8003FFFF: This area may be used by ABK.
 *                        So you cannot put modules to this area.
 *                        Aperios system doesn't use this area, too.
 *                        But libEval.a for evaluation uses top of this
 *                        area to clear cache safely. This means that
 *                        Aperios system doesn't use this area, but touches
 *                        this area.
 *                         
 *
 * Free Area:
 * 0x80040000-          : Assigned to ordinary aperios memory space.
 */


/********************************
 * Page base address Definition *
 ********************************/

#define FIXED_REGION_BASE    0x00003000
#define USER_DIRTY_BASE      0x00002000
#define USER_DIRTY_SIZE      0x00001000
#define KERNEL_GLOBAL_PAGE   (0x80000000 + FIXED_REGION_BASE)
#define CACHE_OPERATION_PAGE 0x80010000

#ifdef    NO_USE_ZERO_PAGE

#define USER_READ_PAGE       KERNEL_GLOBAL_PAGE
#define USER_DIRTY_PAGE      (0x80000000 + USER_DIRTY_BASE)

#else  /* NO_USE_ZERO_PAGE */

#define USER_READ_PAGE       FIXED_REGION_BASE
#define USER_DIRTY_PAGE      USER_DIRTY_BASE

#endif /* NO_USE_ZERO_PAGE */


/* virtual address for normal use starts from here */
#define USER_NORMAL_PAGE     0x00008000




/*****************************
 * Accessor Macro Definition *
 *****************************/

#define GetKernelGlobalPtr(name) (FIXEDPTROFS_##name + KERNEL_GLOBAL_PAGE)
#define GetGlobalReadPtr(name)   (FIXEDPTROFS_##name + USER_READ_PAGE)
#define GetDirtyPagePtr(name)    (DIRTYPTROFS_##name + USER_DIRTY_PAGE)


/*******************************
 * Dirty page Ptr offset Definition *
 *******************************/
#define DIRTYPTROFS_USER_DIRTY_PAGE_BOTTOM      USER_DIRTY_SIZE
#define DIRTYPTROFS_VCPU_STACK_BOTTOM (DIRTYPTROFS_USER_DIRTY_PAGE_BOTTOM-0x00000020)


/*******************************
 * Fixed Ptr offset Definition *
 *******************************/

/* !! Take care of cache boundary, if you move address of entries. !!    */
/* MIPS processor's Cache line size of data cache is 16bytes or 32bytes. */

/* 0x00000000 - 0x00000fff : range 0 (General purpose : 4Kbytes)     */
/* 0x00001000 - 0x00004fff : range 1 (for VCPU        : 16Kbytes)    */

#define FIXEDPTROFS_RANGE0	0x0000
#define FIXEDPTROFS_RANGE1	0x1000

/* range 0 definition   */
/* size 4Kbytes         */
/* General purpose      */

/* 0x00000000 - 0x000003ff : range A architecture independent part */
/* 0x00000400 - 0x000007ff : range B CPU family dependent part     */
/* 0x00000800 - 0x00000bff : range C exact CPU dependent part      */
/* 0x00000c00 - 0x00000dff : range D platform dependent part       */
/* 0x00000e00 - 0x00000fff : renge E exact platform dependent part */
/* range A & B is defined in this file
 * range C is defined in PPLSys_FixedPtrExactCPU.h
 * range D is defined in PPLSys_FixedPtrPlatform.h
 * range E is defined in PPLSys_FixedPtrExactPlatform.h
 * range C-E is not refered by {common/CPU family depend} code.
 * range D and E is not refered by
 *       {common/CPU family depend/exact CPU depend} code.
 * range E is only refered by
 *       exact platform depend code.
 */

#define FIXEDPTROFS_RANGEA (FIXEDPTROFS_RANGE0+0x00000000)
#define FIXEDPTROFS_RANGEB (FIXEDPTROFS_RANGE0+0x00000400)
#define FIXEDPTROFS_RANGEC (FIXEDPTROFS_RANGE0+0x00000800)
#define FIXEDPTROFS_RANGED (FIXEDPTROFS_RANGE0+0x00000c00)
#define FIXEDPTROFS_RANGEE (FIXEDPTROFS_RANGE0+0x00000e00)

/* */

/* global counter for ContextID (4bytes) */
#define FIXEDPTROFS_CONTEXT_COUNTER (FIXEDPTROFS_RANGE0+0x000000c0)

#define FIXEDPTROFS_DEBUGGEROBJECT_DBGCALLPOOLNUM (FIXEDPTROFS_RANGE0+0x000000b8)
#define FIXEDPTROFS_DEBUGGEROBJECT_DBGCALLPOOLSIZE (FIXEDPTROFS_RANGE0+0x000000b4)
#define FIXEDPTROFS_DEBUGGEROBJECT_DBGCALLPOOLBASE (FIXEDPTROFS_RANGE0+0x000000b0)

/* Context Switch Logging control variables */
/* size 16byte                              */
/* alignment 16byte                         */
#define FIXEDPTROFS_CONTEXTSWITCHHANLDERINFO (FIXEDPTROFS_RANGE0+0x000000a0)

/* DebuggerObject Installed Flag */
#define FIXEDPTROFS_DEBUGGEROBJECT_INSTALLED (FIXEDPTROFS_RANGE0+0x00000094)

/* addresses where Memory managiment well-known structure are stored */
#define FIXEDPTROFS_MDRIVE3DESC	(FIXEDPTROFS_RANGE0+0x00000090)
#define FIXEDPTROFS_MDRIVE2DESC	(FIXEDPTROFS_RANGE0+0x0000008c)
#define FIXEDPTROFS_MDRIVEDESC	(FIXEDPTROFS_RANGE0+0x00000088)
#define FIXEDPTROFS_INTDRIVERTHREAD	(FIXEDPTROFS_RANGE0+0x00000084)
#define FIXEDPTROFS_INTMASK       (FIXEDPTROFS_RANGE0+0x00000080)
#define FIXEDPTROFS_INTLEVEL      (FIXEDPTROFS_RANGE0+0x0000007c)
#define FIXEDPTROFS_ASIDMAP       (FIXEDPTROFS_RANGE0+0x00000078)
#define	FIXEDPTROFS_SYSTEMGLOBAL  (FIXEDPTROFS_RANGE0+0x00000074)

/* addresses where objectTable attributes are stored. */
#define FIXEDPTROFS_OBJTBL_ADDR   (FIXEDPTROFS_RANGE0+0x00000070)
#define FIXEDPTROFS_OBJTBL_MAX	  (FIXEDPTROFS_RANGE0+0x0000006c)
#define FIXEDPTROFS_OBJTBL_MASK   (FIXEDPTROFS_RANGE0+0x00000068)

/* Timer related structure. */
/* TickTimer Lower 32bit(when Upper 32 is updated) */
#define FIXEDPTROFS_TICKLOWER     (FIXEDPTROFS_RANGE0+0x00000064)
/* TickTimer Upper 32bit */
#define FIXEDPTROFS_TICKUPPER     (FIXEDPTROFS_RANGE0+0x00000060)
#define FIXEDPTROFS_TICKRATIO     (FIXEDPTROFS_RANGE0+0x0000005C)

/* Panic Handler */
#define FIXEDPTROFS_PANICHANDLER  (FIXEDPTROFS_RANGE0+0x00000058)

/* Exception Dumper */
#define FIXEDPTROFS_EXCEPTION_DUMPER (FIXEDPTROFS_RANGE0+0x00000054)

/* TickTimer Latency 16bytes */
#define FIXEDPTROFS_TIMER_EVALLATENCY     (FIXEDPTROFS_RANGE0+0x00000040)


/* range 1 definition           */
/* size 8Kbytes                 */
/* VCPU variables and constants */

#define FIXEDPTROFS_VCPU_REGS (FIXEDPTROFS_RANGE1+0x0000)
/* This Table reserves 0x100 bytes   */
/* Max number of VCPU register is 64 */

#define FIXEDPTROFS_VCPU_GENERAL_INST_TABLE (FIXEDPTROFS_RANGE1+0x0100)
/* This Table reserves 0x80 bytes               */
/* max size of general instruction table = 0x80 */
/* This means that max entry number of VCPUGeneralInstTable is 32 */

#define FIXEDPTROFS_VCPU_PRIVILEGED_INST_TABLE (FIXEDPTROFS_RANGE1+0x0180)
/* This Table reserves 0x80 bytes                  */
/* max size of privileged instruction table = 0x80 */
/* This means that max entry number of VCPUPrivilegedInstTable is 32 */

#define FIXEDPTROFS_VCPU_INTERRUPT_PENDING_QUEUE (FIXEDPTROFS_RANGE1+0x0200)
/* This Table reserves 0x100 bytes                     */
/* max size of privileged instruction table = 0x100    */
/* This means that max number of interrupt level is 64 */

#define FIXEDPTROFS_VCPU_INTERRUPT_INFO_TABLE (FIXEDPTROFS_RANGE1+0x0300)
/* This Table reserves 0x1000 bytes                  */
/* max size of privileged instruction table = 0x1000 */
/* This means that max number of interrupt is 256 */

#define FIXEDPTROFS_VCPUADDRESSPROBE (FIXEDPTROFS_RANGE1+0x1300)
#define FIXEDPTRSIZE_VCPUADDRESSPROBE 0x100
/* VCPUPrivileged instruction IsValidAddress() code is put here.... */
/* reserve 0x00001300-0x00001400, 0x100bytes( 64 instruction ) */


/********************************
 * Reserved for LowLevelModules *
 * See LowLevelModulePtr.h      *
 ********************************/


/*******************************
 * Dirty Ptr offset Definition *
 *******************************/

/*
 *	DitryPtr part is used mainly debugging and profiling.
 *
 *              0x00000000 - 0x0000000f ANALYSIS_HEAPANALYZER
 */

#define DIRTYPTROFS_ANALYSIS_HEAPANALYZER       0x0000


/**************************
 * Backward compatibility *
 **************************/

/* Following part is definition for backward compatibility 
 * This definition should be removed and Each module should use
 * GetKernelGlobalPtr() directly.
 */

/* addresses where well-known structure are stored */
#define FIXEDPTR_ASIDMAP      GetKernelGlobalPtr(ASIDMAP)
#define	FIXEDPTR_SYSTEMGLOBAL GetKernelGlobalPtr(SYSTEMGLOBAL)

/* addresses where objectTable attributes are stored. */
#define OBJTBL_ADDR     GetKernelGlobalPtr(OBJTBL_ADDR)
#define OBJTBL_MAX	GetKernelGlobalPtr(OBJTBL_MAX)
#define OBJTBL_MASK     GetKernelGlobalPtr(OBJTBL_MASK)

/* addresses where valiables of BIOS are stored. */

#endif /* FixedPtr_h_DEFINED */

