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

#include <OPENR/OPENRAPI.h>
#include <OPENR/OSyslog.h>
#include <OPENR/core_macro.h>
#include "SoundPlay.h"

SoundPlay::SoundPlay() : soundPlayState(SPS_IDLE),
                         speakerID(oprimitiveID_UNDEF),
                         wavID(odesigndataID_UNDEF), wav()
{
    for (int i = 0; i < SOUND_NUM_BUFFER; i++) region[i] = 0;
}

OStatus
SoundPlay::DoInit(const OSystemEvent& event)
{
    OSYSDEBUG(("SoundPlay::DoInit()\n"));

    NEW_ALL_SUBJECT_AND_OBSERVER;
    REGISTER_ALL_ENTRY;
    SET_ALL_READY_AND_NOTIFY_ENTRY;

    LoadWAV();

    OpenSpeaker();
    NewSoundVectorData();
    SetPowerAndVolume();

    return oSUCCESS;
}

OStatus
SoundPlay::DoStart(const OSystemEvent& event)
{
    OSYSDEBUG(("SoundPlay::DoStart()\n"));

    for (int i = 0; i < SOUND_NUM_BUFFER; i++) {
        if (CopyWAVTo(region[i]) == WAV_SUCCESS)
            subject[sbjSpeaker]->SetData(region[i]);
    }
    subject[sbjSpeaker]->NotifyObservers();
    soundPlayState = SPS_START;

    ENABLE_ALL_SUBJECT;
    ASSERT_READY_TO_ALL_OBSERVER;

    return oSUCCESS;
}

OStatus
SoundPlay::DoStop(const OSystemEvent& event)
{
    OSYSDEBUG(("SoundPlay::DoStop()\n"));

    soundPlayState = SPS_IDLE;
    
    DISABLE_ALL_SUBJECT;
    DEASSERT_READY_TO_ALL_OBSERVER;

    return oSUCCESS;
}

OStatus
SoundPlay::DoDestroy(const OSystemEvent& event)
{
    OPENR::DeleteDesignData(wavID);
    DELETE_ALL_SUBJECT_AND_OBSERVER;
    return oSUCCESS;
}

void
SoundPlay::Ready(const OReadyEvent& event)
{
    OSYSDEBUG(("SoundPlay::Ready()\n"));

    if (soundPlayState == SPS_START) {

        RCRegion* rgn = FindFreeRegion();
        if (CopyWAVTo(rgn) == WAV_SUCCESS) {
            subject[sbjSpeaker]->SetData(rgn);
            subject[sbjSpeaker]->NotifyObservers();
        }

    }
}

void
SoundPlay::OpenSpeaker()
{
    OStatus result;
    char design[orobotdesignNAME_MAX+1];

    result = OPENR::GetRobotDesign(design);
    if (result != oSUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s %d",
                  "SoundPlay::OpenSpeaker()",
                  "OPENR::GetRobotDesign() FAILED", result));
    }

    if (strcmp(design, "ERS-7") == 0) {
        result = OPENR::OpenPrimitive(SPEAKER_LOCATOR_ERS7, &speakerID);
    } else { // ERS-210 or ERS-220
        result = OPENR::OpenPrimitive(SPEAKER_LOCATOR, &speakerID);
    }

    if (result != oSUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s %d",
                  "SoundPlay::OpenSpeaker()",
                  "OPENR::OpenPrimitive() FAILED", result));
    }
}

