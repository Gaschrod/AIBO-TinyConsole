//
// OPENRMessages.h
//
// Copyright 1997,1998,1999,2000,2001,2002 Sony Corporation
//

#ifndef _OPENRMessages_h_DEFINED
#define _OPENRMessages_h_DEFINED

#include <string.h>
#include <SystemError.h>    // sUNDEF

#include <OPENR/OPENR.h>
#include <OPENR/ODataFormats.h>
#include <OPENR/OVRData.h>
#include <OPENR/OTime.h>
#include <OPENR/OPower.h>
#include <OPENR/OSyslog.h>
#include <OPENR/MWInformationTypes.h>

//
// OPEN-R Message Type 
//
//   0    <--> 1023  System  
//   1024 <--        Robotic
//
typedef longword OMessageType;

const OMessageType OMessageSystemBase  =    0;
const OMessageType OMessageRoboticBase = 1024;

//
// System Messages
//
const OMessageType omsgNULL             = OMessageSystemBase + 0;
const OMessageType omsgSYS_ERROR        = OMessageSystemBase + 1;
const OMessageType omsgOSTATUS          = OMessageSystemBase + 2;
const OMessageType omsgREASON           = OMessageSystemBase + 3;
const OMessageType omsgOID              = OMessageSystemBase + 4;
const OMessageType omsgSYSOBJ_INFO      = OMessageSystemBase + 5;
const OMessageType omsgCONNECT          = OMessageSystemBase + 6;
const OMessageType omsgSERVICE_NAME     = OMessageSystemBase + 7;
const OMessageType omsgSERVICE_ENTRY    = OMessageSystemBase + 8;
const OMessageType omsgSERVICE_BINDING  = OMessageSystemBase + 9;
const OMessageType omsgMEMORY_REGION_ID = OMessageSystemBase +10;
const OMessageType omsgMWINFO_ID        = OMessageSystemBase +11;
const OMessageType omsgSYSLOG           = OMessageSystemBase +12;
const OMessageType omsgROBOT_DESIGN     = OMessageSystemBase +13;
const OMessageType omsgDESIGN_KEYWORD   = OMessageSystemBase +14;
const OMessageType omsgDESIGN_DATA      = OMessageSystemBase +15;
const OMessageType omsgFATAL            = OMessageSystemBase +16;
const OMessageType omsgMEMORY_STICK     = OMessageSystemBase +17;

//
// Robotic Messages
//
const OMessageType omsgPRIMITIVE_LOCATOR    = OMessageRoboticBase + 0;
const OMessageType omsgPRIMITIVE_ID         = OMessageRoboticBase + 1;
const OMessageType omsgVR_DATA              = OMessageRoboticBase + 2;
const OMessageType omsgVR_CONTROL           = OMessageRoboticBase + 3;
const OMessageType omsgTIME                 = OMessageRoboticBase + 4;
const OMessageType omsgPOWER_MODE           = OMessageRoboticBase + 5;
const OMessageType omsgPOWER_STATUS         = OMessageRoboticBase + 6;
const OMessageType omsgMOTOR_POWER          = OMessageRoboticBase + 7;
const OMessageType omsgPLUNGER              = OMessageRoboticBase + 8;
const OMessageType omsgALARM                = OMessageRoboticBase + 9;
const OMessageType omsgOBSERVE_POWER_STATUS = OMessageRoboticBase +10;
const OMessageType omsgVOLUME               = OMessageRoboticBase +11;
const OMessageType omsgBMN_DATA_STREAM      = OMessageRoboticBase +12;
const OMessageType omsgSTATION_REGISTER     = OMessageRoboticBase +13;
const OMessageType omsgBOOT_CONDITION       = OMessageRoboticBase +14;
const OMessageType omsgBMN_MODE             = OMessageRoboticBase +15;
const OMessageType omsgBMN_SPECIAL_MODE     = OMessageRoboticBase +16;


