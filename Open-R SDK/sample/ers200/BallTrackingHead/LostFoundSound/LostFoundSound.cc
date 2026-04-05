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
#include <OPENR/core_macro.h>
#include "LostFoundSound.h"

LostFoundSound::LostFoundSound() : lfsoundState(LFSS_IDLE),
                                   speakerID(oprimitiveID_UNDEF),
                                   foundSoundID(odesigndataID_UNDEF),
                                   foundWAV(),
                                   lostSoundID(odesigndataID_UNDEF),
                                   lostWAV(), playingWAV()
{
    for (int i = 0; i < SOUND_NUM_BUFFER; i++) region[i] = 0;
}

OStatus
LostFoundSound::DoInit(const OSystemEvent& event)
{
    OSYSDEBUG(("LostFoundSound::DoInit()\n"));

    NEW_ALL_SUBJECT_AND_OBSERVER;
    REGISTER_ALL_ENTRY;
    SET_ALL_READY_AND_NOTIFY_ENTRY;

    OpenSpeaker();
    NewSoundVectorData();
    LoadWAV();
    SetPowerAndVolume();

    return oSUCCESS;
}

OStatus
LostFoundSound::DoStart(const OSystemEvent& event)
{
    OSYSDEBUG(("LostFoundSound::DoStart()\n"));

    lfsoundState = LFSS_START;

    ENABLE_ALL_SUBJECT;
    ASSERT_READY_TO_ALL_OBSERVER;

    return oSUCCESS;
}

OStatus
LostFoundSound::DoStop(const OSystemEvent& event)
{
    OSYSDEBUG(("LostFoundSound::DoStop()\n"));

    lfsoundState = LFSS_IDLE;

    DISABLE_ALL_SUBJECT;
    DEASSERT_READY_TO_ALL_OBSERVER;

    return oSUCCESS;
}

OStatus
LostFoundSound::DoDestroy(const OSystemEvent& event)
{
    OPENR::DeleteDesignData(foundSoundID);
    OPENR::DeleteDesignData(lostSoundID);
    DELETE_ALL_SUBJECT_AND_OBSERVER;
    return oSUCCESS;
}

void
LostFoundSound::NotifyCommand(const ONotifyEvent& event)
{
    OSYSDEBUG(("LostFoundSound::NotifyCommand()\n"));

    if (lfsoundState != LFSS_START) {
        BallTrackingHeadResult result(BTH_BUSY);
        subject[sbjResult]->SetData(&result, sizeof(result));
        subject[sbjResult]->NotifyObservers();
        observer[event.ObsIndex()]->AssertReady();
        return;
    }

    BallTrackingHeadCommand* cmd = (BallTrackingHeadCommand*)event.Data(0);
    if (cmd->type == BTHCMD_PLAY_FOUND_SOUND ||
        cmd->type == BTHCMD_PLAY_LOST_SOUND) {
        
        Play(cmd);
        lfsoundState = LFSS_PLAYING;
            
    } else {

        BallTrackingHeadResult result(BTH_INVALID_ARG);
        subject[sbjResult]->SetData(&result, sizeof(result));
        subject[sbjResult]->NotifyObservers();

    }

    observer[event.ObsIndex()]->AssertReady();
}

void
LostFoundSound::ReadyPlay(const OReadyEvent& event)
{
    OSYSDEBUG(("LostFoundSound::ReadyPlay()\n"));

    if (lfsoundState == LFSS_PLAYING) {

        RCRegion* rgn = FindFreeRegion();
        if (CopyWAVTo(rgn) == WAV_SUCCESS) {
            subject[sbjPlay]->SetData(rgn);
            subject[sbjPlay]->NotifyObservers();
        } else {
            if (IsAllRegionFree() == true) {
                BallTrackingHeadResult result(BTH_SUCCESS);
                subject[sbjResult]->SetData(&result, sizeof(result));
                subject[sbjResult]->NotifyObservers();
                lfsoundState = LFSS_START;
            }
        }

    }
}

void
LostFoundSound::Play(BallTrackingHeadCommand* cmd)
{
    OSYSDEBUG(("LostFoundSound::Play()\n"));

    if (cmd->type == BTHCMD_PLAY_FOUND_SOUND) {
        playingWAV = &foundWAV;
    } else { // cmd->type == BTHCMD_PLAY_LOST_SOUND
        playingWAV = &lostWAV;        
    }
    playingWAV->Rewind();

    for (int i = 0; i < SOUND_NUM_BUFFER; i++) {
        if (CopyWAVTo(region[i]) == WAV_SUCCESS)
            subject[sbjPlay]->SetData(region[i]);
    }
    subject[sbjPlay]->NotifyObservers();
}

