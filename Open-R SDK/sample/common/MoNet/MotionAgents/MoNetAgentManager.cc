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

#include <OPENR/OPENRAPI.h>
#include <OPENR/OSyslog.h>
#include "MoNetAgentManager.h"
#include "MoNetAgent.h"

MoNetAgentManager::MoNetAgentManager() : 
    prevMoNetAgent(0),
    activeMoNetAgent(0),
    activeAgentCommand(),
    moNetAgentList(),
    effectorSubject(0),
    motionDataID(odesigndataID_UNDEF), 
    motionODA()
{
    for (int i = 0; i < MAX_NUM_JOINTS; i++)
        primitiveID[i] = oprimitiveID_UNDEF;

    for (int i = 0; i < NUM_COMMAND_VECTOR; i++)
        commandRegions[i] = 0;
}

void
MoNetAgentManager::InitDRX900()
{
    OSYSDEBUG(("MoNetAgentManager::InitDRX900()\n"));

    OpenPrimitivesDRX900();
    NewCommandVectorDataDRX900();
    LoadMotionODA();

    list<MoNetAgent*>::iterator iter = moNetAgentList.begin();
    list<MoNetAgent*>::iterator last = moNetAgentList.end();
    while (iter != last) {
        (*iter)->Init(&motionODA);
        ++iter;
    }
}

void
MoNetAgentManager::InitDRX1000()
{
    OSYSDEBUG(("MoNetAgentManager::InitDRX1000()\n"));

    OpenPrimitivesDRX1000();
    NewCommandVectorDataDRX1000();
    LoadMotionODA();

    list<MoNetAgent*>::iterator iter = moNetAgentList.begin();
    list<MoNetAgent*>::iterator last = moNetAgentList.end();
    while (iter != last) {
        (*iter)->Init(&motionODA);
        ++iter;
    }
}

void
MoNetAgentManager::Start(OSubject* effector)
{
    OSYSDEBUG(("MoNetAgentManager::Start()\n"));
    effectorSubject = effector;
}

void
MoNetAgentManager::RegisterMoNetAgent(MoNetAgent* m)
{
    OSYSDEBUG(("MoNetAgentManager::RegisterMoNetAgent()\n"));
    moNetAgentList.push_back(m);
    m->SetMoNetAgentManager(this);
}

void
MoNetAgentManager::NotifyCommand(const ONotifyEvent& event,
                                 MoNetAgentResult* result)
{
    MoNetAgentCommand* cmd = (MoNetAgentCommand*)event.Data(0);

    if (activeMoNetAgent != 0) {
        // BUSY
        result->agent      = cmd->agent;
        result->index      = cmd->index;
        result->status     = monetBUSY;
        result->endPosture = monetpostureUNDEF;
        return;
    }
    
    list<MoNetAgent*>::iterator iter = moNetAgentList.begin();
    list<MoNetAgent*>::iterator last = moNetAgentList.end();
    while (iter != last) {
        if ((*iter)->AreYou(cmd->agent) == true) {
            if (*iter != prevMoNetAgent &&
                cmd->agent != monetagentNEUTRAL) {
                (*iter)->SetJointGain();
            }
            (*iter)->NotifyCommand(*cmd, result);
            if (result->status != monetSUCCESS) return;
            activeMoNetAgent   = *iter;
            activeAgentCommand = *cmd;
            return;
        }
        ++iter;
    }

    // INVALID_ARG
    result->agent      = cmd->agent;
    result->index      = cmd->index;
    result->status     = monetINVALID_ARG;
    result->endPosture = monetpostureUNDEF;
}

void
MoNetAgentManager::ReadyEffector(const OReadyEvent& event,
                                 MoNetAgentResult* result)
    
{
    if (activeMoNetAgent == 0) {
        result->agent      = monetagentUNDEF;
        result->index      = -1;
        result->status     = monetSUCCESS;
        result->endPosture = monetpostureUNDEF;
        return;
    }

    activeMoNetAgent->ReadyEffector(activeAgentCommand, result);
    if (result->status != monetSUCCESS) {
        prevMoNetAgent = activeMoNetAgent;
        activeMoNetAgent = 0;
        activeAgentCommand.Clear();
    }
}

void
MoNetAgentManager::OpenPrimitivesDRX900()
{
    OSYSDEBUG(("MoNetAgentManager::OpenPrimitivesDRX900()\n"));

    OStatus result;

    for (int i = 0; i < DRX900_NUM_JOINTS; i++) {
        result = OPENR::OpenPrimitive(DRX900_JOINT_LOCATOR[i],
                                      &primitiveID[i]);
        if (result != oSUCCESS) {
            OSYSLOG1((osyslogERROR, "%s : %s %d",
                      "MoNetAgentManager::OpenPrimitivesDRX900()",
                      "OPENR::OpenPrimitive() FAILED", result));
        }
    }
}

