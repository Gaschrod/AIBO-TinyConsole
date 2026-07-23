//
// OPENR.h
//
// Copyright 1997,1998,1999,2000,2001,2002 Sony Corporation
//

#ifndef _OPENR_h_DEFINED
#define _OPENR_h_DEFINED

#include <string.h>
#include <OID.h>
#include <Types.h>
#include <OPENR/OStatus.h>

//
// OPEN-R System Object & OPEN-R Object Common Entries
//
const Selector oentryINIT    = 0;
const Selector oentrySTART   = 1;
const Selector oentrySTOP    = 2;
const Selector oentryDESTROY = 3;
const Selector oentryBASE    = 4;


//
// OReason
//
typedef longword OReason;
const OReason oreasonUNDEF           =  0xffffffff;
const OReason oreasonBOOT            =  0;
const OReason oreasonSHUTDOWN        =  1;


//
// OMemoryStickStatus
//
typedef longword OMemoryStickStatus;
const OMemoryStickStatus omemorystickUNDEF           = 0xffffffff;
const OMemoryStickStatus omemorystickNOT_EXIST       = 0;
const OMemoryStickStatus omemorystickWRITE_PROTECTED = 1;
const OMemoryStickStatus omemorystickWRITABLE        = 2;


struct OBootParam {
    OMemoryStickStatus memoryStickStatus;

    OBootParam() : memoryStickStatus(omemorystickUNDEF) {}
    OBootParam(OMemoryStickStatus st) : memoryStickStatus(st) {}
};


struct OShutdownParam {
    bool batteryEnough;

    OShutdownParam() : batteryEnough(false) {}
    OShutdownParam(bool enough) : batteryEnough(enough) {}
};


//
// Max Length of OReason Parameter
//
const int oreasonPARAM_MAX_SIZE = 128;


//
// OFatal
//
typedef longword OFatal;
const OFatal ofatalUNDEF        = 0xffffffff;
const OFatal ofatalMEMORY_STICK = 0;
const OFatal ofatalPAUSE_SW     = 1;
const OFatal ofatalORB          = 2;


//
// OConnectCmd
//
typedef longword OConnectCmd;
const OConnectCmd oconnectcmdCONNECT        = 0;
const OConnectCmd oconnectcmdDISCONNECT     = 1;
const OConnectCmd oconnectcmdDISCONNECTALL  = 2;


//
// OServiceEntry
//
struct OServiceEntry {
    OID       oid;
    Selector  selector;

    OServiceEntry() : selector(UNDEF) { oid.MakeInvalid(); }
    OServiceEntry(OID o, Selector s) : oid(o), selector(s) {}
    OServiceEntry(const OServiceEntry& e) : oid(e.oid), selector(e.selector) {}

    void     Set(OID o, Selector s) { oid = o, selector = s; }
    OID      GetOID() const         { return oid;            }
    Selector GetSelector() const    { return selector;       }

    bool IsValid() const {
        return (oid.IsValid() == true && selector != UNDEF) ? true : false;
    }

    void MakeInvalid() { oid.MakeInvalid(); selector = UNDEF; }

    friend bool operator ==(const OServiceEntry& a, const OServiceEntry& b) {
        return (a.oid == b.oid && a.selector == b.selector);
    }

    friend bool operator !=(const OServiceEntry& a, const OServiceEntry& b) {
        return !(a.oid == b.oid && a.selector == b.selector);
    }

    friend bool operator < (const OServiceEntry& a, const OServiceEntry& b) {
        if (a.oid < b.oid) {
            return true;
        } else if (a.oid == b.oid) {
            if (a.selector < b.selector) return true; else return false;
        } else {
            return false;
        }
    }
};


//
// Max length of OPEN-R Service Name
//
const int oserviceNAME_MAX       = 127;

//
// Max length of OPEN-R Object Name
//
const int oobjectNAME_MAX = 63;

//
// Max length of name and pathname of OVirtualRotot
//
const int ovrNAME_MAX = 31;
const int ovrPATH_MAX = 127;

//
// Max length of RobotDesign
//
const int orobotdesignNAME_MAX   =  31;

//
// Max length of design keyword
//
const int odesignkeywordNAME_MAX = 127;

//
// Reserved design keyword
//
#define odesignkeywordCPUINFO   "SYS_CPUINFO"
struct OCPUInfo {               // 256 bytes (total)
    longword sclk;              // system clock (Hz)
    longword pclk;              // pipeline clock (Hz)
    longword processorID;       // Processor ID
    byte reserved[244];
};

//
// Max Frame Number
//
const longword oframeMAX_NUMBER     = 0x0ffffff0;
const longword oframeINVALID_NUMBER = 0xffffffff;

//
// OPEN-R Device ID
//
typedef longword ODeviceID;

//
// OPEN-R Device Type
//
typedef longword ODeviceType;
const ODeviceType odevtypeUNDEF            = 0;
const ODeviceType odevtypeHUB              = 1;
const ODeviceType odevtypeSERVO            = 2;
const ODeviceType odevtypeAUDIO            = 3;
const ODeviceType odevtypeFBK              = 4;

