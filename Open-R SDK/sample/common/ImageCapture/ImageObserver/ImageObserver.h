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
static const char* const TINSW_LOCATOR = "PRM:/r1/c1/c2/c3/c4/s5-Sensor:s5";
static const char* const BACKSW_LOCATOR = "PRM:/r6/s1-Sensor:s1";
static const char* const BACKR7_LOCATOR = "PRM:/t2-Sensor:t2"; // REAR
static const char* const BACKM7_LOCATOR = "PRM:/t3-Sensor:t3"; // FRONT
static const char* const BACKF7_LOCATOR = "PRM:/t4-Sensor:t4"; // MID

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
    void PrintTagInfo(OFbkImageVectorData* imageVec);
    void SaveRawData(char* path,
                     OFbkImageVectorData* imageVec, OFbkImageLayer layer);

    bool ExistData(int serialNumber);
    void SaveData(int serialNumber, OFbkImageVectorData* imageVec);

    ImageObserverState  imageObserverState;
    OPrimitiveID        fbkID;
    OPrimitiveID        tinswID;
    OPrimitiveID        backswID;
    OPrimitiveID        backf7ID;
    OPrimitiveID        backm7ID;
    OPrimitiveID        backr7ID;

    static const int ROBOT_UNDEF  = 0;
    static const int ROBOT_ERS200 = 1;
    static const int ROBOT_ERS7   = 2;
    int robot;
};

#endif // ImageObserver_h_DEFINED