//
//-------------------------------------------------------------------------
// System Messages
//-------------------------------------------------------------------------
//
struct ONullMessage {
    OMessageType  msgType;

    ONullMessage() : msgType(omsgNULL) {}
};


struct OsErrorMessage {
    OMessageType  msgType;
    sError        error;

    OsErrorMessage() : msgType(omsgSYS_ERROR), error(sUNDEF) {}
    OsErrorMessage(sError err) : msgType(omsgSYS_ERROR), error(err) {}
};


struct OStatusMessage {
    OMessageType  msgType;
    OStatus       status;

    OStatusMessage() : msgType(omsgOSTATUS), status(oUNDEF) {}    
    OStatusMessage(OStatus s) : msgType(omsgOSTATUS), status(s) {}
};


struct OReasonMessage {
    OMessageType  msgType;
    OReason       reason;
    size_t        paramSize;
    byte          param[oreasonPARAM_MAX_SIZE];

    OReasonMessage(OReason r)
        : msgType(omsgREASON), reason(r), paramSize(0) {}
    OReasonMessage(OReason r, const void* prm, size_t size) :
    msgType(omsgREASON), reason(r), paramSize(size) {
        memcpy(param, prm, size);
    }
};


struct OOIDMessage {
    OMessageType  msgType;
    OID           oid;

    OOIDMessage() : msgType(omsgOID) {}
    OOIDMessage(OID o) : msgType(omsgOID), oid(o) {}
};


struct OSystemObjectInfoMessage {
    OMessageType  msgType;
    OID           oid;
    char          name[oobjectNAME_MAX + 1];

    OSystemObjectInfoMessage(OID o, const char* n)
        : msgType(omsgSYSOBJ_INFO), oid(o){
            strcpy(name, n);
        }
};


struct OConnectMessage {
    OMessageType    msgType;
    OConnectCmd     command;
    OServiceEntry   entry;
    char            subName[oserviceNAME_MAX + 1];

    OConnectMessage() : msgType(omsgCONNECT)   { }
    OConnectMessage(OConnectCmd c)
        : msgType(omsgCONNECT),command(c)      { }
};


struct OServiceNameMessage {
    OMessageType    msgType;
    char name[oserviceNAME_MAX + 1];
    OID oid;			// added for checking.

    OServiceNameMessage() : msgType(omsgSERVICE_NAME) { name[0] = '\0'; }
    OServiceNameMessage(const char* n) :
    msgType(omsgSERVICE_NAME) {
        strcpy(name, n);
    }
    OServiceNameMessage(const char* n, const OID& o) :
    msgType(omsgSERVICE_NAME), oid(o) {
        strcpy(name, n);
    }
};


struct OServiceEntryMessage {
    OMessageType    msgType;
    OServiceEntry   entry;

    OServiceEntryMessage() : msgType(omsgSERVICE_ENTRY) { }
    OServiceEntryMessage(const OServiceEntry& e)
        : msgType(omsgSERVICE_ENTRY), entry(e)          { }
    OServiceEntryMessage(OID o, Selector s)
        : msgType(omsgSERVICE_ENTRY), entry(o,s)        { }
};


struct OServiceBindingMessage {
    OMessageType  msgType;
    OServiceEntry entry;
    char          name[oserviceNAME_MAX + 1];
    
    OServiceBindingMessage() 
        : msgType(omsgSERVICE_BINDING)  { name[0] = '\0'; }
    OServiceBindingMessage(const OServiceEntry& e, const char* n)
        : msgType(omsgSERVICE_BINDING), entry(e) { strcpy(name,n); }
};


struct OMemoryRegionIDMessage {
    OMessageType    msgType;
    MemoryRegionID  memID;
    OStatus         status;

    OMemoryRegionIDMessage() : msgType(omsgMEMORY_REGION_ID)  {}
    OMemoryRegionIDMessage(MemoryRegionID id, OStatus s)
        : msgType(omsgMEMORY_REGION_ID), memID(id), status(s) {}
};


