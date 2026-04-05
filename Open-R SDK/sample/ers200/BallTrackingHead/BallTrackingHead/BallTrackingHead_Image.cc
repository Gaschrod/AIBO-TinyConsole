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

#include <OPENR/OPENRAPI.h>
#include <OPENR/OSyslog.h>
#include "BallTrackingHead.h"

void
BallTrackingHead::NotifyImage(const ONotifyEvent& event)
{
    static int found = 0;
    static int lost = 0;

    int    xc, yc;              // centroid
    double d_pan, d_tilt;       // delta angle

    if (ballTrackingHeadState == BTHS_IDLE) return; // do nothing

    OFbkImageVectorData* imageVec = (OFbkImageVectorData*)event.Data(0);

    OFbkImageInfo* info = imageVec->GetInfo(ofbkimageLAYER_C);
    byte*          data = imageVec->GetData(ofbkimageLAYER_C);
    OFbkImage cdtImage(info, data, ofbkimageBAND_CDT);

    if (ballTrackingHeadState == BTHS_SEARCHING_BALL) {

        if (cdtImage.ColorFrequency(BALL_CDT_CHAN) >= BALL_THRESHOLD) {
            found++;
        } else {
            found = 0;
            SearchBall();
        }
        
        if (found == FOUND_THRESHOLD) {
            OSYSPRINT(("### BALL FOUND ### \n"));
            PlaySound(BTHCMD_PLAY_FOUND_SOUND);
            found = 0;
            CentroidAndDeltaAngle(cdtImage, &xc, &yc, &d_pan, &d_tilt);
            TrackBall(info->frameNumber, d_pan, d_tilt);
        }

    } else if (ballTrackingHeadState == BTHS_TRACKING_BALL) {

        if (cdtImage.ColorFrequency(BALL_CDT_CHAN) >= BALL_THRESHOLD) {
            lost = 0; 
            CentroidAndDeltaAngle(cdtImage, &xc, &yc, &d_pan, &d_tilt);
            TrackBall(info->frameNumber, d_pan, d_tilt);
        } else {
            lost++;
        }

        if (lost == LOST_THRESHOLD) {
            OSYSPRINT(("### BALL LOST ### \n"));
            PlaySound(BTHCMD_PLAY_LOST_SOUND);
            SearchBall();
            lost = 0;
        }
    }
    
    observer[event.ObsIndex()]->AssertReady();
}

void
BallTrackingHead::SetCdtVectorDataOfPinkBall()
{
    OSYSDEBUG(("BallTrackingHead::SetCdtVectorDataOfPinkBall()\n"));

    OStatus result;
    MemoryRegionID  cdtVecID;
    OCdtVectorData* cdtVec;
    OCdtInfo*       cdt;

    result = OPENR::NewCdtVectorData(&cdtVecID, &cdtVec);
    if (result != oSUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s %d",
                  "ImageObserver::SetCdtVectorDataOfPinkBall()",
                  "OPENR::NewCdtVectorData() FAILED", result));
        return;
    }

    cdtVec->SetNumData(1);

    cdt = cdtVec->GetInfo(0);
    cdt->Init(fbkID, BALL_CDT_CHAN);

    //
    // cdt->Set(Y_segment, Cr_max,  Cr_min, Cb_max, Cb_min)
    //
    cdt->Set( 0, 230, 150, 190, 120);
    cdt->Set( 1, 230, 150, 190, 120);
    cdt->Set( 2, 230, 150, 190, 120);
    cdt->Set( 3, 230, 150, 190, 120);
    cdt->Set( 4, 230, 150, 190, 120);
    cdt->Set( 5, 230, 150, 190, 120);
    cdt->Set( 6, 230, 150, 190, 120);
    cdt->Set( 7, 230, 150, 190, 120);
    cdt->Set( 8, 230, 150, 190, 120);
    cdt->Set( 9, 230, 150, 190, 120);
    cdt->Set(10, 230, 150, 190, 120);
    cdt->Set(11, 230, 150, 190, 120);
    cdt->Set(12, 230, 150, 190, 120);
    cdt->Set(13, 230, 150, 190, 120);
    cdt->Set(14, 230, 150, 190, 120);
    cdt->Set(15, 230, 150, 190, 120);
    cdt->Set(16, 230, 150, 190, 120);
    cdt->Set(17, 230, 150, 190, 120);
    cdt->Set(18, 230, 150, 190, 120);
    cdt->Set(19, 230, 150, 190, 120);
    cdt->Set(20, 230, 160, 190, 120);
    cdt->Set(21, 230, 160, 190, 120);
    cdt->Set(22, 230, 160, 190, 120);
    cdt->Set(23, 230, 160, 190, 120);
    cdt->Set(24, 230, 160, 190, 120);
    cdt->Set(25, 230, 160, 190, 120);
    cdt->Set(26, 230, 160, 190, 120);
    cdt->Set(27, 230, 160, 190, 120);
    cdt->Set(28, 230, 160, 190, 120);
    cdt->Set(29, 230, 160, 190, 120);
    cdt->Set(30, 230, 160, 190, 120);
    cdt->Set(31, 230, 160, 190, 120);

    result = OPENR::SetCdtVectorData(cdtVecID);
    if (result != oSUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s %d",
                  "ImageObserver::SetCdtVectorDataOfPinkBall()",
                  "OPENR::SetCdtVectorData() FAILED", result));
    }

    result = OPENR::DeleteCdtVectorData(cdtVecID);
    if (result != oSUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s %d",
                  "ImageObserver::SetCdtVectorDataOfPinkBall()",
                  "OPENR::DeleteCdtVectorData() FAILED", result));
    }
}

bool
BallTrackingHead::CentroidAndDeltaAngle(const OFbkImage& cdtImage,
                                        int* xcentroid, int* ycentroid,
                                        double* delta_pan, double* delta_tilt)
{
    int w     = cdtImage.Width();
    int h     = cdtImage.Height();
    byte* ptr = cdtImage.Pointer();

    int xsum   = 0;
    int ysum    = 0;
    int npixels = 0;
    
    // Last line is not used because of TagInfo.
    for (int y = 0; y < h-1; y++) {
        for (int x = 0; x < w; x++) {
            if (*ptr++ & BALL_CDT_CHAN) {
                xsum += x;
                ysum += y;
                npixels++;
            }
        }
    }

    if (npixels != 0) {
        *xcentroid  = xsum / npixels;
        *ycentroid  = ysum / npixels;
        *delta_pan  = -1.0 * FIELD_VIEW_H * (*xcentroid - w/2.0) / w;
        *delta_tilt = -1.0 * FIELD_VIEW_V * (*ycentroid - h/2.0) / h;
        return true;
    } else {
        *xcentroid  = 0;
        *ycentroid  = 0;
        *delta_pan  = 0.0;
        *delta_tilt = 0.0;
        return false;
    }
}
