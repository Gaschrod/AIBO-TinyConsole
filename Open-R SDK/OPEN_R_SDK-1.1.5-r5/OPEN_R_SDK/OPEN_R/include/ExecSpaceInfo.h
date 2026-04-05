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
 * $Date: Mon Feb 25 16:34:55 JST 2002 $
 * $Revision: /main/tokyo/AIBO9/1 $
 */

#ifndef _ExecSpaceInfo_h_DEFINED
#define _ExecSpaceInfo_h_DEFINED

#include <Types.h>
#include <RegistryTypes.h>
#include <string.h>

struct ExecSpaceInfo {
    enum	MetaSpaceType {mCOOP = 0, mDrive, mAV, mCore, mSystem, mClass};

    ExecSpaceID		execSpace;
    OID			object;
    char		name[NAME_MAX];
    MetaSpaceType	metaSpaceType;

    ExecSpaceInfo() : execSpace(0) {}
    ExecSpaceInfo(ExecSpaceID esid, OID oid, char *str)
	: execSpace(esid), object(oid) {
	    strncpy(name, str, NAME_MAX);
    }
};

#endif  /* _ExecSpaceInfo_h_DEFINED */
