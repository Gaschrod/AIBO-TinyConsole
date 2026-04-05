//
// Copyright 1998,1999,2002 Sony Corporation 
//
// Permission to use, copy, modify, and redistribute this software for
// non-commercial use is hereby granted.
//
// This software is provided "as is" without warranty of any kind,
// either expressed or implied, including but not limited to the
// implied warranties of fitness for a particular purpose.
//

#ifndef _OMoNetMessage_h_DEFINED
#define _OMoNetMessage_h_DEFINED

#include <OMWares/Name.h>
#include <OMWares/GInfo.h>
#ifndef LIBOBJECTCOMM2
#include <OPENR/ObjcommTypes.h>
#else
#include <OPENR/ObjcommTypes.h>
#endif

const unsigned int maxNumOfBuffer = 10;
const unsigned int numOfNetwork   =  4;

typedef Name         OMoNetString;
typedef unsigned int OMoNetCommand;
typedef unsigned int OMoNetCommandID;
typedef unsigned int OMoNetNetwork;
typedef GInfo        OMoNetDirection;

//
// network
//
const OMoNetNetwork oNonNetwork              = 0x00000000;
const OMoNetNetwork oHead                    = 0x00000100;
const OMoNetNetwork oLegs                    = 0x00000010;
const OMoNetNetwork oTail                    = 0x00000001;
const OMoNetNetwork oHeadLegs                = oHead | oLegs;
const OMoNetNetwork oHeadTail                = oHead | oTail;
const OMoNetNetwork oLegsTail                = oLegs | oTail;
const OMoNetNetwork oAll                     = oHead | oLegs | oTail;

//
// commandID
//
const OMoNetCommandID oNonCommandID          = 0xffffffff;

//
// non command
//
const OMoNetCommand   oNonCommand            = 0xffffffff;

//
// mask of command
//
// mask
const OMoNetCommand MaskMove                 = 0x01010000; 
const OMoNetCommand MaskReportPosture        = 0x01020000; 
const OMoNetCommand MaskFinishCommand        = 0x01030000; 
const OMoNetCommand MaskStartMotion          = 0x02010000; 
const OMoNetCommand MaskRegisterMotion       = 0x02020000; 
const OMoNetCommand MaskReturnStatus         = 0x02030000; 
#if 1 /* LEGACY SUPPORT */
const OMoNetCommand oMove                    = 0x01010000; 
const OMoNetCommand oStopMove                = 0x01010000;
const OMoNetCommand oFinishCommand           = 0x01030000;
const OMoNetCommand oReturnStatus            = 0x02030000;
#endif /* LEGACY SUPPORT */

//
// command
//
//client -> monet(6)
const OMoNetCommand oMoveNow                 = 0x01010001;
const OMoNetCommand oMoveAfter               = 0x01010002;
const OMoNetCommand oMoveStartPosNow         = 0x01010003;
const OMoNetCommand oMoveStartPosAfter       = 0x01010004;
const OMoNetCommand oStopMoveNormal          = 0x01010005;
const OMoNetCommand oStopMoveEmergency       = 0x01010006;
// monet -> client(3)
const OMoNetCommand oReportPosture           = 0x01020001;
const OMoNetCommand oFinishCommandComplete   = 0x01030001;
const OMoNetCommand oFinishCommandIncomplete = 0x01030002;
// monet -> agent(3)
const OMoNetCommand oStartMotion             = 0x02010001;
const OMoNetCommand oStopMotion              = 0x02010002;
const OMoNetCommand oKillMotion              = 0x02010003;
// agent -> monet(4)
const OMoNetCommand oRegisterMotion          = 0x02020001;
const OMoNetCommand oRegisterMotionFinish    = 0x02020002;
const OMoNetCommand oReturnStatusComplete    = 0x02030001;
const OMoNetCommand oReturnStatusIncomplete  = 0x02030002;
#if 1 /* LEGACY SUPPORT */
const OMoNetCommand oMovePostureNow          = 0x01010011;
const OMoNetCommand oMovePostureAfter        = 0x01010012;
const OMoNetCommand oMoveMotionNow           = 0x01010013;
const OMoNetCommand oMoveMotionAfter         = 0x01010014;
const OMoNetCommand oMoveStartOfMotionNow    = 0x01010015;
const OMoNetCommand oMoveStartOfMotionAfter  = 0x01010016;
#endif /* LEGACY SUPPORT */

//
// size of option data
//
const unsigned int SizeOfOMoNetData = 32;

