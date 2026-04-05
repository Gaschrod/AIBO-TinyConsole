//
// MWInformationTypes.h
//
// Copyright 2000 Sony Corporation
//

#ifndef _OPENR_MWInformationTypes_h_DEFINED_
#define _OPENR_MWInformationTypes_h_DEFINED_

#include <Types.h>

typedef longword MWIID;
const MWIID     mwiidINVALID    = 0xFFFFFFFF;

typedef byte SensorType;       // Sensor Type : Camera, Mic, PSD,,,
typedef byte AttachedParts;    // Attached Position : head,leg,body,,
typedef byte AttachedPosition; // Parts Position : 1,2,3,4,,

#endif  /* #ifndef _OPENR_MWInformationTypes_h_DEFINED_ */
