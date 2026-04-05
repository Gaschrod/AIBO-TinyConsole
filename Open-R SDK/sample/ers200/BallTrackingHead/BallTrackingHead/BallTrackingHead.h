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

#ifndef BallTrackingHead_h_DEFINED
#define BallTrackingHead_h_DEFINED

#include <list>
using namespace std;

#include <OPENR/OObject.h>
#include <OPENR/OSubject.h>
#include <OPENR/OObserver.h>
#include <OPENR/ODataFormats.h>
#include <OPENR/OFbkImage.h>
#include <BallTrackingHeadData.h>
#include "def.h"

enum BallTrackingHeadState {
    BTHS_IDLE,
    BTHS_START,
    BTHS_HEAD_ZERO_POS,
    BTHS_LEGS_SLEEPING,
    BTHS_SEARCHING_BALL,
    BTHS_TRACKING_BALL
};

static const char* const FBK_LOCATOR = "PRM:/r1/c1/c2/c3/i1-FbkImageSensor:F1";
static const char* const JOINT_LOCATOR[] = {
    "PRM:/r1/c1-Joint2:j1",     // HEAD TILT
    "PRM:/r1/c1/c2-Joint2:j2"   // HEAD PAN
};

class BallTrackingHead : public OObject {
public:
    BallTrackingHead();
    virtual ~BallTrackingHead() {}

    OSubject*  subject[numOfSubject];
    OObserver* observer[numOfObserver];

    virtual OStatus DoInit   (const OSystemEvent& event);
    virtual OStatus DoStart  (const OSystemEvent& event);
    virtual OStatus DoStop   (const OSystemEvent& event);
    virtual OStatus DoDestroy(const OSystemEvent& event);
    
    void NotifyImage(const ONotifyEvent& event);
    void NotifySensor(const ONotifyEvent& event);
    void NotifyMovingHeadResult(const ONotifyEvent& event);
    void NotifyMovingLegsResult(const ONotifyEvent& event);
    void NotifyLostFoundSoundResult(const ONotifyEvent& event);

private:
    static const size_t      NUM_COMMAND_VECTOR = 4;
    static const size_t      NUM_SENSOR_VECTOR  = 2;
    static const size_t      NUM_JOINTS         = 2;
    static const size_t      NUM_FRAMES         = 5;
    static const int         TILT_INDEX         = 0;
    static const int         PAN_INDEX          = 1;
    static const byte        BALL_THRESHOLD     = 10;
    static const int         FOUND_THRESHOLD    = 2;
    static const int         LOST_THRESHOLD     = 5;
    static const OCdtChannel BALL_CDT_CHAN      = ocdtCHANNEL0;
    static const double      FIELD_VIEW_H       = 57.6;
    static const double      FIELD_VIEW_V       = 47.8;

    void OpenPrimitives();
    void NewCommandVectorData();
    void PlaySound(BallTrackingHeadCommandType type);

    void SetCdtVectorDataOfPinkBall();
    bool CentroidAndDeltaAngle(const OFbkImage& cdtImage,
                               int* xcentroid, int* ycentroid,
                               double* delta_pan, double* delta_pan);

    void InitSensorIndex(OSensorFrameVectorData* sensorVec);
    void GetPanTiltAngle(longword frameNum, double* pan, double* tilt);

    void SearchBall();
    void TrackBall(longword frameNum, double delta_pan, double delta_pan);
    void MoveHead(double s_pan, double s_tilt, double e_pan, double e_tilt,
                  double *r_pan, double *r_tilt,
                  double pan_limit, double tilt_limit);
    void SetJointValue(RCRegion* rgn, int idx, double start, double end);
    RCRegion* FindFreeRegion();

    BallTrackingHeadState   ballTrackingHeadState;
    OPrimitiveID            fbkID;
    OPrimitiveID            jointID[NUM_JOINTS];
    RCRegion*               region[NUM_COMMAND_VECTOR];
    bool                    initSensorIndex;
    int                     sensoridx[NUM_JOINTS];
    list<RCRegion*>         sensorRegions;
    double                  lastRefPan;
    double                  lastRefTilt;
};

#endif // BallTrackingHead_h_DEFINED
