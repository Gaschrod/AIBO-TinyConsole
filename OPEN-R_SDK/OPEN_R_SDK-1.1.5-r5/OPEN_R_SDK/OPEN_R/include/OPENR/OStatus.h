//
// OStatus.h
//
// Copyright 1997,1998,1999,2000,2001,2003 Sony Corporation
//

#ifndef _OStatus_h_DEFINED
#define _OStatus_h_DEFINED

#include <Types.h>

//
// OStatus
//
typedef longword OStatus;

const OStatus OStatusCommonBase          =    0;
const OStatus OStatusSystemInternalBase  =  256;
const OStatus OStatusAPISpecificBase     = 4096;
const OStatus OStatusAlertBase           = 8192;

//
// Common status codes (0 <---> 255)
//
const OStatus oSUCCESS              = OStatusCommonBase +  0;
const OStatus oUNDEF                = OStatusCommonBase +  1;
const OStatus oFAIL                 = OStatusCommonBase +  2;
const OStatus oNOT_SUPPORTED        = OStatusCommonBase +  3;
const OStatus oINVALID_ARG          = OStatusCommonBase +  4;
const OStatus oNO_MEMORY            = OStatusCommonBase +  5;
const OStatus oNOT_FOUND            = OStatusCommonBase +  6;
const OStatus oALREADY_EXIST        = OStatusCommonBase +  7;
const OStatus oOPEN_FAILURE         = OStatusCommonBase +  8;
const OStatus oCLOSE_FAILURE        = OStatusCommonBase +  9;
const OStatus oFULL_OF_QUEUE        = OStatusCommonBase +  10;
const OStatus oITEM_NOT_FOUND       = OStatusCommonBase +  11;
const OStatus oWRONG_MESSAGE        = OStatusCommonBase +  12;
const OStatus oSEND_FAIL            = OStatusCommonBase +  13;
const OStatus oSEND_WITH_RBOX_FAIL  = OStatusCommonBase +  14;
const OStatus oCALL_FAIL            = OStatusCommonBase +  15;
const OStatus oREPLY_FAIL           = OStatusCommonBase +  16;
const OStatus oRECEIVE_FAIL         = OStatusCommonBase +  17;
const OStatus oNOT_INITIALIZED      = OStatusCommonBase +  18;
const OStatus oREFUSED              = OStatusCommonBase +  19;
const OStatus oSIZE_TOO_LARGE       = OStatusCommonBase +  20;
const OStatus oNOT_AVAILABLE        = OStatusCommonBase +  21;
const OStatus oUNKNOWN_DATA         = OStatusCommonBase +  22;
const OStatus oUNKNOWN_COMMAND      = OStatusCommonBase +  23;
const OStatus oOUT_OF_RANGE         = OStatusCommonBase +  24;
const OStatus oOVERFLOW             = OStatusCommonBase +  25;
const OStatus oUNDERFLOW            = OStatusCommonBase +  26;
const OStatus oINVALID_OID          = OStatusCommonBase +  27;
const OStatus oFATAL                = OStatusCommonBase +  28;
const OStatus oOBJ_NOT_FOUND        = OStatusCommonBase +  29;
const OStatus oSYNTAX_ERROR         = OStatusCommonBase +  30;
const OStatus oNO_ROBOT_DESIGN      = OStatusCommonBase +  31;
const OStatus oCHECK_SUM_ERROR      = OStatusCommonBase +  32;
const OStatus oBUSY                 = OStatusCommonBase +  33;

//
// System Internal status codes (256 <---> 4095)
//
const OStatus oOBJECTMANAGER_START_INVOKED   = OStatusSystemInternalBase +  0;
const OStatus oOBJECTMANAGER_STOP_INVOKED    = OStatusSystemInternalBase +  1;
const OStatus oOBJECTMANAGER_DESTROY_INVOKED = OStatusSystemInternalBase +  2;
const OStatus oSYSTEM_OBJECTS_CANT_INIT      = OStatusSystemInternalBase +  3;
const OStatus oSYSTEM_OBJECTS_CANT_START     = OStatusSystemInternalBase +  4;
const OStatus oSYSTEM_OBJECTS_CANT_STOP      = OStatusSystemInternalBase +  5;
const OStatus oSYSTEM_OBJECTS_CANT_DESTROY   = OStatusSystemInternalBase +  6;
const OStatus oOBJECTS_CANT_INIT             = OStatusSystemInternalBase +  7;
const OStatus oOBJECTS_CANT_START            = OStatusSystemInternalBase +  8;
const OStatus oOBJECTS_CANT_STOP             = OStatusSystemInternalBase +  9;
const OStatus oOBJECTS_CANT_DESTROY          = OStatusSystemInternalBase + 10;
const OStatus oINVALID_SYSTEM_OBJECT_NAME    = OStatusSystemInternalBase + 11;
const OStatus oOPEN_REF_COUNT_IS_ZERO        = OStatusSystemInternalBase + 12;
const OStatus oLUT_INVALID_STATE             = OStatusSystemInternalBase + 13;
const OStatus oLUT_SHORT_OF_DATA             = OStatusSystemInternalBase + 14;
const OStatus oPWM_DUTY_OVER_LIMIT           = OStatusSystemInternalBase + 15;
const OStatus oVR_FOR_DIAG                   = OStatusSystemInternalBase + 16;
const OStatus oDEVICE_CONNECTED              = OStatusSystemInternalBase + 17;
const OStatus oDEVICE_DISCONNECTED           = OStatusSystemInternalBase + 18;
const OStatus oAGRUS_DIAG_NG                 = OStatusSystemInternalBase + 19;

//
// API Specific status codes (4096 <--> 8191)
//
const OStatus oINVALID_PRIMITIVE_ID       = OStatusAPISpecificBase +  0;
const OStatus oINVALID_DATA_VECTOR_ID     = OStatusAPISpecificBase +  1;
const OStatus oINVALID_DATA_TYPE          = OStatusAPISpecificBase +  2;
const OStatus oINVALID_DATA               = OStatusAPISpecificBase +  3;
const OStatus oINVALID_REQUEST            = OStatusAPISpecificBase +  4;
const OStatus oALREADY_REGISTERED         = OStatusAPISpecificBase +  5;
const OStatus oNOT_REGISTERED             = OStatusAPISpecificBase +  6;
const OStatus oEXCEEDED_PARAM_SIZE        = OStatusAPISpecificBase +  7;
const OStatus oINVALID_ASYNC_REQ_ID       = OStatusAPISpecificBase +  8;
const OStatus oFULL_OF_ASYNC_QUEUE        = OStatusAPISpecificBase +  9;
const OStatus oNO_SYNC_KEY                = OStatusAPISpecificBase + 10;
const OStatus oINVALID_SYNC_KEY           = OStatusAPISpecificBase + 11;
const OStatus oGAIN_DISABLED              = OStatusAPISpecificBase + 12;
const OStatus oTOO_MANY_ODA               = OStatusAPISpecificBase + 13;
const OStatus oDESIGNDATA_SIZE_ZERO       = OStatusAPISpecificBase + 14;
const OStatus oCPC_PLUG_OUT               = OStatusAPISpecificBase + 15;
const OStatus oMS_NOT_WRITABLE            = OStatusAPISpecificBase + 16;

//
// ALERT status codes (8192 <-- )
//
const OStatus oALERT_JOINT_UNCONTROLLABLE = OStatusAlertBase +  0;

#endif // _OStatus_h_DEFINED

