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

#ifndef SoundPlay_h_DEFINED
#define SoundPlay_h_DEFINED

#include <OPENR/OObject.h>
#include <OPENR/OSubject.h>
#include <OPENR/OObserver.h>
#include "WAV.h"
#include "def.h"

enum SoundPlayState {
    SPS_IDLE,
    SPS_START
};

static const char* const SPEAKER_LOCATOR = "PRM:/r1/c1/c2/c3/s1-Speaker:S1";
static const char* const SPEAKER_LOCATOR_ERS7 = "PRM:/s1-Speaker:S1";

class SoundPlay : public OObject {
public:
    SoundPlay();
    virtual ~SoundPlay() {}

    OSubject*  subject[numOfSubject];
    OObserver* observer[numOfObserver];

    virtual OStatus DoInit   (const OSystemEvent& event);
    virtual OStatus DoStart  (const OSystemEvent& event);
    virtual OStatus DoStop   (const OSystemEvent& event);
    virtual OStatus DoDestroy(const OSystemEvent& event);

    void Ready(const OReadyEvent& event);

private:
    static const size_t SOUND_NUM_BUFFER = 2;

    void      OpenSpeaker();
    void      NewSoundVectorData();
    void      LoadWAV();
    void      SetPowerAndVolume();
    WAVError  CopyWAVTo(RCRegion* region);
    RCRegion* FindFreeRegion();

    SoundPlayState soundPlayState;
    OPrimitiveID   speakerID;
    ODesignDataID  wavID;
    WAV            wav;
    RCRegion*      region[SOUND_NUM_BUFFER];
};

#endif // SoundPlay_h_DEFINED
