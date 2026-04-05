//
// Copyright 2002,2003 Sony Corporation 
//
// Permission to use, copy, modify, and redistribute this software for
// non-commercial use is hereby granted.
//
// This software is provided "as is" without warranty of any kind,
// either expressed or implied, including but not limited to the
// implied warranties of fitness for a particular purpose.
//

#ifndef ImageObserver_h_DEFINED
#define ImageObserver_h_DEFINED

#include <OPENR/OObject.h>
#include <OPENR/OSubject.h> 
#include <OPENR/OObserver.h>
#include "def.h"

enum ImageObserverState {
    IOS_IDLE,
    IOS_START
};

static const char* const FBK_LOCATOR = "PRM:/r1/c1/c2/c3/i1-FbkImageSensor:F1";

class ImageObserver : public OObject {
public:  
    ImageObserver();
    virtual ~ImageObserver() {}
  
    OSubject*  subject[numOfSubject];
    OObserver* observer[numOfObserver];

    virtual OStatus DoInit   (const OSystemEvent& event);
    virtual OStatus DoStart  (const OSystemEvent& event);
    virtual OStatus DoStop   (const OSystemEvent& event);
    virtual OStatus DoDestroy(const OSystemEvent& event);
    
    void Notify(const ONotifyEvent& event);

private:
    void OpenPrimitive();
    void SetCdtVectorDataOfPinkBall();
    void PrintTagInfo(OFbkImageVectorData* imageVec);
    void SaveRawData(char* path,
                     OFbkImageVectorData* imageVec, OFbkImageLayer layer);
    bool ReconstructImage(OFbkImageVectorData* imageVec,
                          OFbkImageLayer layer,
                          byte** image, int* width, int* height);
    void FreeImage(byte* image);

    void PutPixel(byte* img, int w, int x, int y, byte pix) {
        *(img + w*y + x) = pix;
    }

    byte ClipRange(int val) {
        if (val < 0)        { return 0;         }
        else if (val > 255) { return 255;       }
        else                { return (byte)val; }
    }

    ImageObserverState  imageObserverState;
    OPrimitiveID        fbkID;
};

#endif // ImageObserver_h_DEFINED
