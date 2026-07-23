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
 * $Date: 2002/01/28 02:58:43 $
 * $Revision: 1.4 $
 */

#ifndef _MCOOPError_h_DEFINED
#define _MCOOPError_h_DEFINED

#include <Types.h>
#include <SystemError.h>
#include <MailerError.h>
#include <DownloaderError.h>

const sError    sRBOXALREADYUSED            = SystemErrorBase + 9;
const sError    sRIDCANNOTBETREATED         = SystemErrorBase + 10;

const sError    sRBOXWASCLEANED             = SystemErrorBase + 12;
const sError    sREPLIERFAILED              = SystemErrorBase + 13;
const sError    sNOTENOUGHSIZEOFMSG         = SystemErrorBase + 14;

const sError    sTIMEOUT		    = SystemErrorBase + 18;
const sError    sMAILERNOTFOUND             = SystemErrorBase + 19;

const sError    sALREADYATTACHEDSHAREDMEM   = SystemErrorBase + 31;
const sError    sINVALIDSHAREDMEMID         = SystemErrorBase + 32;
const sError    sINVALIDSHAREDMEMPROTECT    = SystemErrorBase + 33;
const sError    sPERMISSIONDENIEDSHAREDMEM  = SystemErrorBase + 34;

const sError    sNOTIMERAVAILABLE           = SystemErrorBase + 70;
	/* this value must be same as MDriveError.h */

const sError    sRBOXCANNOTBECREATED        = sTIDCANNOTBECREATED;
const sError    sINVALIDRID                 = sINVALIDTID;
const sError    sRIDNOTFOUND                = sTIDNOTFOUND;

const sError    sINVALIDREPLY               = sRBOXALREADYUSED;
const sError    sFULLMSGQENT                = sFULLMSGQENTRY;
const sError    sFULLTEVENTMSGPOOL          = sFULLEVENTENTRY;
const sError    sFULLREVENTENT              = sFULLEVENTENTRY;

const sError    sSHMALREADYATTACHED         = sALREADYATTACHEDSHAREDMEM;
const sError    sSHMINVALIDBLOCKID          = sINVALIDSHAREDMEMID;
const sError    sSHMINVALIDPROTECT          = sINVALIDSHAREDMEMPROTECT;
const sError    sSHMPERMISSIONDENIED        = sPERMISSIONDENIEDSHAREDMEM;


#endif  /* _MCOOPError_h_DEFINED */
