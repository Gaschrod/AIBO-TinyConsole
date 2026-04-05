//
// Copyright 2002 Sony Corporation 
//
// Permission to use, copy, modify, and redistribute this software for
// non-commercial use is hereby granted.
//
// This software is provided "as is" without warranty of any kind,
// either expressed or implied, including but not limited to the
// implied warranties of fitness for a particular purpose.
//

#include <math.h>
#include <OPENR/OPENRAPI.h>
#include <OPENR/OUnits.h>
#include <OPENR/OSyslog.h>
#include <OPENR/ODebug.h>
#include "BallTrackingHead.h"

void
BallTrackingHead::SearchBall()
{
    static double phase      = 0.0;
    const  double deltaPhase = 0.03;
    const  double amplitude  = 70.0; // degrees
    
    OSYSDEBUG(("BallTrackingHead::SearchBall()\n"));
    if (ballTrackingHeadState != BTHS_SEARCHING_BALL) {
        ballTrackingHeadState = BTHS_SEARCHING_BALL;
        
        if (lastRefPan < 0.0) {
            phase = lastRefPan / amplitude;
        } else {
            phase = M_PI - lastRefPan / amplitude;
        }
    }
    
    const double panDeltaLimit  = deltaPhase * amplitude; // degrees / 32ms
    const double tiltDeltaLimit = 1.0;                    // degrees / 32ms
    
    double pan  = amplitude * sin(phase);
    double tilt = 0.0;
    phase += deltaPhase;
    
    MoveHead(lastRefPan, lastRefTilt,
             pan, tilt,
             &lastRefPan, &lastRefTilt,
             panDeltaLimit, tiltDeltaLimit );
}

void
BallTrackingHead::TrackBall(longword frameNum,
                            double delta_pan, double delta_tilt)
{
    if (ballTrackingHeadState == BTHS_SEARCHING_BALL) {
        ballTrackingHeadState = BTHS_TRACKING_BALL;
    }

    const double panDeltaLimit  = 5.0; // degrees / 32ms
    const double tiltDeltaLimit = 5.0; // degrees / 32ms
    
    double pan, tilt;
    GetPanTiltAngle(frameNum, &pan, &tilt);
    MoveHead(lastRefPan, lastRefTilt,
             pan + delta_pan, tilt + delta_tilt,
             &lastRefPan, &lastRefTilt,
             panDeltaLimit, tiltDeltaLimit );
}

void
BallTrackingHead::MoveHead(double s_pan, double s_tilt, 
                           double e_pan, double e_tilt,
                           double *r_pan, double *r_tilt,
                           double pan_limit, double tilt_limit)
{
    RCRegion* rgn = FindFreeRegion();
    
    if (e_pan > s_pan + pan_limit) {
        e_pan = s_pan + pan_limit;
    }else
    if (e_pan < s_pan - pan_limit) {
        e_pan = s_pan - pan_limit;
    }
    *r_pan = e_pan;
    
    if (e_tilt > s_tilt + tilt_limit) {
        e_tilt = s_tilt + tilt_limit;
    }else
    if (e_tilt < s_tilt - tilt_limit) {
        e_tilt = s_tilt - tilt_limit;
    }
    *r_tilt = e_tilt;
    
    SetJointValue(rgn, PAN_INDEX, s_pan, e_pan);
    SetJointValue(rgn, TILT_INDEX, s_tilt, e_tilt);
    subject[sbjJoint]->SetData(rgn);
    subject[sbjJoint]->NotifyObservers();
}

void
BallTrackingHead::SetJointValue(RCRegion* rgn,
                                int idx, double start, double end)
{
    OCommandVectorData* cmdVecData = (OCommandVectorData*)rgn->Base();

    OCommandInfo* info = cmdVecData->GetInfo(idx);
    info->Set(odataJOINT_COMMAND2, jointID[idx], NUM_FRAMES);

    OCommandData* data = cmdVecData->GetData(idx);
    OJointCommandValue2* jval = (OJointCommandValue2*)data->value;

    double delta = end - start;
    for (int i = 0; i < NUM_FRAMES; i++) {
        double dval = start + (delta * i) / (double)NUM_FRAMES;
        jval[i].value = oradians(dval);
    }
}

RCRegion*
BallTrackingHead::FindFreeRegion()
{
    for (int i = 0; i < NUM_COMMAND_VECTOR; i++) {
        if (region[i]->NumberOfReference() == 1) return region[i];
    }

    return 0;
}
