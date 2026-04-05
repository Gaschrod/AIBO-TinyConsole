//
// Copyright 2001,2002,2003 Sony Corporation 
//
// Permission to use, copy, modify, and redistribute this software for
// non-commercial use is hereby granted.
//
// This software is provided "as is" without warranty of any kind,
// either expressed or implied, including but not limited to the
// implied warranties of fitness for a particular purpose.
//

#include <OPENR/OPENRAPI.h>
#include <OPENR/OUnits.h>
#include <OPENR/core_macro.h>
#include <OMWares/OMGsensorData.h>
#include <OMWares/CdtStruct.h>
#include <OMWares/OMoNetMessage.h>
#include <OMWares/MMM_CommandList.h>
#include <OMWares/MoNetMessageMaker.h>
#include <string.h>
#include "SoccerLion.h"

SoccerLion::SoccerLion() : 
    behavior_(SOCCERLION_BOOTING), walk_(SCWALK_UNDEF),
    range2obstacle_(0), tiltjoint_(0), panjoint_(0),
    robotdesign_(0),
    lostcounter_(0), hitcounter_(0), mmm_(), msgQ_(), ompsdCPG_(), ompsdRH_()
{
}

OStatus
SoccerLion::DoInit(const OSystemEvent& event)
{
    OStatus status;
    char design[256];

    NEW_ALL_SUBJECT_AND_OBSERVER;
    REGISTER_ALL_ENTRY;
    SET_ALL_READY_AND_NOTIFY_ENTRY;

// check ERS-210 or ERS-220

    status = OPENR::GetRobotDesign(design);
    if (status != oSUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s %d",
                  "RobotDesign::DoStart()",
                  "OPENR::GetRobotDesign() FAILED", status));
        return status;
    }

    if (!strcmp(design, "ERS-210"))
        robotdesign_ = 1; // ERS-210
    else if (!strcmp(design, "ERS-220"))
        robotdesign_ = 2; // ERS-220
    // OTHER robotdesign_ = 0;

    return oSUCCESS;
}

OStatus
SoccerLion::DoStart(const OSystemEvent& event)
{
    ENABLE_ALL_SUBJECT;
    ASSERT_READY_TO_ALL_OBSERVER;

    SendParamPSD();
    SendParamCDT();

	OPENR::SetMotorPower(opowerON);
	OPENR::EnableJointGain(oprimitiveID_UNDEF); // Is it neccesary?

    return oSUCCESS;
}

OStatus
SoccerLion::DoStop(const OSystemEvent& event)
{
    OPENR::SetMotorPower(opowerOFF);

    DISABLE_ALL_SUBJECT;
    DEASSERT_READY_TO_ALL_OBSERVER;

    return oSUCCESS;
}

OStatus
SoccerLion::DoDestroy(const OSystemEvent& event)
{
    DELETE_ALL_SUBJECT_AND_OBSERVER;
    return oSUCCESS;
}

void
SoccerLion::ReadySetMotion(const OReadyEvent&)
{
    SendMotion();
}

void
SoccerLion::ResultMotion(const ONotifyEvent& event)
{
    OMoNetMessage* result = (OMoNetMessage*)event.Data(0);

    if (strcmp(result->motion_.s_, "ReactiveGU") == 0) {
        walk_ = SCWALK_SUSPENDING;
    }

    OSYSDEBUG(("ResultMotion() : motion %s behavior_ %d walk_ %d\n",
               result->motion_.s_, behavior_, walk_));

    observer[event.ObsIndex()]->AssertReady();
}

void
SoccerLion::ResultPSD(const ONotifyEvent& event)
{
    void* result = (void*)event.Data(0);
    ompsdRH_.InportResult(&result);
    range2obstacle_ = ompsdRH_.GetRangeToObstacle();

    observer[event.ObsIndex()]->AssertReady();
}

