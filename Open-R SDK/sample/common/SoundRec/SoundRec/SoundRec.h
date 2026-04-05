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

#ifndef SoundRec_h_DEFINED
#define SoundRec_h_DEFINED

#include <OPENR/OObject.h>
#include <OPENR/OSubject.h>
#include <OPENR/OObserver.h>
#include <OPENR/ODataFormats.h>
#include "def.h"

enum SoundRecState {
    SRS_IDLE,
    SRS_START
};

static const char* const MIC_LOCATOR = "PRM:/r1/c1/c2/c3/m1-Mic:M1";

class SoundRec : public OObject {
public:
    SoundRec();
    virtual ~SoundRec() {}

    OSubject*  subject[numOfSubject];
    OObserver* observer[numOfObserver];

    virtual OStatus DoInit   (const OSystemEvent& event);
    virtual OStatus DoStart  (const OSystemEvent& event);
    virtual OStatus DoStop   (const OSystemEvent& event);
    virtual OStatus DoDestroy(const OSystemEvent& event);

    void Notify(const ONotifyEvent& event);

private:
    void ChooseMic(OPrimitiveRequest req);
    void NewSoundBuffer();
    void DeleteSoundBuffer();
    int  CopyToSoundBuffer(OSoundVectorData* soundVecData);
    void SaveSoundBufferAsWAV(char* path);

    //
    // 16KHz 16bits STEREO 
    // 
    // 16 * 2 * 2 =   64 bytes /  1 ms
    // 64 * 32    = 2048 bytes / 32 ms
    // 
    // 2048 * 512 = 1048576 bytes
    // 32ms * 512 = 16384 ms
    //
    static const size_t SOUND_BUFFER_SIZE = 1048576;
    static const size_t WAVE_HEADER_SIZE  = 4 + 8 + 16 + 8; 
    static const size_t FMTSIZE_WITHOUT_EXTINFO = 16;

    SoundRecState  soundRecState;
    byte*          soundBuf;
    byte*          soundBufPtr;
};

#endif  // SoundRec_h_DEFINED