void
LostFoundSound::OpenSpeaker()
{
    OStatus result = OPENR::OpenPrimitive(SPEAKER_LOCATOR, &speakerID);
    if (result != oSUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s %d",
                  "LostFoundSound::OpenSpeaker()",
                  "OPENR::OpenPrimitive() FAILED", result));
    }
}

void
LostFoundSound::NewSoundVectorData()
{
    OStatus result;
    MemoryRegionID    soundVecDataID;
    OSoundVectorData* soundVecData;

    for (int i = 0; i < SOUND_NUM_BUFFER; i++) {

        result = OPENR::NewSoundVectorData(1, SOUND_UNIT_SIZE,
                                           &soundVecDataID, &soundVecData);
        if (result != oSUCCESS) {
            OSYSLOG1((osyslogERROR, "%s : %s %d",
                      "LostFoundSound::NewSoundVectorData()",
                      "OPENR::NewSoundVectorData() FAILED", result));
            return;
        }

        soundVecData->SetNumData(1);
        soundVecData->GetInfo(0)->Set(odataSOUND_VECTOR,
                                      speakerID, SOUND_UNIT_SIZE);

        region[i] = new RCRegion(soundVecData->vectorInfo.memRegionID,
                                 soundVecData->vectorInfo.offset,
                                 (void*)soundVecData,
                                 soundVecData->vectorInfo.totalSize);
    }
}

void
LostFoundSound::LoadWAV()
{
    OStatus result;
    size_t size;
    byte*  addr;

    result = OPENR::FindDesignData(FOUND_SOUND,
                                   &foundSoundID, &addr, &size);
    if (result != oSUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s %d",
                  "LostFoundSound::LoadWAV()",
                  "OPENR::FindDesignData() FAILED", result));
    }

    foundWAV.Set(addr);

    result = OPENR::FindDesignData(LOST_SOUND,
                                   &lostSoundID, &addr, &size);
    if (result != oSUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s %d",
                  "LostFoundSound::LoadWAV()",
                  "OPENR::FindDesignData() FAILED", result));
    }

    lostWAV.Set(addr);
}

void
LostFoundSound::SetPowerAndVolume()
{
    OStatus result;

    result = OPENR::ControlPrimitive(speakerID,
                                     oprmreqSPEAKER_MUTE_ON, 0, 0, 0, 0);
    if (result != oSUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s %d",
                  "LostFoundSound::SetPowerAndVolume()", 
                  "OPENR::ControlPrimitive(oprmreqSPEAKER_MUTE_ON) FAILED",
                  result));
    }

    result = OPENR::SetMotorPower(opowerON);
    if (result != oSUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s %d",
                  "LostFoundSound::SetPowerAndVolume()", 
                  "OPENR::SetMotorPower() FAILED", result));
    }

    result = OPENR::ControlPrimitive(speakerID,
                                     oprmreqSPEAKER_MUTE_OFF, 0, 0, 0, 0);
    if (result != oSUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s %d",
                  "LostFoundSound::SetPowerAndVolume()", 
                  "OPENR::ControlPrimitive(oprmreqSPEAKER_MUTE_OFF) FAILED",
                  result));
    }

    OPrimitiveControl_SpeakerVolume volume(ospkvol10dB);
    result = OPENR::ControlPrimitive(speakerID,
                                     oprmreqSPEAKER_SET_VOLUME,
                                     &volume, sizeof(volume), 0, 0);
    if (result != oSUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s %d",
                  "LostFoundSound::SetPowerAndVolume()", 
                  "OPENR::ControlPrimitive(oprmreqSPEAKER_SET_VOLUME) FAILED",
                  result));
    }
}

WAVError
LostFoundSound::CopyWAVTo(RCRegion* rgn)
{
    OSoundVectorData* soundVecData = (OSoundVectorData*)rgn->Base();
    return playingWAV->CopyTo(soundVecData);
}

RCRegion*
LostFoundSound::FindFreeRegion()
{
    for (int i = 0; i < SOUND_NUM_BUFFER; i++) {
        if (region[i]->NumberOfReference() == 1) return region[i];
    }

    return 0;
}

bool
LostFoundSound::IsAllRegionFree()
{
    for (int i = 0; i < SOUND_NUM_BUFFER; i++) {
        if (region[i]->NumberOfReference() > 1) return false;
    }

    return true;
}