void
SoccerLion::ResultCDT(const ONotifyEvent& event)
{
    CdtResults* result = (CdtResults*)event.Data(0);

    if (result->status == POSITION) {
        if (result->npixel >= NPIXELS_THRESHOLD) {
            lostcounter_ = 0;   // BALL FOUND
        } else {
            lostcounter_++;     // BALL NOT FOUND
        }
    }
    int prev_behavior = behavior_;

    if (behavior_ != SOCCERLION_SUSPENDING) {
        if (lostcounter_ > 10) {
            behavior_ = SOCCERLION_SEARCHING_BALL;
        } else if (lostcounter_ == 0) {
            behavior_ = SOCCERLION_TRACKING_BALL;
        }
    }

    if (prev_behavior != behavior_) {
        SendTailCommand();
        SendHeadCommand();
        SendWalkCommand();
    }

    observer[event.ObsIndex()]->AssertReady();
}


void
SoccerLion::ResultGSensor(const ONotifyEvent& event)
{
    OMGsensorMessage* result = (OMGsensorMessage*)event.Data(0);
    
    if (result->IsPosture(FALL_DOWN_FRONT) ||
        result->IsPosture(FALL_DOWN_REAR ) ||
        result->IsPosture(FALL_DOWN_RIGHT) ||
        result->IsPosture(FALL_DOWN_LEFT )) {

        if (walk_ != SCWALK_FALLINGDOWN) {
            walk_ = SCWALK_FALLINGDOWN;
            SendGetUpCommand();
        }
    }

    observer[event.ObsIndex()]->AssertReady();
}

void
SoccerLion::ResultSensor(const ONotifyEvent& event)
{
    OSensorFrameVectorData* sensor = (OSensorFrameVectorData*)event.Data(0);
	int head_touch_force; // PRM:/r1/c1/c2/c3/f2-Sensor:f2

    // tilt and pan angles (degrees)
    tiltjoint_ = (int)degrees(sensor->GetData(0)->frame[0].value / 1000000.0);
    panjoint_ = (int)degrees(sensor->GetData(1)->frame[0].value / 1000000.0);

    // Head sensor > 0.1 N

    if (robotdesign_ == 1)
        head_touch_force = sensor->GetData(4)->frame[0].value;
    else if (robotdesign_ == 2)
        head_touch_force = sensor->GetData(5)->frame[0].value; // Max 19217

    if (head_touch_force > 100000) {
        hitcounter_++;
    } else {
        if (hitcounter_ != 0) {
            // Head sensor is pushed for over 320ms, then released.
            if (hitcounter_ > 10) {
                if (behavior_ == SOCCERLION_SUSPENDING) {
                    behavior_ = SOCCERLION_SEARCHING_BALL;
                } else {
                    behavior_ = SOCCERLION_SUSPENDING;
                }
            }
            hitcounter_ = 0;
        }
    }

    SoccerLionWalk prev_walk  = walk_;

    if (behavior_ == SOCCERLION_BOOTING) {

        behavior_ = SOCCERLION_SUSPENDING;
        walk_ = SCWALK_SUSPENDING;

    } if (behavior_ == SOCCERLION_SUSPENDING) {

        walk_ = SCWALK_SUSPENDING;

    } else if (behavior_ == SOCCERLION_TRACKING_BALL) {
        if ( (tiltjoint_ < -45) && (range2obstacle_ < 150) ) {
            if (panjoint_ > 0) {
                walk_ = SCWALK_LEFTKICK;
            } else {
                walk_ = SCWALK_RIGHTKICK;
            }
        } else if (panjoint_ >  60) {
            walk_ = SCWALK_TURNLEFT;
        } else if (panjoint_ >  45) {
            walk_ = SCWALK_LEFT;
        } else if (panjoint_ >  15) {
            walk_ = SCWALK_LEFTFORWARD;
        } else if (panjoint_ < -60) {
            walk_ = SCWALK_TURNRIGHT;
        } else if (panjoint_ < -45) {
            walk_ = SCWALK_RIGHT;
        } else if (panjoint_ < -15) {
            walk_ = SCWALK_RIGHTFORWARD;
        } else {
            walk_ = SCWALK_FORWARD;
        }

    } else { // behavior_ == SOCCERLION_SEARCHING_BALL

        walk_ = SCWALK_TURNRIGHT;
    }

    if (prev_walk != walk_) SendWalkCommand();
    observer[event.ObsIndex()]->AssertReady();
}

