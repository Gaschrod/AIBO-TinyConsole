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

#include <OPENR/OPENRAPI.h>
#include <OPENR/OSyslog.h>
#include <OPENR/core_macro.h>
#include "SoundAgent.h"

SoundAgent::SoundAgent() : soundAgentState(SAS_IDLE),
                           speakerID(oprimitiveID_UNDEF),
                           soundDataID(odesigndataID_UNDEF),
                           soundODA(), playingIndex(-1), playingWAV()
{
    for (int i = 0; i < SOUND_NUM_BUFFER; i++) region[i] = 0;
}

OStatus
SoundAgent::DoInit(const OSystemEvent& event)
{
    NEW_ALL_SUBJECT_AND_OBSERVER;
    REGISTER_ALL_ENTRY;
    SET_ALL_READY_AND_NOTIFY_ENTRY;

    OpenSpeaker();
    NewSoundVectorData();
    LoadODA();
    SetPowerAndVolume();

    return oSUCCESS;
}

OStatus
SoundAgent::DoStart(const OSystemEvent& event)
{
    soundAgentState = SAS_START;

    ENABLE_ALL_SUBJECT;
    ASSERT_READY_TO_ALL_OBSERVER;

    return oSUCCESS;
}

OStatus
SoundAgent::DoStop(const OSystemEvent& event)
{
    soundAgentState = SAS_IDLE;

    DISABLE_ALL_SUBJECT;
    DEASSERT_READY_TO_ALL_OBSERVER;

    return oSUCCESS;
}

OStatus
SoundAgent::DoDestroy(const OSystemEvent& event)
{
    OPENR::DeleteDesignData(soundDataID);

    DELETE_ALL_SUBJECT_AND_OBSERVER;

    return oSUCCESS;
}

void
SoundAgent::NotifyCommand(const ONotifyEvent& event)
{
    OSYSDEBUG(("SoundAgent::NotifyCommand()\n"));

    MoNetAgentCommand* cmd = (MoNetAgentCommand*)event.Data(0);
    
    if (soundAgentState == SAS_IDLE) {

        ; // do nothing
        
    } else if (soundAgentState == SAS_START) {

        byte* data = soundODA.GetData(cmd->index);
        if (data == 0 || cmd->agent != monetagentSOUND) {
            ReplyAgentResult(cmd->agent, cmd->index, monetINVALID_ARG);
            observer[event.ObsIndex()]->AssertReady();
            return;
        }

        MoNetStatus st = Play(cmd->index, cmd->syncKey, data);
        if (st != monetSUCCESS) {
            ReplyAgentResult(cmd->agent, cmd->index, st);
            observer[event.ObsIndex()]->AssertReady();
            return;
        }

        soundAgentState = SAS_PLAYING;
        observer[event.ObsIndex()]->AssertReady();

    } else if (soundAgentState == SAS_PLAYING) {

        ReplyAgentResult(cmd->agent, cmd->index, monetBUSY);
        observer[event.ObsIndex()]->AssertReady();

    }
}

void
SoundAgent::ReadyPlay(const OReadyEvent& event)
{
    if (soundAgentState == SAS_PLAYING) {

        RCRegion* rgn = FindFreeRegion();
        if (CopyWAVTo(rgn) == WAV_SUCCESS) {
            subject[sbjSpeaker]->SetData(rgn);
            subject[sbjSpeaker]->NotifyObservers();
        } else {
            ReplyAgentResult(monetagentSOUND, playingIndex, monetCOMPLETION);
            soundAgentState = SAS_START;
        }

    }
}

MoNetStatus
SoundAgent::Play(int index, OVRSyncKey syncKey, byte* data)
{
    OSYSDEBUG(("SoundAgent::Play()\n"));

    playingIndex = index;
    playingWAV.Set(data);
    
    OStatus result;
    if (playingWAV.GetSamplingRate() == 16000 && 
        playingWAV.GetBitsPerSample() == 16) {
        OPrimitiveControl_SpeakerSoundType soundType(ospksndMONO16K16B);
        result = OPENR::ControlPrimitive(speakerID,
                                         oprmreqSPEAKER_SET_SOUND_TYPE,
                                         &soundType, sizeof(soundType), 0, 0);
    } else {
        OPrimitiveControl_SpeakerSoundType soundType(ospksndMONO8K8B);
        result = OPENR::ControlPrimitive(speakerID,
                                         oprmreqSPEAKER_SET_SOUND_TYPE,
                                         &soundType, sizeof(soundType), 0, 0);
    }
    if (result != oSUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s %d",
                  "SoundAgent::Play()", 
                  "OPENR::ControlPrimitive(SPEAKER_SET_SOUND_TYPE) FAILED",
                  result));
    }

    RCRegion* rgn = FindFreeRegion();
    OSoundVectorData* soundVec = (OSoundVectorData*)rgn->Base();
    soundVec->vectorInfo.syncKey = syncKey;

    if (CopyWAVTo(rgn) == WAV_SUCCESS) {
        subject[sbjSpeaker]->SetData(rgn);
    } else {
        OSYSLOG1((osyslogERROR, "SoundAgent::Play() : INVALID WAV"));
        return monetINVALID_WAV;
    }
    
    rgn = FindFreeRegion();
    if (CopyWAVTo(rgn) == WAV_SUCCESS) {
        subject[sbjSpeaker]->SetData(rgn);
    }
    
    subject[sbjSpeaker]->NotifyObservers();
    return monetSUCCESS;
}

