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
 * $Date: Mon Feb 25 16:34:57 JST 2002 $
 * $Revision: /main/tokyo/AIBO9/1 $
 */

#ifndef _SegmentInfo_h_DEFINED
#define _SegmentInfo_h_DEFINED

#include <Types.h>

struct SegmentInfo {
    void*	baseAddr;
    size_t	size;
};

#endif  /* _SegmentInfo_h_DEFINED */