struct OMWInfoIDMessage {
    OMessageType    msgType;
    MWIID           mwiID;
    OStatus         status;

    OMWInfoIDMessage() : msgType(omsgMWINFO_ID)  {}
    OMWInfoIDMessage(MWIID id, OStatus s)
        : msgType(omsgMWINFO_ID), mwiID(id), status(s) {}
};


struct OSyslogMessage {
    OMessageType     msgType;
    OID		     oid;
    OSyslogPriority  priority;
    char             log[osyslogMAX_SIZE + 1];

    OSyslogMessage() :
    msgType(omsgSYSLOG), priority(osyslogWARNING) { log[0] = 0; }
};


struct ORobotDesignMessage {
    OMessageType    msgType;
    char            robotDesign[orobotdesignNAME_MAX + 1];
    OStatus         status;

    ORobotDesignMessage()
        : msgType(omsgROBOT_DESIGN), status(oUNDEF) { robotDesign[0] = 0; }
    ORobotDesignMessage(char* n, OStatus st) :
    msgType(omsgROBOT_DESIGN), status(st) {
        strcpy(robotDesign, n);
    }
};


struct ODesignKeywordMessage {
    OMessageType    msgType;
    char keyword[odesignkeywordNAME_MAX + 1];

    ODesignKeywordMessage() : msgType(omsgDESIGN_KEYWORD) { keyword[0] = 0; }
    ODesignKeywordMessage(const char* n) : msgType(omsgDESIGN_KEYWORD) {
        strcpy(keyword, n);
    }
};


struct ODesignDataMessage {
    OMessageType    msgType;
    ODesignDataID   dataID;
    size_t          size;
    OStatus         status;

    ODesignDataMessage() : msgType(omsgDESIGN_DATA)  {}
    ODesignDataMessage(ODesignDataID id)
        : msgType(omsgDESIGN_DATA), dataID(id), size(0), status(oUNDEF) {}
    ODesignDataMessage(ODesignDataID id, size_t sz, OStatus st)
        : msgType(omsgDESIGN_DATA), dataID(id), size(sz), status(st) {}
};


struct OFatalMessage {
    OMessageType  msgType;
    OFatal        fatal;

    OFatalMessage() : msgType(omsgFATAL), fatal(ofatalUNDEF) {}    
    OFatalMessage(OFatal f) : msgType(omsgFATAL), fatal(f) {}
};


struct OMemoryStickStatusMessage {
    OMessageType        msgType;
    OMemoryStickStatus  status;

    OMemoryStickStatusMessage()
        : msgType(omsgMEMORY_STICK), status(omemorystickUNDEF) {}
    OMemoryStickStatusMessage(OMemoryStickStatus st)
        : msgType(omsgMEMORY_STICK), status(st) {}
};


//
//-------------------------------------------------------------------------
// Robotic Messages
//-------------------------------------------------------------------------
//
struct OPrimitiveLocatorMessage {
    OMessageType  msgType;
    char          locator[ovrPATH_MAX + 1]; 

    OPrimitiveLocatorMessage(const char* l) : msgType(omsgPRIMITIVE_LOCATOR) {
        strcpy(locator, l);
    }
};


struct OPrimitiveIDMessage {
    OMessageType  msgType;
    OStatus       status;
    OPrimitiveID  primitiveID;

    OPrimitiveIDMessage() :
    msgType(omsgPRIMITIVE_ID), status(oUNDEF), primitiveID(0) {}

    // For Request
    OPrimitiveIDMessage(OPrimitiveID prmID) :
    msgType(omsgPRIMITIVE_ID), status(oUNDEF), primitiveID(prmID) {}

    // For Result
    OPrimitiveIDMessage(OStatus st, OPrimitiveID prmID) :
    msgType(omsgPRIMITIVE_ID), status(st), primitiveID(prmID) {}
};


struct OVRDataMessage {
    OMessageType    msgType;
    OServiceEntry   entry;
    OAsyncReqID     asyncReqID;
    size_t          numData;
    OVRData         data[ovrdataMAX_SIZE];

