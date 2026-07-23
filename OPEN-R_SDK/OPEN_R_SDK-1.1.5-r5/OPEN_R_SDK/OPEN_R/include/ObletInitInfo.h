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
 * $Date: Mon Feb 25 16:33:32 JST 2002 $
 * $Revision: /main/tokyo/AIBO9/1 $
 */

#ifndef	_ObletInitInfo_h_DEFINED
#define	_ObletInitInfo_h_DEFINED

#include	<ObletDependencyDescrList.h>

/*
 * This structure is used to get the initialization information about oblet
 * from bootInfo.
 */

struct ObletInitInfo {
	ObletDependencyDescrList*	pList;
	ObletSpec*			pSpec;
  size_t pGOT_Offset;
};

#endif	/* _ObletInitInfo_h_DEFINED */
