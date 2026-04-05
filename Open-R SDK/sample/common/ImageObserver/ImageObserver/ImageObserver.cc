//
// Copyright 2002,2003,2004 Sony Corporation 
//
// Permission to use, copy, modify, and redistribute this software for
// non-commercial use is hereby granted.
//
// This software is provided "as is" without warranty of any kind,
// either expressed or implied, including but not limited to the
// implied warranties of fitness for a particular purpose.
//

#include <OPENR/ODataFormats.h>
#include <OPENR/OFbkImage.h>
#include <OPENR/OPENRAPI.h>
#include <OPENR/OSyslog.h>
#include <OPENR/core_macro.h>
#include "ImageObserver.h"
#include "BMP.h"

ImageObserver::ImageObserver() : imageObserverState(IOS_IDLE),
                                 fbkID(oprimitiveID_UNDEF)
{
}

OStatus
ImageObserver::DoInit(const OSystemEvent& event)
{
    NEW_ALL_SUBJECT_AND_OBSERVER;
    REGISTER_ALL_ENTRY;
    SET_ALL_READY_AND_NOTIFY_ENTRY;

    OpenPrimitive();
    SetCdtVectorDataOfPinkBall();

    return oSUCCESS;
}

OStatus
ImageObserver::DoStart(const OSystemEvent& event)
{
    imageObserverState = IOS_START;

    ENABLE_ALL_SUBJECT;
    ASSERT_READY_TO_ALL_OBSERVER;

    return oSUCCESS;
}    

OStatus
ImageObserver::DoStop(const OSystemEvent& event)
{
    imageObserverState = IOS_IDLE;

    DISABLE_ALL_SUBJECT;
    DEASSERT_READY_TO_ALL_OBSERVER;

    return oSUCCESS;
}

OStatus
ImageObserver::DoDestroy(const OSystemEvent& event)
{
    DELETE_ALL_SUBJECT_AND_OBSERVER;
    return oSUCCESS;
}

void
ImageObserver::Notify(const ONotifyEvent& event)
{
    static int counter = 0;

    if (imageObserverState == IOS_IDLE) {
        return; // do nothing
    }

    OFbkImageVectorData* fbkImageVectorData 
        = (OFbkImageVectorData*)event.Data(0);

    BMP bmp;
    if (counter == 0) {

        OSYSPRINT(("SAVE LAYER_H.BMP ... "));
        bmp.SaveYCrCb2RGB("/MS/OPEN-R/MW/DATA/P/LAYER_H.BMP", 
                          fbkImageVectorData, ofbkimageLAYER_H);
        OSYSPRINT(("DONE\n"));

        OSYSPRINT(("SAVE LAYER_H.RAW ... "));
        SaveRawData("/MS/OPEN-R/MW/DATA/P/LAYER_H.RAW", 
                    fbkImageVectorData, ofbkimageLAYER_H);
        OSYSPRINT(("DONE\n"));

        OSYSPRINT(("SAVE LAYER_HR.BMP ... "));
        byte* image;
        int width, height;
        ReconstructImage(fbkImageVectorData,
                         ofbkimageLAYER_H, &image, &width, &height);
        bmp.SaveRaw2Gray("/MS/OPEN-R/MW/DATA/P/LAYER_HR.BMP",
                         image, width, height, 0);
        FreeImage(image);
        OSYSPRINT(("DONE\n"));

    } else if (counter == 1) {

        OSYSPRINT(("SAVE LAYER_M.BMP ... "));
        bmp.SaveYCrCb2RGB("/MS/OPEN-R/MW/DATA/P/LAYER_M.BMP", 
                          fbkImageVectorData, ofbkimageLAYER_M);
        OSYSPRINT(("DONE\n"));

    } else if (counter == 2) {

        OSYSPRINT(("SAVE LAYER_L.BMP ... "));
        bmp.SaveYCrCb2RGB("/MS/OPEN-R/MW/DATA/P/LAYER_L.BMP", 
                          fbkImageVectorData, ofbkimageLAYER_L);
        OSYSPRINT(("DONE\n"));

    } else if (counter == 3) {
        
        OSYSPRINT(("SAVE LAYER_C ... "));
        bmp.SaveLayerC("/MS/OPEN-R/MW/DATA/P/LAYER_C", fbkImageVectorData);
        OSYSPRINT(("DONE\n"));- 

            OSYSPRINT(("SAVE LAYER_M2.BMP ... "));
            bmp.SaveYCrCb2RGB("/MS/OPEN-R/MW/DATA/P/LAYER_M2.BMP", 
                              fbkImageVectorData, ofbkimageLAYER_M);
            OSYSPRINT(("DONE\n"));

    } else {

        PrintTagInfo(fbkImageVectorData);

    }

    counter++;
    observer[event.ObsIndex()]->AssertReady(event.SenderID());
}

void
ImageObserver::OpenPrimitive()
{
    OStatus result = OPENR::OpenPrimitive(FBK_LOCATOR, &fbkID);
    if (result != oSUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s %d",
                  "ImageObserver::OpenPrimitive()",
                  "OPENR::OpenPrimitive() FAILED", result));
    }
}

