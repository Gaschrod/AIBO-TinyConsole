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
 * $Date: Mon Feb 25 16:34:19 JST 2002 $
 * $Revision: /main/tokyo/AIBO9/1 $
 */

#ifndef _SystemResource_h_DEFINED
#define _SystemResource_h_DEFINED

#include    <Types.h>


typedef	longword	sResource;

const	longword	SystemCommonResourceBase	= 0;

/*
 * These are the common resource for the objects.
 */
const sResource	rUNDEF			= SystemCommonResourceBase + 0;

const	longword	SystemResourceBase		= 10;

/*
 * Each metaspace has its own resource codes. MetaspaceResource.h defines
 * each resource from SystemResourceBase.
 */

/*
 * These resources are used in mCOOP and mDrive.
 */
const sResource	rMAXMESSAGESIZE		= SystemResourceBase + 0;
const sResource	rMAXQUEUESIZE		= SystemResourceBase + 1;
const sResource	rMAXEVENT		= SystemResourceBase + 2;
const sResource	rMAXENTRIES		= SystemResourceBase + 3;

/*
 * These resouruces are used in mDrive
 */
const sResource rMAXSWITCHNUMOFS	= SystemResourceBase + 4;

#define rMAXSWITCHNUM(x)	(rMAXSWITCHNUMOFS + x)

const longword	NumberOfResource	= SystemResourceBase + 4 + 256;

#endif  /* _SystemResource_h_DEFINED */
