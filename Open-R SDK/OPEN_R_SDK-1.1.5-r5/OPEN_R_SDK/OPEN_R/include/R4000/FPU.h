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
 * $Date: Mon Feb 25 16:33:13 JST 2002 $
 * $Revision: /main/tokyo/AIBO9/1 $
 */

#ifndef	FPU_h_DEFINED
#define	FPU_h_DEFINED

#define VCPU_FPU_STATREG_OFS	0

#ifdef LANGUAGE_C

#include <Types.h>
/*
 * MIPS R4000 family FPU Control/Status Registers.
 *  only fcr31 should be saved.
 */
typedef union {
    longword	lval[1];
} FPUStatRegisters;


/*
 * MIPS R4000 family FPU General Registers.
 *  32 float registers or 16 double registers.
 *  64bit mode (32 double registers) is not supported.
 */
typedef union {
    float	fval[32];
    double	dval[16];
} FPUGeneralRegisters;

#endif
    
#endif	/* FPU_h_DEFINED */
