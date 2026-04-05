//
// Copyright 2001,2002 Sony Corporation 
//
// Permission to use, copy, modify, and redistribute this software for
// non-commercial use is hereby granted.
//
// This software is provided "as is" without warranty of any kind,
// either expressed or implied, including but not limited to the
// implied warranties of fitness for a particular purpose.
//

#ifndef _SoccerLion_h_DEFINED
#define _SoccerLion_h_DEFINED

#include <OPENR/OObject.h>
#include <OPENR/OSubject.h>
#include <OPENR/OObserver.h>
#include <OPENR/ODataFormats.h>

#include <OMWares/OMPsdStatus.h>
#include <OMWares/OMPsdCPG.h>
#include <OMWares/OMPsdRH.h>
#include <OMWares/OMoNetMessage.h>
#include <OMWares/MoNetMessageMaker.h>
#include <OMWares/MMM_CommandList.h>

#include "def.h"
#include "MNetMsgQ.h"

enum SoccerLionBehavior {
    SOCCERLION_BOOTING,
    SOCCERLION_SUSPENDING,
    SOCCERLION_TRACKING_BALL,
    SOCCERLION_SEARCHING_BALL
};

enum SoccerLionWalk {
    SCWALK_UNDEF,
    SCWALK_FALLINGDOWN,
    SCWALK_SUSPENDING,
    SCWALK_FORWARD,
    SCWALK_RIGHTFORWARD,
    SCWALK_LEFTFORWARD,
    SCWALK_RIGHT,
    SCWALK_LEFT,
    SCWALK_BACKWARD,
    SCWALK_TURNRIGHT,
    SCWALK_TURNLEFT,
    SCWALK_RIGHTKICK,
    SCWALK_LEFTKICK
};

// Param Set Flag
#define NOTYET 0
#define DONE   1

// Flag to block sending command while FallDown=>GetUp
#define STATE_FALLDOWN     0
#define STATE_NOTFALLDOWN  1


// Threshold for size of CDT Result
#define NPIXELS_THRESHOLD  32

//
// sample color table of a pink ball
//
const unsigned char pink_ball[ocdtMAX_Y_SEGMENT*4] =
{
    230, 150, 190, 120,
    230, 150, 190, 120,
    230, 150, 190, 120,
    230, 150, 190, 120,
    230, 150, 190, 120,
    230, 150, 190, 120,
    230, 150, 190, 120,
    230, 150, 190, 120,
    230, 150, 190, 120,
    230, 150, 190, 120,
    230, 150, 190, 120,
    230, 150, 190, 120,
    230, 150, 190, 120,
    230, 150, 190, 120,
    230, 150, 190, 120,
    230, 150, 190, 120,
    230, 150, 190, 120,
    230, 150, 190, 120,
    230, 150, 190, 120,
    230, 150, 190, 120,
    230, 160, 190, 120,
    230, 160, 190, 120,
    230, 160, 190, 120,
    230, 160, 190, 120,
    230, 160, 190, 120,
    230, 160, 190, 120,
    230, 160, 190, 120,
    230, 160, 190, 120,
    230, 160, 190, 120,
    230, 160, 190, 120,
    230, 160, 190, 120,
    230, 160, 190, 120
};

class SoccerLion : public OObject {
public:
    OSubject*  subject[numOfSubject];
    OObserver* observer[numOfObserver];
 
    SoccerLion();
    virtual ~SoccerLion() {}
        
    virtual OStatus DoInit   (const OSystemEvent& event);
    virtual OStatus DoStart  (const OSystemEvent& event);
    virtual OStatus DoStop   (const OSystemEvent& event);
    virtual OStatus DoDestroy(const OSystemEvent& event);
    
    void ReadySetMotion(const OReadyEvent& event);

    void ResultMotion  (const ONotifyEvent& event);
    void ResultPSD     (const ONotifyEvent& event);
    void ResultCDT     (const ONotifyEvent& event);
    void ResultGSensor (const ONotifyEvent& event);
    void ResultSensor  (const ONotifyEvent& event);

private:
    SoccerLionBehavior  behavior_;
    SoccerLionWalk      walk_;
    
    int range2obstacle_; // Range to Obstacle
    int tiltjoint_;      // tilt angle (degrees)
    int panjoint_;       // pan angle (degrees)

    int  lostcounter_;   // Counter use to count the Frames of the Lost Ball 
    int  hitcounter_;    // Counter use to count the frames of hitted
	int  robotdesign_;   // ers-210 = 1, ers-220 = 2

    MoNetMessageMaker mmm_;      // to make OMoNetMessage
    MNetMsgQ          msgQ_;     // OMoNetMessage Que

    OMPsdCPG          ompsdCPG_; // for OMPsd Control Param Generator
    OMPsdRH           ompsdRH_;  // for OMPsd Result Handler

    void SendMotion();
    void SendWalkCommand();
    void SendHeadCommand();
    void SendTailCommand();
    void SendGetUpCommand();
    void SendParamPSD();
    void SendParamCDT();
};

#endif // _SoccerLion_h_DEFINED
