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

#include <stdlib.h>
#include <OPENR/OPENRAPI.h>
#include <OPENR/OSyslog.h>
#include <OPENR/core_macro.h>
#include "SoundRec.h"

SoundRec::SoundRec(void) : soundRecState(SRS_IDLE),
                           soundBuf(0), soundBufPtr(0)
{
}

OStatus
SoundRec::DoInit(const OSystemEvent& event)
{
    NEW_ALL_SUBJECT_AND_OBSERVER;
    REGISTER_ALL_ENTRY;
    SET_ALL_READY_AND_NOTIFY_ENTRY;

    char design[orobotdesignNAME_MAX+1];
    OStatus result = OPENR::GetRobotDesign(design);
    if (result != oSUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s %d",
                  "SoundRec::DoInit()",
                  "OPENR::GetRobotDesign() FAILED", result));
    }

    if (!strcmp(design, "ERS-210") || !strcmp(design, "ERS-220")) {
        ChooseMic(oprmreqMIC_OMNI); // oprmreqMIC_OMNI or oprmreqMIC_UNI
    }

    NewSoundBuffer();

    return oSUCCESS;
}

OStatus
SoundRec::DoStart(const OSystemEvent& event)
{
    OSYSPRINT(("START RECORDING (about 16sec) ... \n"));
    if (soundBuf != 0) soundRecState = SRS_START;

    ENABLE_ALL_SUBJECT;
    ASSERT_READY_TO_ALL_OBSERVER;

    return oSUCCESS;
}

OStatus
SoundRec::DoStop(const OSystemEvent& event)
{
    soundRecState = SRS_IDLE;

    DISABLE_ALL_SUBJECT;
    DEASSERT_READY_TO_ALL_OBSERVER;

    return oSUCCESS;
}

OStatus
SoundRec::DoDestroy(const OSystemEvent& event)
{
    DeleteSoundBuffer();
    DELETE_ALL_SUBJECT_AND_OBSERVER;
    return oSUCCESS;
}

void
SoundRec::Notify(const ONotifyEvent& event)
{
    // 
    // SoundRec::Notify() is invoked : every 16ms (ERS-7)
    //                               : every 32ms (ERS-210/220)
    //

    if (soundRecState == SRS_IDLE) {

        ; // do nothing
        return;

    } else {

        OSoundVectorData* soundVecData = (OSoundVectorData*)event.Data(0);
        int size = CopyToSoundBuffer(soundVecData);

        if (size < SOUND_BUFFER_SIZE) {
            observer[event.ObsIndex()]->AssertReady(event.SenderID());
        } else {
            OSYSPRINT(("SAVE SOUNDREC.WAV. WAIT ... "));
            SaveSoundBufferAsWAV("/MS/OPEN-R/MW/DATA/P/SOUNDREC.WAV");
            OSYSPRINT(("DONE\n"));
        }

    }
}

void
SoundRec::ChooseMic(OPrimitiveRequest req)
{
    OStatus result;
    OPrimitiveID micID;
    
    result = OPENR::OpenPrimitive(MIC_LOCATOR, &micID);
    if (result != oSUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s %d",
                  "SoundRec::ChooseMic()",
                  "OPENR::OpenPrimitive() FAILED", result));
        return;
    }

    result = OPENR::ControlPrimitive(micID, req, 0, 0, 0, 0);
    if (result != oSUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s %d",
                  "SoundRec::ChooseMic()",
                  "OPENR::ControlPrimitive() FAILED", result));
    }
}

void
SoundRec::NewSoundBuffer()
{
    soundBuf = (byte*)malloc(SOUND_BUFFER_SIZE);
    if (soundBuf == 0) {
        OSYSLOG1((osyslogERROR,
                  "SoundRec::NewSoundBuffer() : malloc() failed"));
    }

    soundBufPtr = soundBuf;    
}

void
SoundRec::DeleteSoundBuffer()
{
    if (soundBuf != 0) {
        free(soundBuf);
        soundBuf = soundBufPtr = 0;
    }
}

int
SoundRec::CopyToSoundBuffer(OSoundVectorData* soundVecData)
{
    OSoundInfo* info = soundVecData->GetInfo(0);
    byte* data = soundVecData->GetData(0);
    memcpy(soundBufPtr, data, info->dataSize);
    soundBufPtr += info->dataSize;

    return (int)(soundBufPtr - soundBuf);
}

void
SoundRec::SaveSoundBufferAsWAV(char* path)
{
    longword lwd;
    word     wd;

    FILE* fp = fopen(path, "w");
    if (fp == 0) {
        OSYSLOG1((osyslogERROR,
                  "SoundRec::SaveSoundBufferAsWAV() : can't open %s", path));
        return;
    }

    fwrite("RIFF", 1, 4, fp);

    // RIFF data size
    lwd = WAVE_HEADER_SIZE + SOUND_BUFFER_SIZE;
    fwrite(&lwd, sizeof(lwd), 1, fp);

    // "WAVE"
    fwrite("WAVE", 1, 4, fp);

    // "fmt "
    fwrite("fmt ", 1, 4, fp);

    // Length of the fmt chunk
    lwd = FMTSIZE_WITHOUT_EXTINFO;
    fwrite(&lwd, sizeof(lwd), 1, fp);

    // WAVE File Encoding Tag (PCM)
    wd = 0x01;
    fwrite(&wd, sizeof(wd), 1, fp);

    // Channels (STEREO)
    wd = 0x02;
    fwrite(&wd, sizeof(wd), 1, fp);

    // Sampling Rate (16kHz)
    lwd = 16000;
    fwrite(&lwd, sizeof(lwd), 1, fp);

    // bytes/sec (16KHz * 16bits * STEREO)
    lwd = 16000 * 2 * 2;
    fwrite(&lwd, sizeof(lwd), 1, fp);

    // Block Align (channels * bits/sample / 8)
    wd = 0x04;
    fwrite(&wd, sizeof(wd), 1, fp);

    // bits/sample
    wd = 16;
    fwrite(&wd, sizeof(wd), 1, fp);

    // "data"
    fwrite("data", 1, 4, fp);

    // data size
    lwd = SOUND_BUFFER_SIZE;
    fwrite(&lwd, sizeof(lwd), 1, fp);

    // data
    size_t nwrite = fwrite(soundBuf, 1, lwd, fp);
    if (nwrite != lwd) {
        OSYSLOG1((osyslogERROR, "%s : %s %d",
                  "SoundRec::SaveSoundBufferAsWAV()",
                  "fwrite() FAILED", nwrite));
    }

    fclose(fp);
}
