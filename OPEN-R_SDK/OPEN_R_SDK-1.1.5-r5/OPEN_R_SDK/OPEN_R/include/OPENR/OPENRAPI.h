//
// OPENRAPI.h
//
// Copyright 2000,2001,2002,2003,2004 Sony Corporation
//

#ifndef _OPENRAPI_h_DEFINED
#define _OPENRAPI_h_DEFINED

#include <OPENR/OPENR.h>
#include <OPENR/ODataFormats.h>
#include <OPENR/OPrimitiveControl.h>
#include <OPENR/OPower.h>
#include <OPENR/MWInformationTypes.h>

namespace OPENR {
    //
    // OPEN-R API for Level2
    //
    OStatus OpenPrimitive(const char* locator, OPrimitiveID* primitiveID);
    OStatus ClosePrimitive(OPrimitiveID primitiveID);
    OStatus ControlPrimitive(OPrimitiveID primitiveID,
                             OPrimitiveRequest request,
                             void* param, size_t paramSize,
                             void* result, size_t resultSize);

    OStatus NewCommandVectorData(size_t numCommands,
                                 MemoryRegionID* memID,
                                 OCommandVectorData** baseAddr);
    OStatus DeleteCommandVectorData(MemoryRegionID memID);

    OStatus NewSoundVectorData(size_t numSounds,
                               size_t dataSize,
                               MemoryRegionID* memID,
                               OSoundVectorData** baseAddr);
    OStatus DeleteSoundVectorData(MemoryRegionID memID);

    OStatus EnableJointGain(OPrimitiveID primitiveID);
    OStatus DisableJointGain(OPrimitiveID primitiveID);
    OStatus SetJointGain(OPrimitiveID primitiveID,
                         word pg, word ig, word dg, word ps, word is, word ds);
    OStatus RegisterDefaultJointGain(OPrimitiveID primitiveID,
                                     word pg, word ig, word dg,
                                     word ps, word is, word ds);
    OStatus GetJointGain(OPrimitiveID primitiveID,
                         word* pg, word* ig, word* dg,
                         word* ps, word* is, word* ds);
    OStatus GetDefaultJointGain(OPrimitiveID primitiveID,
                                word* pg, word* ig, word* dg,
                                word* ps, word* is, word* ds);
    OStatus SetDefaultJointGain(OPrimitiveID primitiveID);
    OStatus GetJointValue(OPrimitiveID primitiveID, OJointValue* value);
    OStatus GetSensorValue(OPrimitiveID primitiveID, OSensorValue* value);
    
    OStatus NewSyncKey(OVRSyncKey* key);
    OStatus CancelSyncKey(OVRSyncKey key);
    OStatus DivideSyncKey(OVRSyncKey key, OVRSyncKey* key1, OVRSyncKey* key2);

    OStatus NewCdtVectorData(MemoryRegionID* memID, OCdtVectorData** baseAddr);
    OStatus DeleteCdtVectorData(MemoryRegionID memID);
    OStatus SetCdtVectorData(MemoryRegionID memID);

    OStatus SetMotorPower(OPower power);
    OStatus Shutdown(const OBootCondition& bootCondition);

    OStatus GetBootCondition(OBootCondition* bootCondition);
    OStatus GetPowerStatus(OPowerStatus* powerStatus);
    OStatus ObservePowerStatus(const OPowerStatus& notifyStatus,
                               const OServiceEntry& entry);
    OStatus UnobservePowerStatus(const OServiceEntry& entry);

    OStatus FindDesignData(const char* keyword,
                           ODesignDataID* dataID, byte** data, size_t* size);
    OStatus DeleteDesignData(ODesignDataID dataID);
    OStatus GetRobotDesign(char* robotDesign);
    OStatus GetMemoryStickStatus(OMemoryStickStatus* status);
    OStatus Fatal(OFatal fatal);
    OStatus SetTime(const OTime& time);
    OStatus GetTime(OTime* time);
    OStatus SetTimeDifference(sbyte timeDifference);
    OStatus GetTimeDifference(sbyte* timeDifference);
    OStatus SetVolumeSwitch(OVolumeSwitch volSW);
    OStatus GetVolumeSwitch(OVolumeSwitch* volSW);

    //
    // OPEN-R API for Level1
    //
    OStatus RequireMWInfo(MWIID mwiID);
    OStatus CancelMWInfo(MWIID mwiID);
    OStatus IsMWInfoRequired(MWIID mwiID, bool* required);

    //
    // DON'T USE 
    //
    OStatus NewSensorFrameVectorData(size_t numSensors,
                                     MemoryRegionID* memID,
                                     OSensorFrameVectorData** baseAddr);
    OStatus DeleteSensorFrameVectorData(MemoryRegionID memID);
    OStatus NewFbkImageVectorData(MemoryRegionID* memID,
                                  OFbkImageVectorData** baseAddr);
    OStatus DeleteFbkImageVectorData(MemoryRegionID memID);
}

#endif /* _OPENRAPI_h_DEFINED */
