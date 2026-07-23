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
 * REVISION
 * $Date: Mon Feb 25 16:33:12 JST 2002 $
 * $Revision: /main/tokyo/AIBO9/1 $
 */

#ifndef	CPU_h_DEFINED
#define	CPU_h_DEFINED

#define InvocationWorkSize 16 /* reserved space for arguments (MAX 4) */

#ifdef __cplusplus
#define BEGIN_EXTERN_C extern "C" {
#define END_EXTERN_C }
#else /* __cplusplus */
#define BEGIN_EXTERN_C
#define END_EXTERN_C
#endif /* __cplusplus */

/*
 * The CPU dependent structure (for MIPS R seriese)
 */

#define mSYSTEM 0
#define mUSER   1

#define SYSTEM_RESOURCE_ALIGNMENT_SIZE  32

/*
 * Index to structure CPURegisters
 */
#define CPU_GENERALREG_PC   0 /* esp reg cp0-$14 */
#define CPU_GENERALREG_SP   1 /* sp  reg $29     */
#define CPU_GENERALREG_TB   2 /* text base reg (s7) $23 */
#define CPU_GENERALREG_DB   3 /* data base reg (gp) $28 */
#define CPU_GENERALREG_S0   4 /* s0  $16 */
#define CPU_GENERALREG_S1   5 /* s1  $17 */
#define CPU_GENERALREG_S2   6 /* s2  $18 */
#define CPU_GENERALREG_S3   7 /* s3  $19 */
#define CPU_GENERALREG_S4   8 /* s4  $20 */
#define CPU_GENERALREG_S5   9 /* s5  $21 */
#define CPU_GENERALREG_S6  10 /* s6  $22 */
#define CPU_GENERALREG_S8  11 /* s8  $30 */
#define CPU_GENERALREG_V0  12 /* v0  $2  */
#define CPU_GENERALREG_RA  13 /* ra  $31 */
#define CPU_GENERALREG_A0  14 /* a0  $4  */
#define CPU_GENERALREG_A1  15 /* a1  $5  */
#define CPU_GENERALREG_A2  16 /* a2  $6  */
#define CPU_GENERALREG_A3  17 /* a3  $7  */
#define CPU_GENERALREG_T0  18 /* t0  $8  */
#define CPU_GENERALREG_T1  19 /* t1  $9  */
#define CPU_GENERALREG_T2  20 /* t2  $10 */
#define CPU_GENERALREG_T3  21 /* t3  $11 */
#define CPU_GENERALREG_T4  22 /* t4  $12 */
#define CPU_GENERALREG_T5  23 /* t5  $13 */
#define CPU_GENERALREG_T6  24 /* t6  $14 */
#define CPU_GENERALREG_T7  25 /* t7  $15 */
#define CPU_GENERALREG_T8  26 /* t8  $24 */
#define CPU_GENERALREG_T9  27 /* t9  $25 */
#define CPU_GENERALREG_LO  28 /* lo  */
#define CPU_GENERALREG_HI  29 /* hi  */
#define CPU_GENERALREG_AT  30 /* at  $1  */
#define CPU_GENERALREG_V1  31 /* v1  $3  */


#define	RX_AT	CPU_GENERALREG_AT
#define	RX_V0	CPU_GENERALREG_V0
#define	RX_V1	CPU_GENERALREG_V1
#define	RX_A0	CPU_GENERALREG_A0
#define	RX_A1	CPU_GENERALREG_A1
#define	RX_A2	CPU_GENERALREG_A2
#define	RX_A3	CPU_GENERALREG_A3
#define	RX_T0	CPU_GENERALREG_T0
#define	RX_T1	CPU_GENERALREG_T1
#define	RX_T2	CPU_GENERALREG_T2
#define	RX_T3	CPU_GENERALREG_T3
#define	RX_T4	CPU_GENERALREG_T4
#define	RX_T5	CPU_GENERALREG_T5
#define	RX_T6	CPU_GENERALREG_T6
#define	RX_T7	CPU_GENERALREG_T7
#define	RX_S0	CPU_GENERALREG_S0
#define	RX_S1	CPU_GENERALREG_S1
#define	RX_S2	CPU_GENERALREG_S2
#define	RX_S3	CPU_GENERALREG_S3
#define	RX_S4	CPU_GENERALREG_S4
#define	RX_S5	CPU_GENERALREG_S5
#define	RX_S6	CPU_GENERALREG_S6
#define	RX_S7	CPU_GENERALREG_TB
#define	RX_T8	CPU_GENERALREG_T8
#define	RX_T9	CPU_GENERALREG_T9
#define	RX_GP	CPU_GENERALREG_DB
#define	RX_SP	CPU_GENERALREG_SP
#define	RX_S8	CPU_GENERALREG_S8
#define	RX_RA	CPU_GENERALREG_RA
#define	RX_HI	CPU_GENERALREG_HI
#define	RX_LO	CPU_GENERALREG_LO
#define	RX_PC	CPU_GENERALREG_PC


#define	RX_ARGSAVE0	RX_A0
#define	RX_ARGSAVE1	RX_A1
#define	RX_ARGSAVE2	RX_A2
#define	RX_ARGSAVE3	RX_A3

#ifdef  USEFPU
#define VCPU_USE_EXTRA_PERSISTENT
#define VCPU_USE_EXTRA_TRANSIENT
#endif /* USEFPU */


#ifdef	LANGUAGE_C
/* C or C++ declaration part */

#include	<Types.h>

#ifdef  USEFPU

/* load FPU information if needed */
#ifdef	LANGUAGE_C
#include <FPU.h>
#endif /* LANGUAGE_C */

typedef struct {
    FPUStatRegisters	fpuStat;
} ExtraPersistentResources;

typedef struct {
    FPUGeneralRegisters	fpuGeneral;
} ExtraTransientResources;

#else /* USEFPU */

typedef struct {
} ExtraPersistentResources;

typedef struct {
} ExtraTransientResources;

#endif  /* USEFPU */

typedef longword CPUProtectionMode;

/* for backword compatibility */
typedef CPUProtectionMode CPUMode;

BEGIN_EXTERN_C

typedef union {
    longword lval[32];
    void*    pval[32];
} CPURegisters;

/*
 * R4000 family Persistent registers. 
 * only one entrie status reg.
 */
typedef struct {
	longword	stat;
} CPUStatReg;


typedef CPURegisters TransientResources;
typedef CPUStatReg   PersistentResources;

END_EXTERN_C

#endif /*  LANGUAGE_C */ 

#define	TOOLCHAIN_CODEBASE_REG	22
#define	TOOLCHAIN_DATABASE_REG	25

#endif /* CPU_h_DEFINED */
