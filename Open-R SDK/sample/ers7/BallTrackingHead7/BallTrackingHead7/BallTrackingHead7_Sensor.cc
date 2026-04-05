//
// Copyright 2002,2004 Sony Corporation 
//
// Permission to use, copy, modify, and redistribute this software for
// non-commercial use is hereby granted.
//
// This software is provided "as is" without warranty of any kind,
// either expressed or implied, including but not limited to the
// implied warranties of fitness for a particular purpose.
//

#include <OPENR/OSyslog.h>
#include "BallTrackingHead7.h"

void
BallTrackingHead7::NotifySensor(const ONotifyEvent& event)
{
    RCRegion* rgn = event.RCData(0);

    if (initSensorIndex == false) {
        OSensorFrameVectorData* sv = (OSensorFrameVectorData*)rgn->Base();
        InitSensorIndex(sv);
        initSensorIndex = true;
    }

    if (sensorRegions.size() == NUM_SENSOR_VECTOR) {
        sensorRegions.front()->RemoveReference();
        sensorRegions.pop_front();
    }
    rgn->AddReference();
    sensorRegions.push_back(rgn);

    observer[event.ObsIndex()]->AssertReady();
}

void
BallTrackingHead7::InitSensorIndex(OSensorFrameVectorData* sensorVec)
{
    for (int i = 0; i < NUM_JOINTS; i++) {
        for (int j = 0; j < sensorVec->vectorInfo.numData; j++) {
            OSensorFrameInfo* info = sensorVec->GetInfo(j);
            if (info->primitiveID == jointID[i]) {
                sensoridx[i] = j;
                OSYSDEBUG(("[%2d] %s\n",
                           sensoridx[i], JOINT_LOCATOR[i]));
                break;
            }
        }
    }
}

void
BallTrackingHead7::GetPanTiltAngle(longword frameNum,
                                   double* pan, double* tilt)
{
    list<RCRegion*>::iterator iter = sensorRegions.begin();
    list<RCRegion*>::iterator last = sensorRegions.end();
    OSensorFrameVectorData* sv;
    OSensorFrameInfo* info;
    OSensorFrameData* data;

    while (iter != last) {

        sv = (OSensorFrameVectorData*)(*iter)->Base();
        info = sv->GetInfo(sensoridx[PAN_INDEX]);
        longword fn = info->frameNumber;

        for (int i = 0; i < info->numFrames; i++) {

            if (fn == frameNum) {

                data = sv->GetData(sensoridx[PAN_INDEX]);
                *pan = degrees((double)data->frame[i].value / 1000000.0);

                data = sv->GetData(sensoridx[TILT2_INDEX]);
                *tilt = degrees((double)data->frame[i].value / 1000000.0);

                return;
            }

            fn++;
            if (fn > oframeMAX_NUMBER) fn = 1;
        }
        
        ++iter;
    }

    //
    // If sensor data at frameNum is not found, 
    // return latest sensor data.
    //
    RCRegion* rgn = sensorRegions.back();
    sv = (OSensorFrameVectorData*)rgn->Base();
    info = sv->GetInfo(sensoridx[PAN_INDEX]);

    data = sv->GetData(sensoridx[PAN_INDEX]);
    *pan = degrees((double)data->frame[info->numFrames-1].value / 1000000.0);

    data = sv->GetData(sensoridx[TILT2_INDEX]);
    *tilt = degrees((double)data->frame[info->numFrames-1].value / 1000000.0);
}