void
SoccerLion::SendMotion()
{
    if (subject[sbjSetMotion]->IsReady()) {
        OMoNetMessage* msg =  msgQ_.GetMoNetMessage();
        if (msg) {
            subject[sbjSetMotion]->SetData(msg, sizeof(OMoNetMessage));
            subject[sbjSetMotion]->NotifyObservers();
            msgQ_.DeleteFirstOne();
        }
    }
}

void
SoccerLion::SendWalkCommand()
{
    if (walk_ == SCWALK_FALLINGDOWN) return;

    switch (walk_) {
    case  SCWALK_FORWARD:
        mmm_.MakeWalkingCommand(mmmmcMoveNow, mmmscWalkForward);
        break;

    case  SCWALK_RIGHTFORWARD:
        mmm_.MakeWalkingCommand(mmmmcMoveNow, mmmscWalkRightForward);
        break;

    case  SCWALK_LEFTFORWARD:
        mmm_.MakeWalkingCommand(mmmmcMoveNow, mmmscWalkLeftForward);
        break;

    case  SCWALK_RIGHT:
        mmm_.MakeWalkingCommand(mmmmcMoveNow, mmmscWalkRight);
        break;

    case  SCWALK_LEFT:
        mmm_.MakeWalkingCommand(mmmmcMoveNow, mmmscWalkLeft);
        break;

    case  SCWALK_BACKWARD:
        mmm_.MakeWalkingCommand(mmmmcMoveNow, mmmscWalkBackward);
        break;

    case  SCWALK_TURNRIGHT:
        mmm_.MakeWalkingCommand(mmmmcMoveNow, mmmscWalkTurnRight);
        break;

    case  SCWALK_TURNLEFT:
        mmm_.MakeWalkingCommand(mmmmcMoveNow, mmmscWalkTurnLeft);
        break;

    case  SCWALK_RIGHTKICK:
        mmm_.MakeKickingCommand(mmmmcMoveNow,
                                mmmscRightKick, radians(panjoint_));
        break;

    case  SCWALK_LEFTKICK:
        mmm_.MakeKickingCommand(mmmmcMoveNow,
                                mmmscLeftKick, radians(panjoint_));
        break;

    case  SCWALK_SUSPENDING:
        mmm_.MakeStepWalkingCommand(mmmmcMoveNow, mmmscWalkForward, 1);
        break;
    }

    OStatus result = msgQ_.AddMoNetMessage(mmm_.ProvideMoNetMessage());
    if (result != oSUCCESS) {
        OSYSLOG1((osyslogERROR,
                  "msgQ_.AddMoNetMessage() FAILED %d", result));
        return;
    }

    SendMotion();
}

void
SoccerLion::SendHeadCommand()
{
    if (walk_ == SCWALK_FALLINGDOWN) return;

    if (behavior_ == SOCCERLION_TRACKING_BALL) {

        mmm_.MakeHeadCommand(mmmmcMoveNow,
                             mmmscHeadCTracking, (unsigned int)1000);


        OStatus result = msgQ_.AddMoNetMessage(mmm_.ProvideMoNetMessage());
        if (result != oSUCCESS) {
            OSYSLOG1((osyslogERROR,
                      "msgQ_.AddMoNetMessage() FAILED %d", result));
            return;
        }
        SendMotion();
    } else { // behavior_ is SOCCERLION_SEARCHING_BALL or SOOCERLION_SUSPENDING

        int numCommands = 0;

        // Set Search Motion 3 times
        for (int j = 0; j < 3; j++) {

            // Search motion is constructed by 9 HeadMoveCommand.
            for (int i = 0; i < 9; i++) {
                double tilt, pan;
                switch (i) {
                case 0:
                    tilt = radians(-15.0);
                    pan  = radians(0.0);
                    break;

                case 1:
                    tilt = radians(-15.0);
                    pan  = radians(-40.0);
                    break;

                case 2:
                    tilt = radians(-15.0);
                    pan  = radians(-80.0);
                    break;

                case 3:
                    tilt = radians(-45.0);
                    pan  = radians(-40.0);
                    break;

                case 4:
                    tilt = radians(-45.0);
                    pan  = radians(0.0);
                    break;

                case 5:
                    tilt = radians(-45.0);
                    pan  = radians(40.0);
                    break;

                case 6:
                    tilt = radians(-45.0);
                    pan  = radians(80.0);
                    break;

                case 7:
                    tilt = radians(-15.0);
                    pan  = radians(40.0);
                    break;

                case 8:
                    tilt = radians(-15.0);
                    pan  = radians(0.0);
                    break;
                }

                mmm_.MakeHeadCommand(mmmmcMoveAfter,
                                     mmmscHeadMove,
                                     tilt, pan, 0.0,
                                     (unsigned int)500);

                OStatus result;
                result = msgQ_.AddMoNetMessage(mmm_.ProvideMoNetMessage());

                if (result == oSUCCESS) {
                    numCommands++;
                } else {
                    OSYSLOG1((osyslogERROR,
                              "msgQ_.AddMoNetMessage() FAILED %d", result));
                }
            }
        }

        if (numCommands > 0) SendMotion();
    }
}

