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

#ifndef MoNetAgentManager_h_DEFINED
#define MoNetAgentManager_h_DEFINED

#include <list>
using namespace std;

#include <OPENR/OSubject.h>
#include <OPENR/OObserver.h>
#include <MoNetData.h>
#include <ODA.h>

const int HEAD_TILT  =  0; // DRX-900  (ERS-210/220)
const int HEAD_TILT1 =  0; // DRX-1000 (ERS-7)
const int HEAD_PAN   =  1;
const int HEAD_ROLL  =  2; // DRX-900  (ERS-210/220)
const int HEAD_TILT2 =  2; // DRX-1000 (ERS-7)
const int RFLEG_J1   =  3;
const int RFLEG_J2   =  4;
const int RFLEG_J3   =  5;
const int LFLEG_J1   =  6;
const int LFLEG_J2   =  7;
const int LFLEG_J3   =  8;
const int RRLEG_J1   =  9;
const int RRLEG_J2   = 10;
const int RRLEG_J3   = 11;
const int LRLEG_J1   = 12;
const int LRLEG_J2   = 13;
const int LRLEG_J3   = 14;
const int TAIL_TILT  = 15;
const int TAIL_PAN   = 16;
const int MOUTH      = 17;
const int LEFT_EAR   = 18;
const int RIGHT_EAR  = 19;

const int DRX900_NUM_JOINTS = 15;
static const char* const DRX900_JOINT_LOCATOR[] = {
    "PRM:/r1/c1-Joint2:j1",             // HEAD TILT
    "PRM:/r1/c1/c2-Joint2:j2",          // HEAD PAN
    "PRM:/r1/c1/c2/c3-Joint2:j3",       // HEAD ROLL
    "PRM:/r4/c1-Joint2:j1",             // RFLEG J1 (Right Front Leg)
    "PRM:/r4/c1/c2-Joint2:j2",          // RFLEG J2
    "PRM:/r4/c1/c2/c3-Joint2:j3",       // RFLEG J3
    "PRM:/r2/c1-Joint2:j1",             // LFLEG J1 (Left Front Leg)
    "PRM:/r2/c1/c2-Joint2:j2",          // LFLEG J2
    "PRM:/r2/c1/c2/c3-Joint2:j3",       // LFLEG J3
    "PRM:/r5/c1-Joint2:j1",             // RRLEG J1 (Right Rear Leg)
    "PRM:/r5/c1/c2-Joint2:j2",          // RRLEG J2
    "PRM:/r5/c1/c2/c3-Joint2:j3",       // RRLEG J3
    "PRM:/r3/c1-Joint2:j1",             // LRLEG J1 (Left Rear Leg)
    "PRM:/r3/c1/c2-Joint2:j2",          // LRLEG J2
    "PRM:/r3/c1/c2/c3-Joint2:j3",       // LRLEG J3
};

const int DRX1000_NUM_JOINT2 = 18;
const int DRX1000_NUM_JOINT4 =  2;
const int DRX1000_NUM_JOINTS = DRX1000_NUM_JOINT2 + DRX1000_NUM_JOINT4;
static const char* const DRX1000_JOINT_LOCATOR[] = {
    "PRM:/r1/c1-Joint2:11",          // TILT1
    "PRM:/r1/c1/c2-Joint2:12",       // PAN
    "PRM:/r1/c1/c2/c3-Joint2:13",    // TILT2
    "PRM:/r4/c1-Joint2:41",          // RFLEG J1 (Right Front Leg)
    "PRM:/r4/c1/c2-Joint2:42",       // RFLEG J2
    "PRM:/r4/c1/c2/c3-Joint2:43",    // RFLEG J3
    "PRM:/r2/c1-Joint2:21",          // LFLEG J1 (Left Front Leg)
    "PRM:/r2/c1/c2-Joint2:22",       // LFLEG J2
    "PRM:/r2/c1/c2/c3-Joint2:23",    // LFLEG J3
    "PRM:/r5/c1-Joint2:51",          // RRLEG J1 (Right Rear Leg)
    "PRM:/r5/c1/c2-Joint2:52",       // RRLEG J2
    "PRM:/r5/c1/c2/c3-Joint2:53",    // RRLEG J3
    "PRM:/r3/c1-Joint2:31",          // LRLEG J1 (Left Rear Leg)
    "PRM:/r3/c1/c2-Joint2:32",       // LRLEG J2
    "PRM:/r3/c1/c2/c3-Joint2:33",    // LRLEG J3
    "PRM:/r6/c1-Joint2:61",          // TAIL TILT
    "PRM:/r6/c2-Joint2:62",          // TAIL PAN
    "PRM:/r1/c1/c2/c3/c4-Joint2:14", // MOUTH
    "PRM:/r1/c1/c2/c3/e5-Joint4:15", // LEFT EAR
    "PRM:/r1/c1/c2/c3/e6-Joint4:16"  // RIGHT EAR
};

const int MAX_NUM_JOINTS = DRX1000_NUM_JOINTS;
const int NUM_FRAMES     = 4;

class MoNetAgent;

class MoNetAgentManager {
public:
    MoNetAgentManager();
    ~MoNetAgentManager() {}

    void InitDRX900();
    void InitDRX1000();
    void Start(OSubject* effector);

    void RegisterMoNetAgent(MoNetAgent* m);

    void NotifyCommand(const ONotifyEvent& event, MoNetAgentResult* result);
    void ReadyEffector(const OReadyEvent& event,  MoNetAgentResult* result);

    OPrimitiveID PrimitiveID(int idx) { return primitiveID[idx]; }
    OSubject*    Effector()           { return effectorSubject;  }
    RCRegion*    FindFreeCommandRegion();

private:
    void OpenPrimitivesDRX900();
    void NewCommandVectorDataDRX900();

    void OpenPrimitivesDRX1000();
    void NewCommandVectorDataDRX1000();

    void LoadMotionODA();

    static const size_t NUM_COMMAND_VECTOR =  4;

    MoNetAgent*          prevMoNetAgent;
    MoNetAgent*          activeMoNetAgent;
    MoNetAgentCommand    activeAgentCommand;
    list<MoNetAgent*>    moNetAgentList;

    OSubject*    effectorSubject;
    OPrimitiveID primitiveID[MAX_NUM_JOINTS];

    RCRegion*    commandRegions[NUM_COMMAND_VECTOR];

    ODesignDataID  motionDataID;
    ODA            motionODA;
};

#endif // MoNetAgentManager_h_DEFINED
