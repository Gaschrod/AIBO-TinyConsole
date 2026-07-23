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
 * $Date: Mon Feb 25 16:34:56 JST 2002 $
 * $Revision: /main/tokyo/AIBO9/1 $
 */

#ifndef _CPUResisterInfo_h_DEFINED
#define _CPUResisterInfo_h_DEFINED

/* For API User */
struct CPURegisterInfo {
    longword	rAT;	/* general register #1 */
    longword	rV0;	/* general register #2 */
    longword	rV1;	/* general register #3 */
    longword	rA0;	/* general register #4 */
    longword	rA1;	/* general register #5 */
    longword	rA2;	/* general register #6 */
    longword	rA3;	/* general register #7 */
    longword	rT0;	/* general register #8 */
    longword	rT1;	/* general register #9 */
    longword	rT2;	/* general register #10 */
    longword	rT3;	/* general register #11 */
    longword	rT4;	/* general register #12 */
    longword	rT5;	/* general register #13 */
    longword	rT6;	/* general register #14 */
    longword	rT7;	/* general register #15 */
    longword	rS0;	/* general register #16 */
    longword	rS1;	/* general register #17 */
    longword	rS2;	/* general register #18 */
    longword	rS3;	/* general register #19 */
    longword	rS4;	/* general register #20 */
    longword	rS5;	/* general register #21 */
    longword	rS6;	/* general register #22 */
    longword	rS7;	/* general register #23 */
    longword	rT8;	/* general register #24 */
    longword	rT9;	/* general register #25 */
    void*	rGP;	/* general register #28 */
    void*	rSP;	/* general register #29 */
    longword	rS8;	/* general register #30 */
    void*	rRA;	/* general register #31 */
    longword	rHI;	/* multiply/divide registers */
    longword	rLO;	/* multiply/divide registers */
    void*	rPC;	/* program counter */
    longword	rSR;	/* status register */
};

#ifdef DEBUGGER_SUPPORT
/* For Debugger */
typedef	CPURegisterInfo AO_CPURegisterInfoForDebugger
#endif /* DEBUGGER_SUPPORT */

#endif /* _CPUResisterInfo_h_DEFINED */
