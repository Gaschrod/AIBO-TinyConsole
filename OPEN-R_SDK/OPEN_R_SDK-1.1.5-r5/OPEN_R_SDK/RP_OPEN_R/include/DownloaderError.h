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
 * Copyright 1988-1999 Sony Corporation
 * All Rights Reserved.
 *
 */
/*
 * $Date: 2002/01/28 02:16:00 $
 * $Revision: 1.3 $
 */

#ifndef	_DownloaderError_h_DEFINED
#define	_DownloaderError_h_DEFINED

#include	<SystemError.h>

const sError	sNOMODULE			= SystemErrorBase + 80;
const sError	sBADFORMATMODULE		= SystemErrorBase + 81;
const sError	sBADCONFIGMODULE		= SystemErrorBase + 82;
const sError	sOBLETNOTFOUND			= SystemErrorBase + 83;
const sError	sOBLETNAMETOOLONG		= SystemErrorBase + 84;

#endif	/* _DownloaderError_h_DEFINED */