void
ImageObserver::SetCdtVectorDataOfPinkBall()
{
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
    cdt->Init(fbkID, ocdtCHANNEL0);

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

void
ImageObserver::PrintTagInfo(OFbkImageVectorData* imageVec)
{
    OFbkImageInfo* info = imageVec->GetInfo(ofbkimageLAYER_H);
    byte*          data = imageVec->GetData(ofbkimageLAYER_H);
    OFbkImage yImage(info, data, ofbkimageBAND_Y);
    
    OSYSPRINT(("FrameNumber %x FieldCounter %x ColorFreq ",
               imageVec->GetInfo(0)->frameNumber, yImage.FieldCounter()));

    OSYSPRINT(("%d ", yImage.ColorFrequency(ocdtCHANNEL0)));
    OSYSPRINT(("%d ", yImage.ColorFrequency(ocdtCHANNEL1)));
    OSYSPRINT(("%d ", yImage.ColorFrequency(ocdtCHANNEL2)));
    OSYSPRINT(("%d ", yImage.ColorFrequency(ocdtCHANNEL3)));
    OSYSPRINT(("%d ", yImage.ColorFrequency(ocdtCHANNEL4)));
    OSYSPRINT(("%d ", yImage.ColorFrequency(ocdtCHANNEL5)));
    OSYSPRINT(("%d ", yImage.ColorFrequency(ocdtCHANNEL6)));
    OSYSPRINT(("%d ", yImage.ColorFrequency(ocdtCHANNEL7)));
    OSYSPRINT(("\n"));
}

void
ImageObserver::SaveRawData(char* path,
                           OFbkImageVectorData* imageVec, OFbkImageLayer layer)
{
    OFbkImageInfo* info = imageVec->GetInfo(layer);
    byte*          data = imageVec->GetData(layer);

    size_t size;
    if (layer == ofbkimageLAYER_C) {

        size = info->width * info->height;

    } else { // ofbkimageLAYER_H or ofbkimageLAYER_M or ofbkimageLAYER_L

        if (info->type == odataFBK_YCrCb) {
            size = 3 * info->width * info->height;
        } else if (info->type == odataFBK_YCrCb_HPF) {
            size = 6 * info->width * info->height;
        }
        
    }

    FILE* fp = fopen(path, "w");
    if (fp == 0) {
        OSYSLOG1((osyslogERROR, "can't open %s", path));
        return;
    }
    fwrite(data, 1, size, fp);
    fclose(fp);
}

bool
ImageObserver::ReconstructImage(OFbkImageVectorData* imageVec,
                                OFbkImageLayer layer,
                                byte** image, int* width, int* height)
{
    *image = 0;
    *width = *height = 0;
    if (layer == ofbkimageLAYER_C) return false;

    OFbkImageInfo* info = imageVec->GetInfo(layer);
    byte*          data = imageVec->GetData(layer);

    OFbkImage yLLImg(info, data, ofbkimageBAND_Y); // Y_LL
    OFbkImage yLHImg(info, data, ofbkimageBAND_Y_LH);
    OFbkImage yHLImg(info, data, ofbkimageBAND_Y_HL);
    OFbkImage yHHImg(info, data, ofbkimageBAND_Y_HH);

    int w = yLLImg.Width();
    int h = yLLImg.Height();

    byte* img = (byte*)malloc((2*w)*(2*h));
    if (img == 0) return false;

    for (int y = 0; y < h; y++) {

        for (int x = 0; x < w; x++) {
            //
            // yLH, yHL, yHH : offset binary [0, 255] -> signed int [-128, 127]
            //
            int yLL = (int)yLLImg.Pixel(x, y);
            int yLH = (int)yLHImg.Pixel(x, y) - 128;
            int yHL = (int)yHLImg.Pixel(x, y) - 128;
            int yHH = (int)yHHImg.Pixel(x, y) - 128;

            int a = yLL + yLH + yHL + yHH; // ypix11
            int b = 2 * (yLL + yLH);       // ypix11 + ypix01
            int c = 2 * (yLL + yHL);       // ypix11 + ypix10
            int d = 2 * (yLL + yHH);       // ypix11 + ypix00
            
            byte ypix00 = ClipRange(d - a);
            byte ypix10 = ClipRange(c - a);
            byte ypix01 = ClipRange(b - a);
            byte ypix11 = ClipRange(a);
            
            PutPixel(img, 2*w, 2*x,   2*y,   ypix00);
            PutPixel(img, 2*w, 2*x+1, 2*y,   ypix10);
            PutPixel(img, 2*w, 2*x,   2*y+1, ypix01);
            PutPixel(img, 2*w, 2*x+1, 2*y+1, ypix11);
        }

    }

    *image  = img;
    *width  = 2 * w;
    *height = 2 * h;

    return true;
}

void
ImageObserver::FreeImage(byte* image)
{
    if (image != 0) free(image);
}
