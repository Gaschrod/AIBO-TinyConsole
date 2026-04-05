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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <OPENR/ODataFormats.h>
#include <OPENR/OFbkImage.h>
#include <OPENR/OPENRAPI.h>
#include <OPENR/OSyslog.h>
#include <OPENR/core_macro.h>
#include "ImageObserver.h"
#include "BMP.h"

ImageObserver::ImageObserver() : imageObserverState(IOS_IDLE),
                                 fbkID(oprimitiveID_UNDEF),
                                 tinswID(oprimitiveID_UNDEF),
                                 backswID(oprimitiveID_UNDEF)
{
}

OStatus
ImageObserver::DoInit(const OSystemEvent& event)
{
    NEW_ALL_SUBJECT_AND_OBSERVER;
    REGISTER_ALL_ENTRY;
    SET_ALL_READY_AND_NOTIFY_ENTRY;

    char design[orobotdesignNAME_MAX+1];
    OStatus result = OPENR::GetRobotDesign(design);
    if (result != oSUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s %d",
                  "ImageObserver::DoInit()",
                  "OPENR::GetRobotDesign() FAILED", result));
    }

    if (!strcmp(design, "ERS-210") || !strcmp(design, "ERS-220")) {
        robot = ROBOT_ERS200;
    } else if (!strcmp(design, "ERS-7")) {
        robot = ROBOT_ERS7;
    } else {
        robot = ROBOT_UNDEF;
        OSYSLOG1((osyslogERROR,
                  "ImageObserver::DoInit() UNKNOWN ROBOT DESIGN"));
    }

    OpenPrimitive();

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

    OFbkImageVectorData* fbkImageVectorData 
        = (OFbkImageVectorData*)event.Data(0);

    if (imageObserverState == IOS_IDLE) {
        return; // do nothing
    }

    if (robot == ROBOT_ERS200) {
        OSensorValue val1, val2;
        OPENR::GetSensorValue(tinswID, &val1);
        OPENR::GetSensorValue(backswID, &val2);
        if (val1.value == oswitchON || val2.value == oswitchON) {
            while (ExistData(counter) == true) counter++;
            SaveData(counter, fbkImageVectorData);
        }
    } else if (robot == ROBOT_ERS7) {
        OSensorValue val1, valf, valm, valr;
        OPENR::GetSensorValue(tinswID,  &val1);
        OPENR::GetSensorValue(backf7ID, &valf);
        OPENR::GetSensorValue(backm7ID, &valm);
        OPENR::GetSensorValue(backr7ID, &valr);
        if (val1.value == oswitchON ||
            valf.value >= 12 || valm.value >= 14 || valr.value >= 14) {
            while (ExistData(counter) == true) counter++;
            SaveData(counter, fbkImageVectorData);
        }
    }

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

    result = OPENR::OpenPrimitive(TINSW_LOCATOR, &tinswID);
    if (result != oSUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s %d",
                  "ImageObserver::OpenPrimitive()",
                  "OPENR::OpenPrimitive() FAILED", result));
    }

    if (robot == ROBOT_ERS200) {
        result = OPENR::OpenPrimitive(BACKSW_LOCATOR, &backswID);
        if (result != oSUCCESS) {
            OSYSLOG1((osyslogERROR, "%s : %s %d",
                      "ImageObserver::OpenPrimitive()",
                      "OPENR::OpenPrimitive() FAILED", result));
        }
    } else if (robot == ROBOT_ERS7) {
        result = OPENR::OpenPrimitive(BACKF7_LOCATOR, &backf7ID);
        if (result != oSUCCESS) {
            OSYSLOG1((osyslogERROR, "%s : %s %d",
                      "ImageObserver::OpenPrimitive()",
                      "OPENR::OpenPrimitive() FAILED", result));
        }
        result = OPENR::OpenPrimitive(BACKM7_LOCATOR, &backm7ID);
        if (result != oSUCCESS) {
            OSYSLOG1((osyslogERROR, "%s : %s %d",
                      "ImageObserver::OpenPrimitive()",
                      "OPENR::OpenPrimitive() FAILED", result));
        }
        result = OPENR::OpenPrimitive(BACKR7_LOCATOR, &backr7ID);
        if (result != oSUCCESS) {
            OSYSLOG1((osyslogERROR, "%s : %s %d",
                      "ImageObserver::OpenPrimitive()",
                      "OPENR::OpenPrimitive() FAILED", result));
        }
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
ImageObserver::ExistData(int serialNumber)
{
    char name[128];
    struct stat st;

    sprintf(name, "/MS/OPEN-R/MW/DATA/P/RGBH%04d.BMP", serialNumber);
    if (stat(name, &st) == 0) return true;

    sprintf(name, "/MS/OPEN-R/MW/DATA/P/LAYH%04d.RAW", serialNumber);
    if (stat(name, &st) == 0) return true;

    sprintf(name, "/MS/OPEN-R/MW/DATA/P/LAYM%04d.RAW", serialNumber);
    if (stat(name, &st) == 0) return true;

    sprintf(name, "/MS/OPEN-R/MW/DATA/P/LAYL%04d.RAW", serialNumber);
    if (stat(name, &st) == 0) return true;

    return false;
}

void
ImageObserver::SaveData(int serialNumber, OFbkImageVectorData* imageVec)
{
    char name[128];
    BMP bmp;

    sprintf(name, "/MS/OPEN-R/MW/DATA/P/RGBH%04d.BMP", serialNumber);
    OSYSPRINT(("writing %s ...\n", name));
    bmp.SaveYCrCb2RGB(name, imageVec, ofbkimageLAYER_H);

    sprintf(name, "/MS/OPEN-R/MW/DATA/P/LAYH%04d.RAW", serialNumber);
    OSYSPRINT(("writing %s ...\n", name));
    SaveRawData(name, imageVec, ofbkimageLAYER_H);

    sprintf(name, "/MS/OPEN-R/MW/DATA/P/LAYM%04d.RAW", serialNumber);
    OSYSPRINT(("writing %s ...\n", name));
    SaveRawData(name, imageVec, ofbkimageLAYER_M);

    sprintf(name, "/MS/OPEN-R/MW/DATA/P/LAYL%04d.RAW", serialNumber);
    OSYSPRINT(("writing %s ...\n", name));
    SaveRawData(name, imageVec, ofbkimageLAYER_L);

    OSYSPRINT(("done.\n"));
}
