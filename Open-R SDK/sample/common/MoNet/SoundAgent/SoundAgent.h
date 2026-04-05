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

#ifndef SoundAgent_h_DEFINED
#define SoundAgent_h_DEFINED

#include <OPENR/OObject.h>
#include <OPENR/OSubject.h>
#include <OPENR/OObserver.h>
#include <OPENR/ODataArchive.h>
#include <MoNetData.h>
#include <ODA.h>
#include "WAV.h"
#include "def.h"

enum SoundAgentState {
    SAS_IDLE,
    SAS_START,
    SAS_PLAYING
};

static const char* const SPEAKER_LOCATOR = "PRM:/r1/c1/c2/c3/s1-Speaker:S1";
static const char* const SPEAKER_LOCATOR_ERS7 = "PRM:/s1-Speaker:S1";

class SoundAgent : public OObject {
public:
    SoundAgent();
    virtual ~SoundAgent() {}

    OSubject*  subject[numOfSubject];
    OObserver* observer[numOfObserver];

    virtual OStatus DoInit   (const OSystemEvent& event);
    virtual OStatus DoStart  (const OSystemEvent& event);
    virtual OStatus DoStop   (const OSystemEvent& event);
    virtual OStatus DoDestroy(const OSystemEvent& event);

    void NotifyCommand(const ONotifyEvent& event);
    void ReadyPlay(const OReadyEvent& event);

private:
    MoNetStatus Play(int index, OVRSyncKey syncKey, byte* data);
    void        OpenSpeaker();
    void        NewSoundVectorData();
    void        LoadODA();
    void        SetPowerAndVolume();
    WAVError    CopyWAVTo(RCRegion* region);
    RCRegion*   FindFreeRegion();
    void ReplyAgentResult(MoNetAgentID agt, int idx, MoNetStatus st);

    //
    //  8KHz  8bits MONO (8  * 1 * 1 * 32 =  256)
    // 16KHz 16bits MONO (16 * 2 * 1 * 32 = 1024)
    //
    static const size_t SOUND_UNIT_SIZE  = 1024;
    static const size_t SOUND_NUM_BUFFER = 4;

    SoundAgentState soundAgentState;
    OPrimitiveID    speakerID;
    ODesignDataID   soundDataID;
    ODA             soundODA;
    int             playingIndex;
    WAV             playingWAV;
    RCRegion*       region[SOUND_NUM_BUFFER];
};

#endif // SoundAgent_h_DEFINED
