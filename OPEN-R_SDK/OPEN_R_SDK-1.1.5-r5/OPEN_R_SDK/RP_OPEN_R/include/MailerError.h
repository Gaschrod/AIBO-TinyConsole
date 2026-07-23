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

#ifndef _MailerError_h_DEFINED
#define _MailerError_h_DEFINED

#include <Types.h>
#include <SystemError.h>

const sError    sOBJNOTFOUND                = SystemErrorBase + 2;
const sError    sFULLMSGQENTRY              = SystemErrorBase + 3;
const sError    sFULLMSGPOOL                = SystemErrorBase + 4;
const sError    sEXCEEDEDSIZEOFMSG          = SystemErrorBase + 5;
const sError    sEXCEEDEDSIZEOFENTRYTABLE   = SystemErrorBase + 6;
const sError    sTIDCANNOTBECREATED	    = SystemErrorBase + 7;
const sError    sINVALIDTID		    = SystemErrorBase + 8;

const sError    sTIDNOTFOUND		    = SystemErrorBase + 11;

const sError    sFULLEVENTENTRY		    = SystemErrorBase + 15;
const sError    sINVALIDEVENTID		    = SystemErrorBase + 16;
const sError    sINVALIDTEVENTINFO	    = SystemErrorBase + 17;

const sError    sFAILINTERCOM               = SystemErrorBase + 20;
const sError    sINVALIDRESOURCE            = SystemErrorBase + 21;

const sError    sMailer_FullSystemTimerEntry	= SystemErrorBase + 22;

#endif  /* _MailerError_h_DEFINED */
