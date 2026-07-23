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
 * $Date: Mon Feb 25 16:34:51 JST 2002 $
 * $Revision: /main/tokyo/AIBO9/1 $
 */

#ifndef ContextInfo_h_DEFINED
#define ContextInfo_h_DEFINED

#include <Types.h>
#include <CPURegisterInfo.h>

struct ContextInfo {
    CName		ctxt;
    /* status */
    longword		state;
    CPURegisterInfo	registers;

    /* stack */
    longword		stackSize;
    void		*stackBase;

    /* mode */
    longword		cpuProtect;
    longword		intrPriority;
};

#endif /* ContextInfo_h_DEFINED */