//
// OMoNetMessage(total 200byte)
//
struct OMoNetMessage {
//    OMoNetString     objectname_;             // 32 byte
    ObserverID       objectname_;             // 8byte
    OMoNetCommand    command_;                //  4
    OMoNetCommandID  commandID_;              //  4
    OMoNetNetwork    network_;                //  4
    OMoNetString     motion_;                 // 32
    OMoNetString     startpos_;               // 32
    OMoNetString     endpos_;                 // 32
    OMoNetDirection  startdir_;               // 12 
    OMoNetDirection  enddir_;                 // 12 
    unsigned char    data_[SizeOfOMoNetData]; // 32 
    unsigned int     activeDataSize_;         //  4

    //
    // constructor
    //
    OMoNetMessage() :
        objectname_(),
        command_(oNonCommand),
        commandID_(oNonCommandID),
        network_(oNonNetwork),
        motion_(),
        startpos_(),
        endpos_(),
        startdir_(),
        enddir_(),
        activeDataSize_(0) {
        for (int i = 0; i < SizeOfOMoNetData; ++i) {
            data_[i] = 0;
        }
    }

    OMoNetMessage(const OMoNetMessage& a) {
        *this = a;
    }

    //
    // destructor
    //
    ~OMoNetMessage() {}

    //
    // operator
    //
    OMoNetMessage& operator=(const OMoNetMessage& a) {
        if (this != &a) {
            objectname_ = a.objectname_;
            command_    = a.command_;
            commandID_  = a.commandID_;
            network_    = a.network_;
            motion_     = a.motion_;
            startpos_   = a.startpos_;
            endpos_     = a.endpos_;
            startdir_   = a.startdir_;
            enddir_     = a.enddir_;
            for (int i = 0; i < SizeOfOMoNetData; ++i) {
                data_[i] = a.data_[i];
            }
            activeDataSize_ = a.activeDataSize_;
        }

        return *this;
    }

    //
    // copy
    //
    void CopyData(const OMoNetMessage& a) {
        for (int i = 0; i < SizeOfOMoNetData; ++i) {
            data_[i] = a.data_[i];
        }
    }

    //
    // is
    //
    bool IsSameNetwork(const OMoNetNetwork& a) const {
        return (network_ == a)? true : false;
    }

    bool IsSameMotion(
        const OMoNetNetwork& network,
        const OMoNetString&  motion,
        const OMoNetString&  startpos,
        const OMoNetString&  endpos) {
        return (network_  == network  &&
                motion_   == motion   &&
                startpos_ == startpos &&
                endpos_   == endpos )? true : false;
    }

    bool IsSameMotion(const OMoNetMessage& a) const {
        return (network_  == a.network_  &&
                motion_   == a.motion_   &&
                startpos_ == a.startpos_ &&
                endpos_   == a.endpos_ )? true : false;
    }

#if 1 /* LEGACY SUPPORT */    
    void SetRegisterMotion(
        ObserverID    objectname,
        OMoNetNetwork network,
        OMoNetString  motion,
        OMoNetString  startpos,
        OMoNetString  endpos) {
        command_    = oRegisterMotion;
        objectname_ = objectname;
        network_    = network;
        motion_     = motion;
        startpos_   = startpos;
        endpos_     = endpos;
    }
#endif /* LEGACY SUPPORT */    

    //
    // print
    //
    void Print() {
//        cout << "\tobjectname_ = " << (char*)objectname_ << endl;
        cout << "\tcommand_    = " << (void*)command_    << endl;
        cout << "\tcommandID_  = " << (void*)commandID_  << endl;
        cout << "\tnetwork_    = " << (void*)network_    << endl;
        cout << "\tmotion_     = " << (char*)motion_     << endl;
        cout << "\tstartpos_   = " << (char*)startpos_   << endl;
        cout << "\tendpos_     = " << (char*)endpos_     << endl;

        if (startdir_.IsValid() == true) {
            cout << "\tstartdir_   = "; startdir_.Print();
        }

        if (enddir_.IsValid() == true) {
            cout << "\tenddir_     = "; enddir_.Print();
        }

        for (int i = 0; i < SizeOfOMoNetData; ++i) {
            if (data_[i] != 0) {
                cout << "\tdata_[" << i << "]    = " 
                     << (int)(data_[i]) << endl;
            }
        }

        if (activeDataSize_ != 0) {
            cout << "\tactiveDataSize_  = " << activeDataSize_ << endl;
        }
    }
};

#endif // _OMoNetMessage_h_DEFINED