void
SoundPlay::NewSoundVectorData()
{
    OStatus result;
    MemoryRegionID    soundVecDataID;
    OSoundVectorData* soundVecData;

    size_t soundUnitSize = wav.GetSoundUnitSize();

    for (int i = 0; i < SOUND_NUM_BUFFER; i++) {

        result = OPENR::NewSoundVectorData(1, soundUnitSize,
                                           &soundVecDataID, &soundVecData);
        if (result != oSUCCESS) {
            OSYSLOG1((osyslogERROR, "%s : %s %d",
                      "SoundPlay::NewSoundVectorData()",
                      "OPENR::NewSoundVectorData() FAILED", result));
            return;
        }

        soundVecData->SetNumData(1);
        soundVecData->GetInfo(0)->Set(odataSOUND_VECTOR,
                                      speakerID, soundUnitSize);

        region[i] = new RCRegion(soundVecData->vectorInfo.memRegionID,
                                 soundVecData->vectorInfo.offset,
                                 (void*)soundVecData,
                                 soundVecData->vectorInfo.totalSize);
    }
}

void
SoundPlay::LoadWAV()
{
    byte*  addr;
    size_t size;

    OStatus result = OPENR::FindDesignData("SOUNDPLAY_WAV",
                                           &wavID, &addr, &size);
    if (result != oSUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s %d",
                  "SoundPlay::LoadWAV()",
                  "OPENR::FindDesignData() FAILED", result));
        return;
    }

    WAVError error = wav.Set(addr);
    if (error != WAV_SUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s %d",
                  "SoundPlay::LoadWAV()",
                  "wav.Set() FAILED", error));
    }
}

void
SoundPlay::SetPowerAndVolume()
{
    OStatus result;

    result = OPENR::ControlPrimitive(speakerID,
                                     oprmreqSPEAKER_MUTE_ON, 0, 0, 0, 0);
    if (result != oSUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s %d",
                  "SoundPlay::SetPowerAndVolume()", 
                  "OPENR::ControlPrimitive(SPEAKER_MUTE_ON) FAILED", result));
    }

    result = OPENR::SetMotorPower(opowerON);
    if (result != oSUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s %d",
                  "SoundPlay::SetPowerAndVolume()", 
                  "OPENR::SetMotorPower() FAILED", result));
    }

    result = OPENR::ControlPrimitive(speakerID,
                                     oprmreqSPEAKER_MUTE_OFF, 0, 0, 0, 0);
    if (result != oSUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s %d",
                  "SoundPlay::SetPowerAndVolume()", 
                  "OPENR::ControlPrimitive(SPEAKER_MUTE_OFF) FAILED", result));
    }

    OPrimitiveControl_SpeakerVolume volume(ospkvol10dB);
    result = OPENR::ControlPrimitive(speakerID,
                                     oprmreqSPEAKER_SET_VOLUME,
                                     &volume, sizeof(volume), 0, 0);
    if (result != oSUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s %d",
                  "SoundPlay::SetPowerAndVolume()", 
                  "OPENR::ControlPrimitive(SPEAKER_SET_VOLUME) FAILED",
                  result));
    }

    if (wav.GetSamplingRate() == 16000 && wav.GetBitsPerSample() == 16) {
        OPrimitiveControl_SpeakerSoundType soundType(ospksndMONO16K16B);
        result = OPENR::ControlPrimitive(speakerID,
                                         oprmreqSPEAKER_SET_SOUND_TYPE,
                                         &soundType, sizeof(soundType), 0, 0);
        if (result != oSUCCESS) {
            OSYSLOG1((osyslogERROR, "%s : %s %d",
                      "SoundPlay::SetPowerAndVolume()", 
                      "OPENR::ControlPrimitive(SPEAKER_SET_SOUND_TYPE) FAILED",
                      result));
        }
    }
}

WAVError
SoundPlay::CopyWAVTo(RCRegion* rgn)
{
    OSoundVectorData* soundVecData = (OSoundVectorData*)rgn->Base();
    
    WAVError error = wav.CopyTo(soundVecData);
    if (error != WAV_SUCCESS) {
        wav.Rewind();
        error = wav.CopyTo(soundVecData);
    }

    return error;
}

RCRegion*
SoundPlay::FindFreeRegion()
{
    for (int i = 0; i < SOUND_NUM_BUFFER; i++) {
        if (region[i]->NumberOfReference() == 1) return region[i];
    }

    return 0;
}