#define odevtypeSERVO_NAME                 "Servo"
#define odevtypeAUDIO_NAME                 "Audio"
#define odevtypeFBK_NAME                   "Fbk"

//
// CPC Primitive ID
//
typedef longword OPrimitiveID;
const OPrimitiveID oprimitiveID_UNDEF = 0xffffffff;

//
// CPC Primitive Type
//
typedef longword OPrimitiveType;
const OPrimitiveType oprimitiveUNDEF           =  0;
const OPrimitiveType oprimitiveCOORDINATE      =  1;
const OPrimitiveType oprimitiveRECEPTOR        =  2;
const OPrimitiveType oprimitivePLUG            =  3;
const OPrimitiveType oprimitiveLINK            =  4;
const OPrimitiveType oprimitiveJOINT           =  5;
const OPrimitiveType oprimitiveSENSOR          =  6;
const OPrimitiveType oprimitiveSPEAKER         =  7;
const OPrimitiveType oprimitiveMIC             =  8;
const OPrimitiveType oprimitiveFBKIMAGESENSOR  =  9;
const OPrimitiveType oprimitiveLED             = 10;
const OPrimitiveType oprimitiveJOINT2          = 11;
const OPrimitiveType oprimitiveLED2            = 12;
const OPrimitiveType oprimitiveJOINT3          = 13;
const OPrimitiveType oprimitiveJOINT4          = 14;
const OPrimitiveType oprimitiveBLUETOOTH       = 15;
const OPrimitiveType oprimitiveLED3            = 16;

//
// CPC Primitive Name
//
#define oprimitiveCOORDINATE_NAME          "Coordinate"
#define oprimitiveRECEPTOR_NAME            "Receptor"
#define oprimitivePLUG_NAME                "Plug"
#define oprimitiveLINK_NAME                "Link"
#define oprimitiveJOINT_NAME               "Joint"
#define oprimitiveJOINT2_NAME              "Joint2"
#define oprimitiveJOINT3_NAME              "Joint3"
#define oprimitiveJOINT4_NAME              "Joint4"
#define oprimitiveSENSOR_NAME              "Sensor"
#define oprimitiveSPEAKER_NAME             "Speaker"
#define oprimitiveMIC_NAME                 "Mic"
#define oprimitiveFBKIMAGESENSOR_NAME      "FbkImageSensor"
#define oprimitiveLED_NAME                 "LED"
#define oprimitiveLED2_NAME                "LED2"
#define oprimitiveLED3_NAME                "LED3"
#define oprimitiveBLUETOOTH_NAME           "Bluetooth"

//
// CPC Model ID
//
typedef longword OCPCModelID;
const OCPCModelID ocpcmodelID_UNDEF = 0xffffffff;

//
// Sensor Type
//
typedef longword OSensorType;
const OSensorType osensorUNDEF            = 0;
const OSensorType osensorSWITCH           = 1;
const OSensorType osensorACCELEROMETER    = 2;
const OSensorType osensorGYROMETER        = 3;
const OSensorType osensorTACTILE_SENSOR   = 4;
const OSensorType osensorTHERMOMETER      = 5;
const OSensorType osensorPSD              = 6;
const OSensorType osensorROTARY_SENSOR    = 7;
const OSensorType osensorBT_SWITCH        = 8;
const OSensorType osensorRAW_SENSOR       = 9;
const OSensorType osensorTM_SENSOR        = 10;

//
// OAysncReqID
//
const size_t oasyncReqID_MAX_SIZE = 32;
typedef longword OAsyncReqID;
const OAsyncReqID oasyncReqID_UNDEF = 0xffffffff;

//
// OVRSyncKey
//
const size_t ovrsynckeyMAX_SIZE         =  8; // LEGACY SUPPORT
const size_t ovrsynckeyMAX_NUM_KEY      =  8;
const size_t ovrsynckeyMAX_NUM_DIVISION = 16;
typedef longword OVRSyncKey;
const OVRSyncKey ovrsynckeyUNDEF = 0xffffffff;

//
// OVRRequest
//
const size_t ovrctrlPARAM_MAX_SIZE = 192;
const size_t ovrctrlDEVICE_MAX     = 32;
typedef longword OVRRequest;
const OVRRequest ovrreqUNDEF = 0xffffffff;

//
// OPrimitiveRequest
//
const size_t oprmctrlPARAM_MAX_SIZE = ovrctrlPARAM_MAX_SIZE - 16;
typedef longword OPrimitiveRequest;
const OPrimitiveRequest oprmreqUNDEF = 0xffffffff;

//
// ODesignDataID
//
typedef longword ODesignDataID;
const ODesignDataID odesigndataID_UNDEF = 0xffffffff;

//
// OVolumeSwitch
//
typedef byte OVolumeSwitch;
const OVolumeSwitch ovolumeSW0 = 0;
const OVolumeSwitch ovolumeSW1 = 1;
const OVolumeSwitch ovolumeSW2 = 2;
const OVolumeSwitch ovolumeSW3 = 3;

#endif /* _OPENR_h_DEFINED */
