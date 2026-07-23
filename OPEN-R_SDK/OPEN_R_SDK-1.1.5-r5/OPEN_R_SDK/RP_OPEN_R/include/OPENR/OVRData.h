//
// OVRData.h
//
// Copyright 1997,1998,1999,2000 Sony Corporation
//

#ifndef _OVRData_h_DEFINED
#define _OVRData_h_DEFINED

#include <OPENR/ODataFormats.h>

const size_t ovrdataMAX_SIZE = 8;

struct OVRData {		          // 16 bytes (total)
    OStatus         status;	      //  4 bytes
    ODataType       type;	      //  4 bytes
    MemoryRegionID  dataVectorID; //  4 bytes
    size_t          offset;       //  4 bytes

    OVRData() : status(oUNDEF), type(odataUNDEF), dataVectorID(0), offset(0) {}
    OVRData(ODataType t, MemoryRegionID id) :
    status(oUNDEF), type(t), dataVectorID(id), offset(0) {}
    OVRData(OStatus s, ODataType t, MemoryRegionID id) :
    status(s), type(t), dataVectorID(id), offset(0) {}

    void Set(ODataType t, MemoryRegionID id) {
	status = oUNDEF; type = t; dataVectorID = id; offset = 0;
    }
    void Set(OStatus s, ODataType t, MemoryRegionID id) {
	status = s; type = t; dataVectorID = id; offset = 0;
    }
};

#endif /* _OVRData_h_DEFINED */