void
SoundAgent::OpenSpeaker()
{
    OStatus result;
    char design[orobotdesignNAME_MAX+1];

    result = OPENR::GetRobotDesign(design);
    if (result != oSUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s %d",
                  "SoundPlay::OpenSpeaker()",
                  "OPENR::GetRobotDesign() FAILED", result));
    }

    if (!strcmp(design, "ERS-7")) {
        result = OPENR::OpenPrimitive(SPEAKER_LOCATOR_ERS7, &speakerID);
    } else { // ERS-210 or ERS-220
        result = OPENR::OpenPrimitive(SPEAKER_LOCATOR, &speakerID);
    }

    if (result != oSUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s %d",
                  "SoundAgent::OpenSpeaker()",
                  "OPENR::OpenPrimitive() FAILED", result));
    }
}

void
SoundAgent::NewSoundVectorData()
{
    OStatus result;
    MemoryRegionID    soundVecDataID;
    OSoundVectorData* soundVecData;

    for (int i = 0; i < SOUND_NUM_BUFFER; i++) {

        result = OPENR::NewSoundVectorData(1, SOUND_UNIT_SIZE,
                                           &soundVecDataID, &soundVecData);
        if (result != oSUCCESS) {
            OSYSLOG1((osyslogERROR, "%s : %s %d",
                      "SoundAgent::NewSoundVectorData()",
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
SoundAgent::LoadODA()
{
    OStatus result;
    size_t size;
    byte*  addr;

    result = OPENR::FindDesignData(MONET_SOUND_KEYWORD,
                                   &soundDataID, &addr, &size);
    if (result != oSUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s %d",
                  "SoundAgent::LoadODA()",
                  "OPENR::FindDesignData() FAILED", result));
        return;
    }

    soundODA.Set(addr);
}

void
SoundAgent::SetPowerAndVolume()
{
    OSYSDEBUG(("SoundAgent::SetPowerAndVolume()\n"));

    OStatus result;

    result = OPENR::ControlPrimitive(speakerID,
                                     oprmreqSPEAKER_MUTE_ON, 0, 0, 0, 0);
    if (result != oSUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s %d",
                  "SoundAgent::SetPowerAndVolume()", 
                  "OPENR::ControlPrimitive(oprmreqSPEAKER_MUTE_ON) FAILED",
                  result));
    }

    result = OPENR::SetMotorPower(opowerON);
    if (result != oSUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s %d",
                  "SoundAgent::SetPowerAndVolume()", 
                  "OPENR::SetMotorPower() FAILED", result));
    }

    result = OPENR::ControlPrimitive(speakerID,
                                     oprmreqSPEAKER_MUTE_OFF, 0, 0, 0, 0);
    if (result != oSUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s %d",
                  "SoundAgent::SetPowerAndVolume()", 
                  "OPENR::ControlPrimitive(oprmreqSPEAKER_MUTE_OFF) FAILED",
                  result));
    }

    OSpeakerVolume vol;
    OVolumeSwitch volSW;
    result = OPENR::GetVolumeSwitch(&volSW);
    if (result != oSUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s %d",
                  "SoundAgent::SetPowerAndVolume()", 
                  "OPENR::GetVolumeSwitch() FAILED", result));
        volSW = ovolumeSW0;
    }

    OSYSDEBUG(("volSW is %d.\n", volSW));
    if (volSW == ovolumeSW0) {
        vol = ospkvolinfdB;
    } else if (volSW == ovolumeSW1) {
        vol = ospkvol25dB;
    } else if (volSW == ovolumeSW2) {
        vol = ospkvol18dB;
    } else { // volSW == ovolumeSW3
        vol = ospkvol10dB;
    }

    OPrimitiveControl_SpeakerVolume volume(vol);
    result = OPENR::ControlPrimitive(speakerID,
                                     oprmreqSPEAKER_SET_VOLUME,
                                     &volume, sizeof(volume), 0, 0);
    if (result != oSUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s %d",
                  "SoundAgent::SetPowerAndVolume()", 
                  "OPENR::ControlPrimitive(oprmreqSPEAKER_SET_VOLUME) FAILED",
                  result));
    }
}

WAVError
SoundAgent::CopyWAVTo(RCRegion* rgn)
{
    OSoundVectorData* soundVecData = (OSoundVectorData*)rgn->Base();
    return playingWAV.CopyTo(soundVecData);
}

RCRegion*
SoundAgent::FindFreeRegion()
{
    for (int i = 0; i < SOUND_NUM_BUFFER; i++) {
        if (region[i]->NumberOfReference() == 1) return region[i];
    }

    return 0;
}

void
SoundAgent::ReplyAgentResult(MoNetAgentID agt, int idx, MoNetStatus st)
{
    MoNetAgentResult result(agt, idx, st, monetpostureANY);
    subject[sbjResult]->SetData(&result, sizeof(result));
    subject[sbjResult]->NotifyObservers();
}