void
MoNetAgentManager::OpenPrimitivesDRX1000()
{
    OSYSDEBUG(("MoNetAgentManager::OpenPrimitivesDRX1000()\n"));

    OStatus result;

    for (int i = 0; i < DRX1000_NUM_JOINTS; i++) {
        result = OPENR::OpenPrimitive(DRX1000_JOINT_LOCATOR[i],
                                      &primitiveID[i]);
        if (result != oSUCCESS) {
            OSYSLOG1((osyslogERROR, "%s : %s %d",
                      "MoNetAgentManager::OpenPrimitivesDRX1000()",
                      "OPENR::OpenPrimitive() FAILED", result));
        }

        OSYSDEBUG(("%s %x\n", DRX1000_JOINT_LOCATOR[i], primitiveID[i]));
    }
}

void
MoNetAgentManager::NewCommandVectorDataDRX900()
{
    OSYSDEBUG(("MoNetAgentManager::NewCommandVectorData()\n"));

    OStatus result;
    MemoryRegionID      cmdVecDataID;
    OCommandVectorData* cmdVecData;
    OCommandInfo*       info;

    for (int i = 0; i < NUM_COMMAND_VECTOR; i++) {

        result = OPENR::NewCommandVectorData(DRX900_NUM_JOINTS, 
                                             &cmdVecDataID, &cmdVecData);
        if (result != oSUCCESS) {
            OSYSLOG1((osyslogERROR, "%s : %s %d",
                      "MoNetAgentManager::NewCommandVectorData()",
                      "OPENR::NewCommandVectorData() FAILED", result));
        }

        commandRegions[i] = new RCRegion(cmdVecData->vectorInfo.memRegionID,
                                         cmdVecData->vectorInfo.offset,
                                         (void*)cmdVecData,
                                         cmdVecData->vectorInfo.totalSize);

        cmdVecData->SetNumData(DRX900_NUM_JOINTS);

        for (int j = 0; j < DRX900_NUM_JOINTS; j++) {
            info = cmdVecData->GetInfo(j);
            info->Set(odataJOINT_COMMAND2, primitiveID[j], NUM_FRAMES);
        }
    }
}

void
MoNetAgentManager::NewCommandVectorDataDRX1000()
{
    OSYSDEBUG(("MoNetAgentManager::NewCommandVectorData()\n"));

    OStatus result;
    MemoryRegionID      cmdVecDataID;
    OCommandVectorData* cmdVecData;
    OCommandInfo*       info;

    for (int i = 0; i < NUM_COMMAND_VECTOR; i++) {

        result = OPENR::NewCommandVectorData(DRX1000_NUM_JOINTS, 
                                             &cmdVecDataID, &cmdVecData);
        if (result != oSUCCESS) {
            OSYSLOG1((osyslogERROR, "%s : %s %d",
                      "MoNetAgentManager::NewCommandVectorData()",
                      "OPENR::NewCommandVectorData() FAILED", result));
        }

        commandRegions[i] = new RCRegion(cmdVecData->vectorInfo.memRegionID,
                                         cmdVecData->vectorInfo.offset,
                                         (void*)cmdVecData,
                                         cmdVecData->vectorInfo.totalSize);

        for (int j = 0; j < DRX1000_NUM_JOINT2; j++) {
            info = cmdVecData->GetInfo(j);
            info->Set(odataJOINT_COMMAND2, primitiveID[j], NUM_FRAMES);
        }



        for (int j = DRX1000_NUM_JOINT2; j < DRX1000_NUM_JOINTS; j++) {
            info = cmdVecData->GetInfo(j);
            info->Set(odataJOINT_COMMAND4, primitiveID[j], NUM_FRAMES);
        }

        // cmdVecData->SetNumData(DRX1000_NUM_JOINT2);
        cmdVecData->SetNumData(DRX1000_NUM_JOINTS);
    }
}

RCRegion*
MoNetAgentManager::FindFreeCommandRegion()
{   
    for (int i = 0; i < NUM_COMMAND_VECTOR; i++) {
        if (commandRegions[i]->NumberOfReference() == 1) {
            return commandRegions[i];
        }
    }

    return 0;
}

void
MoNetAgentManager::LoadMotionODA()
{
    OSYSDEBUG(("MoNetAgentManager::LoadMotionODA()\n"));

    OStatus result;
    byte*   addr;
    size_t  size;

    result = OPENR::FindDesignData(MONET_MOTION_KEYWORD,
                                   &motionDataID, &addr, &size);
    if (result != oSUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s %d",
                  "MTNAgent::LoadMotionODA()",
                  "OPENR::FindDesignData() FAILED", result));
        return;
    }

    motionODA.Set(addr);
}