void
SoccerLion::SendTailCommand()
{
    if (robotdesign_ != 1) // work only ERS-210
    	return; 
    	
    if (walk_ == SCWALK_FALLINGDOWN) return;
    
    if (behavior_ == SOCCERLION_TRACKING_BALL) {

        // tail swing
        mmm_.MakeTailCommand(mmmmcMoveNow,
                             mmmscTailSwing,
                             radians(0.0),       // tilt  0 [deg]
                             radians(45.0),      // pan  45 [deg]
                             (unsigned int)600); // cycle 600msec

    } else { // behavior_ is SOCCERLION_SEARCHING_BALL or SOCCERLION_SUSPENDING

        // tail up
        mmm_.MakeTailCommand(mmmmcMoveNow,
                             mmmscTailMove,
                             radians(25.0),       // tilt 25 [deg]
                             radians(0.0),        // pan   0 [deg]
                             (unsigned int)1000); // cycle 1000msec
    }

    OStatus result = msgQ_.AddMoNetMessage(mmm_.ProvideMoNetMessage());
    if (result != oSUCCESS) {
        OSYSLOG1((osyslogERROR,
                  "msgQ_.AddMoNetMessage() FAILED %d", result));
        return;
    }

    SendMotion();
}

void
SoccerLion::SendGetUpCommand()
{
    OSYSDEBUG(("SendGetUpCommand() walk_ %d\n", walk_));

    OMoNetMessage message;
    message.command_ = oStopMoveEmergency;

    OStatus result = msgQ_.AddMoNetMessage(&message);
    if (result != oSUCCESS) {
        OSYSLOG1((osyslogERROR,
                  "msgQ_.AddMoNetMessage() FAILED %d", result));
        return;
    }

    message.command_ = oMoveNow;
    message.network_ = oAll;
    strcpy(message.motion_.s_, "ReactiveGU");

    result = msgQ_.AddMoNetMessage(&message);
    if (result != oSUCCESS) {
        OSYSLOG1((osyslogERROR,
                  "msgQ_.AddMoNetMessage() FAILED %d", result));
        return;
    }
        
    SendMotion();        
}

void
SoccerLion::SendParamPSD()
{
    OServiceEntry notifyEntry(myOID_ , Entry_Notify[obsGetPSDResult]);
    void*  ctrlParam;
    size_t sizeData;

    ompsdCPG_.Generate_NotDefined((ObserverID)notifyEntry);
    ompsdCPG_.ProvideOMPsdControlParameter(ctrlParam, sizeData);

    subject[sbjSetPSDParam]->SetData(ctrlParam, sizeData);
    subject[sbjSetPSDParam]->NotifyObservers();
}

void
SoccerLion::SendParamCDT()
{
    CdtCntrl parameter;
    parameter.command = SETTABLE;
    parameter.channel = 0;

    for (int y = 0; y < ocdtMAX_Y_SEGMENT; y++ ) {
        
        longword crMax = (longword)pink_ball[4*y+0];
        longword crMin = (longword)pink_ball[4*y+1];
        longword cbMax = (longword)pink_ball[4*y+2];
        longword cbMin = (longword)pink_ball[4*y+3];
            
        crMax = crMax <<  8;
        cbMin = cbMin << 16;
        cbMax = cbMax << 24;

        parameter.table[y] = crMax|crMin|cbMax|cbMin;
    }

    subject[sbjSetCDTTable]->SetData( &parameter, sizeof(parameter) );
    subject[sbjSetCDTTable]->NotifyObservers();
}
