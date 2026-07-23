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
 * $Date: Mon Feb 25 16:33:07 JST 2002 $
 * $Revision: /main/tokyo/AIBO9/1 $
 */

#ifndef _SystemError_h_DEFINED
#define _SystemError_h_DEFINED

/* This value should not be changed. */
#define sSUCCESS      0


/* error value for the MetaCore primitives
 *   mcSUCCESS      The specified primitive has been successfully ended.
 *   mcCBIND	    The specified vector has not been defined. 
 *   mcCTXTBUSY     Context is busy. 
 *   mcCTXTSUSPEND  Context is suspended. 
 *   mcHARDMSG      Structure HardMessage is required to CBind. 
 *   mcINSTALLED    The specified vector has already been installed. 
 *   mcINVENTRY     The specified entry field is invalid. 
 *   mcINVMODE      The mode field has been invalid. 
 *   mcINVSELECT    The selector of structure Message has been invalid.
 *   mcINVSTAT      The state of Context has been invalid. 
 *   mcINVTMP       The address of temporary has been invalid. 
 *   mcMFAILED      Primitive M has failed. 
 *   mcNOBASE       This Context had no link to the base Context. 
 *   mcNOMETA       There has been no meta-Contexts. 
 *   mcNOTEMP       There has been no temporary region. 
 *   mcPRIMITIVE    The specified primitive is not defined. 
 *   mcRFAILED      Primitive R has failed. 
 */

#define VCPUErrorBase -1

#define mcCBIND	      (VCPUErrorBase -  0)
#define mcCTXTBUSY    (VCPUErrorBase -  1)
#define mcCTXTSUSPEND (VCPUErrorBase -  2)
#define mcHARDMSG     (VCPUErrorBase -  3)
#define mcINSTALLED   (VCPUErrorBase -  4)
#define mcINVENTRY    (VCPUErrorBase -  5)
#define mcINVMODE     (VCPUErrorBase -  6)
#define mcINVSELECT   (VCPUErrorBase -  7)
#define mcINVSTAT     (VCPUErrorBase -  8)
#define mcINVTMP      (VCPUErrorBase -  9)
#define mcMFAILED     (VCPUErrorBase - 10)
#define mcNOBASE      (VCPUErrorBase - 11)
#define mcNOMETA      (VCPUErrorBase - 12)
#define mcNOTEMP      (VCPUErrorBase - 13)
#define mcPRIMITIVE   (VCPUErrorBase - 14)
#define mcRFAILED     (VCPUErrorBase - 15)
#define errVCPUINVINST   (VCPUErrorBase - 16)
#define errPLIBINVINDEX (VCPUErrorBase - 17)

#define sErrorBASE 0

/*
 * These are the common errors for the objects.
 */
#define SystemCommonErrorBase (sErrorBASE+0)
/* SystemCommonErrorBase + 0 is overlaped with sSUCCESS */
/* common error starts from offset 1                    */
#define sUNDEF        (SystemCommonErrorBase + 1)
#define sFAIL         (SystemCommonErrorBase + 2)
#define sNOTSUPPORTED (SystemCommonErrorBase + 3)
#define sNOTENOUGHRES (SystemCommonErrorBase + 4)
#define sINVALIDARG   (SystemCommonErrorBase + 5)
#define sNOMEMORY     (SystemCommonErrorBase + 6)
#define sINVALIDOID   (SystemCommonErrorBase + 7)

#define sNOTIMPLYET   (SystemCommonErrorBase + 9)


/*
 * Each metaspace has its own status codes. MetaspaceError.h defines such
 * error from SystemErrorBase... (cf. MCOOPError.h is available for mCOOP)
 */
#define SystemErrorBase  (sErrorBASE+10)
/* range of SystemError : 10 - 399 */

/*
 * Global (cross over meta space) API related error number.
 * Meaning of error is specific to each API.
 * So following error number is not a part of SystemCommonError.
 *
 * Global errors are classified into 2 category from view point of mechanism.
 * One is VCPU error.
 * Another is error whitch meta objects issues.
 * To keep independency of VCPU and objects,
 * range of these errors are separated.
 */

#define VCPUAPIErrorBase    (sErrorBASE+400)
/* range of VCPUAPIError : 400 - 599 */

#define GlobalLibAPIErrorBase    (sErrorBASE+600)
/* range of GLOBALibAPIError : 600 - 999 */

        /* enter to TimeCriticalSection twice */
#define errVCPUTCSREENTER   (VCPUAPIErrorBase + 0)
        /* exit TimeCriticalSection before enter */
#define errVCPUTCSREEXIT    (VCPUAPIErrorBase + 1)


#define GlobalLibErrorBase  (sErrorBASE+600)

/*
 * Common but internal error status code.
 * defined in SystemInternalError.h
 */
#define SystemInternalErrorBase   (sErrorBASE+1000)

/*
 * RTCDriverErrorBase needs for compatible library of calendar time.
 */
#define RTCDriverErrorBase	(SystemInternalErrorBase + 20000)

/*
 * API Specific error status code.
 * defined in the header file for each APIs.
 */
#define APISpecificErrorBase (sErrorBASE+2000)

#endif /* _SystemError_h_DEFINED */
