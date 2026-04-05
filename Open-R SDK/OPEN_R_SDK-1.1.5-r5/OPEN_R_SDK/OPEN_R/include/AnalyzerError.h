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
 * $Date: Mon Feb 25 16:35:00 JST 2002 $
 * $Revision: /main/tokyo/AIBO9/1 $
 */

#ifndef _AnalyzerError_h_DEFINED
#define _AnalyzerError_h_DEFINED

/*
 * For Compatible
 */
// Same to sSUCCESS
#ifdef _SystemError_h_DEFINED
#define azrSUCCESS	sSUCCESS
#else	/* _SystemError_h_DEFINED */
#define azrSUCCESS	0
#endif	/* _SystemError_h_DEFINED */
// Same to sFAIL
#ifdef _SystemError_h_DEFINED
#define azrFAIL		sFAIL
#else	/* _SystemError_h_DEFINED */
#define azrFAIL		2
#endif	/* _SystemError_h_DEFINED */

/*
 * These are the common errors
 */
#define azrErrorBASE	2000

#define azrINVALIDARG	(azrErrorBASE + 1)
#define azrNOTSUPPORTED	(azrErrorBASE + 2)

#endif /* _AnalyzerError_h_DEFINED */