    OVRDataMessage() :
    msgType(omsgVR_DATA), entry(), asyncReqID(0), numData(0) {}
    OVRDataMessage(OVRData* dat, size_t nData,
                   const OServiceEntry& e, OAsyncReqID reqID) :
    msgType(omsgVR_DATA), entry(e), asyncReqID(reqID) {
        numData = nData;
        for (size_t i = 0; i < numData; i++) data[i] = dat[i];
    }
};


struct OVRControlMessage {
    OMessageType    msgType;	                  //   4 bytes
    OStatus         status;			              //   4 bytes
    OVRRequest      request;			          //   4 bytes
    OServiceEntry   entry;			              //   8 bytse
    OAsyncReqID     asyncReqID;			          //   4 bytes
    size_t          paramSize;			          //   4 bytes
    longword        padding;			          //   4 bytes
    byte            param[ovrctrlPARAM_MAX_SIZE]; // 192 bytes
    
    // Default
    OVRControlMessage() : 
    msgType(omsgVR_CONTROL), status(oUNDEF), request(ovrreqUNDEF), entry(),
    asyncReqID(0), paramSize(0) {}

    // Request
    OVRControlMessage(OVRRequest req, void* prm, size_t size) :
    msgType(omsgVR_CONTROL), status(oUNDEF), request(req), entry(),
    asyncReqID(0), paramSize(size) {
        memcpy(param, prm, size);
    }

    // Asynchronous Request
    OVRControlMessage(OVRRequest req, void* prm, size_t size,
                      const OServiceEntry& e, OAsyncReqID reqID) :
    msgType(omsgVR_CONTROL), status(oUNDEF), request(req), entry(e),
    asyncReqID(reqID), paramSize(size) {
        memcpy(param, prm, size);
    }

    // Result
    OVRControlMessage(OStatus st, OVRRequest req, void* prm, size_t size) :
    msgType(omsgVR_CONTROL), status(st), request(req), entry(),
    asyncReqID(0), paramSize(size) {
        memcpy(param, prm, size);
    }

    // Asynchronous Result
    OVRControlMessage(OStatus st, OVRRequest req,
                      void* prm, size_t size, OAsyncReqID reqID) :
    msgType(omsgVR_CONTROL), status(st), request(req), entry(),
    asyncReqID(reqID), paramSize(size) {
        memcpy(param, prm, size);
    }
};


struct OTimeMessage {
    OMessageType msgType;
    OTime        time;

    OTimeMessage() : msgType(omsgTIME) {}
    OTimeMessage(const OTime& t) : msgType(omsgTIME), time(t) {}

    OTimeMessage(time_t gmt, sbyte timeDif) : msgType(omsgTIME),
    time(gmt, timeDif) {}

    OTimeMessage(time_t gmt) : msgType(omsgTIME),
    time(gmt, otimeTIME_DIF_UNDEF) {}

    OTimeMessage(sbyte timeDif) : msgType(omsgTIME), time(0, timeDif) {}
};

struct OVolumeMessage {
    OMessageType msgType;
    byte volume;

    OVolumeMessage() : msgType(omsgVOLUME) {}
    OVolumeMessage(byte v) : msgType(omsgVOLUME), volume(v) {}
};


struct OPowerStatusMessage {
    OMessageType msgType;
    OPowerStatus powerStatus;

    OPowerStatusMessage() :
    msgType(omsgPOWER_STATUS) {}

    OPowerStatusMessage(const OPowerStatus& status) :
    msgType(omsgPOWER_STATUS), powerStatus(status) {}

    OPowerStatusMessage(longword robotStatus, word batteryStatus,
                        word remainingCapacity, word temperature,
                        word fullyChargedCapacity, word voltage, word current,
                        sbyte timeDif, word volume) :
    msgType(omsgPOWER_STATUS), powerStatus(robotStatus, batteryStatus,
                                           remainingCapacity, temperature,
                                           fullyChargedCapacity, voltage,
                                           current, timeDif, volume) {}
};


