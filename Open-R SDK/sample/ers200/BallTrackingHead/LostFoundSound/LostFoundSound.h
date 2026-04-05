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

#ifndef LostFoundSound_h_DEFINED
#define LostFoundSound_h_DEFINED

#include <OPENR/OObject.h>
#include <OPENR/OSubject.h>
#include <OPENR/OObserver.h>
#include <BallTrackingHeadData.h>
#include "WAV.h"
#include "def.h"

enum LostFoundSoundState {
    LFSS_IDLE,
    LFSS_START,
    LFSS_PLAYING
};

static const char* const SPEAKER_LOCATOR = "PRM:/r1/c1/c2/c3/s1-Speaker:S1";
static const char* const FOUND_SOUND = "FOUND_SOUND";
static const char* const LOST_SOUND  = "LOST_SOUND";

class LostFoundSound : public OObject {
public:
    LostFoundSound();
    virtual ~LostFoundSound() {}

    OSubject*  subject[numOfSubject];
    OObserver* observer[numOfObserver];

    virtual OStatus DoInit   (const OSystemEvent& event);
    virtual OStatus DoStart  (const OSystemEvent& event);
    virtual OStatus DoStop   (const OSystemEvent& event);
    virtual OStatus DoDestroy(const OSystemEvent& event);

    void NotifyCommand(const ONotifyEvent& event);
    void ReadyPlay(const OReadyEvent& event);

private:
    void      Play(BallTrackingHeadCommand* cmd);
    void      OpenSpeaker();
    void      NewSoundVectorData();
    void      LoadWAV();
    void      SetPowerAndVolume();
    WAVError  CopyWAVTo(RCRegion* region);
    RCRegion* FindFreeRegion();
    bool      IsAllRegionFree();

    //
    // 8KHz 8bits MONO (8 * 1 * 1 * 32 = 256)
    //
    static const size_t SOUND_UNIT_SIZE  = 256; // bytes / 32ms
    static const size_t SOUND_NUM_BUFFER = 2;

    LostFoundSoundState lfsoundState;
    OPrimitiveID   speakerID;
    ODesignDataID  foundSoundID;
    WAV            foundWAV;
    ODesignDataID  lostSoundID;
    WAV            lostWAV;
    WAV*           playingWAV;
    RCRegion*      region[SOUND_NUM_BUFFER];
};

#endif // LostFoundSound_h_DEFINED