struct OMotorPowerMessage {
    OMessageType msgType;
    bool         motorPower;

    OMotorPowerMessage() :
    msgType(omsgMOTOR_POWER), motorPower(false) {}

    OMotorPowerMessage(bool mp) :
    msgType(omsgMOTOR_POWER), motorPower(mp) {}
};


struct OPlungerMessage {
    OMessageType msgType;
    bool         plunger;

    OPlungerMessage() :
    msgType(omsgPLUNGER), plunger(false) {}

    OPlungerMessage(bool plng) :
    msgType(omsgPLUNGER), plunger(plng) {}
};


struct OBmnModeMessage {
    OMessageType msgType;
    word         bmnMode;

    OBmnModeMessage() :
    msgType(omsgBMN_MODE), bmnMode(0) {}

    OBmnModeMessage(word mode) :
    msgType(omsgBMN_MODE), bmnMode(mode) {}
};


struct OBmnSpecialModeMessage {
    OMessageType msgType;
    byte         bmnSpecialMode;

    OBmnSpecialModeMessage() :
    msgType(omsgBMN_SPECIAL_MODE), bmnSpecialMode(0) {}

    OBmnSpecialModeMessage(byte specialMode) :
    msgType(omsgBMN_SPECIAL_MODE), bmnSpecialMode(specialMode) {}
};


struct OBootConditionMessage {
    OMessageType   msgType;
    OBootCondition bootCondition;

    OBootConditionMessage() :
    msgType(omsgBOOT_CONDITION) {}

    OBootConditionMessage(const OBootCondition& bootCondition) :
    msgType(omsgBOOT_CONDITION), bootCondition(bootCondition) {}
};


struct OAlarmMessage {
    OMessageType  msgType;
    OTime         alarmTime;
    OServiceEntry entry;

    OAlarmMessage() :
    msgType(omsgALARM) {}

    OAlarmMessage(const OServiceEntry& e) :
    msgType(omsgALARM), entry(e) {}

    OAlarmMessage(const OTime& time, const OServiceEntry& e) :
    msgType(omsgALARM), alarmTime(time), entry(e) {}
};


struct OObservePowerStatusMessage {
    OMessageType  msgType;
    OServiceEntry entry;
    OPowerStatus  notifyStatus;

    OObservePowerStatusMessage() : msgType(omsgOBSERVE_POWER_STATUS) {}

    OObservePowerStatusMessage(const OServiceEntry& e) :
    msgType(omsgOBSERVE_POWER_STATUS), entry(e) {}

    OObservePowerStatusMessage(const OServiceEntry& e,
                               const OPowerStatus& status) :
    msgType(omsgOBSERVE_POWER_STATUS), entry(e), notifyStatus(status) {}
};


struct OBmnDataStreamMessage {
    OMessageType   msgType;
    byte           dataType;
    MemoryRegionID memID;
    longword       offset;
    longword       length;
    OServiceEntry  entry;

    OBmnDataStreamMessage() : msgType(omsgBMN_DATA_STREAM) {}
    OBmnDataStreamMessage(byte dt, MemoryRegionID mid, longword ofst,
                          longword len, const OServiceEntry& e) :
    msgType(omsgBMN_DATA_STREAM), dataType(dt), memID(mid), offset(ofst),
    length(len), entry(e) {}
};


struct OStationRegisterMessage {
    OMessageType msgType;
    OStatus      status;
    byte         addr;
    longword     data;

    OStationRegisterMessage() : msgType(omsgSTATION_REGISTER) {}
    OStationRegisterMessage(byte ad, longword dt) :
    msgType(omsgSTATION_REGISTER), addr(ad), data(dt) {}
    OStationRegisterMessage(byte ad) :
    msgType(omsgSTATION_REGISTER), addr(ad), data(0) {}
};

#endif /* _OPENRMessages_h_DEFINED */
