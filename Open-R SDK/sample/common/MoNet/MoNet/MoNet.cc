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

#include <string.h>
#include <OPENR/OPENRAPI.h>
#include <OPENR/OSyslog.h>
#include <OPENR/core_macro.h>
#include <MTNFile.h>
#include "MoNet.h"

MoNet::MoNet() : moNetState(MNS_IDLE), moNetCommandInfoManager(),
                 moNet(), commandArcPath(), 
                 currentCommandID(-1), currentPosture(monetpostureNT),
                 motionDataID(odesigndataID_UNDEF),
                 soundDataID(odesigndataID_UNDEF), motionODA(), soundODA()
{
}

OStatus
MoNet::DoInit(const OSystemEvent& event)
{
    NEW_ALL_SUBJECT_AND_OBSERVER;
    REGISTER_ALL_ENTRY;
    SET_ALL_READY_AND_NOTIFY_ENTRY;

    LoadMotionODA();
    LoadSoundODA();

    char design[orobotdesignNAME_MAX+1];
    OStatus result = OPENR::GetRobotDesign(design);
    if (result != oSUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s %d",
                  "MoNet::DoInit()",
                  "OPENR::GetRobotDesign() FAILED", result));
    }

    if (!strcmp(design, "ERS-210") || !strcmp(design, "ERS-220")) {
        ReadMoNetConfig(MONET_CONFIG);
        ReadMoNetCommandConfig(MONETCMD_CONFIG);
    } else if (!strcmp(design, "ERS-7")) {
        ReadMoNetConfig(MONET7_CONFIG);
        ReadMoNetCommandConfig(MONETCMD7_CONFIG);
    } else {
        OSYSLOG1((osyslogERROR,
                  "MoNet::DoInit() UNKNOWN ROBOT DESIGN"));
    }

    return oSUCCESS;
}

OStatus
MoNet::DoStart(const OSystemEvent& event)
{
    moNetState = MNS_START;

    ENABLE_ALL_SUBJECT;
    ASSERT_READY_TO_ALL_OBSERVER;

    return oSUCCESS;
}

OStatus
MoNet::DoStop(const OSystemEvent& event)
{
    moNetState = MNS_IDLE;

    DISABLE_ALL_SUBJECT;
    DEASSERT_READY_TO_ALL_OBSERVER;

    return oSUCCESS;
}

OStatus
MoNet::DoDestroy(const OSystemEvent& event)
{
    DELETE_ALL_SUBJECT_AND_OBSERVER;
    return oSUCCESS;
}

void
MoNet::NotifyClientCommand(const ONotifyEvent& event)
{
    MoNetCommand* cmd = (MoNetCommand*)event.Data(0);

    if (moNetState == MNS_IDLE) {

        ; // do nothing

    } else if (moNetState == MNS_START) {

        currentCommandID = Execute(cmd->commandID);
        if (currentCommandID != monetcommandID_UNDEF) {
            moNetState = MNS_AGENT_RUNNING;
        } else {
            OSYSLOG1((osyslogERROR,
                      "INVALID MoNetCommandID %d", cmd->commandID));
            ReplyClientResult(cmd->commandID,
                              monetINVALID_ARG, currentPosture);
        }

        observer[event.ObsIndex()]->AssertReady();

    } else if (moNetState == MNS_AGENT_RUNNING) {

        ReplyClientResult(cmd->commandID, monetBUSY, monetpostureUNDEF);
        observer[event.ObsIndex()]->AssertReady();
    }
}

void
MoNet::NotifyAgentResult(const ONotifyEvent& event)
{
    MoNetAgentResult* result = (MoNetAgentResult*)event.Data(0);

    OSYSPRINT(("AGENT RESULT : agent %d index %d status %d endPos %d\n",
               result->agent, result->index,
               result->status, result->endPosture));

    if (moNetState == MNS_IDLE) {

        ; // do nothing

    } else if (moNetState == MNS_START) {

        OSYSLOG1((osyslogERROR,
                  "MoNet::NotifyAgentResult() : moNetState ERROR %d %d %d %d",
                  result->agent, result->index,
                  result->status, result->endPosture));

        observer[event.ObsIndex()]->AssertReady();

    } else if (moNetState == MNS_AGENT_RUNNING) {

        MoNetCommandInfo* cinfo = commandArcPath.front().CommandInfo();
        cinfo->SetAgentResult(result->agent, result->index, result->status);
        if (cinfo->IsDone() == true) {
            
            MoNetPosture endpos = cinfo->EndPosture();
            if (endpos != monetpostureANY) currentPosture = endpos;
                
            cinfo->ClearAgentResult();
            commandArcPath.pop_front();
        
            if (commandArcPath.size() == 0) {
                ReplyClientResult(currentCommandID,
                                  result->status,
                                  currentPosture);
                moNetState = MNS_START;
            } else {
                cinfo = commandArcPath.front().CommandInfo();
                ExecuteCommandInfo(cinfo);
            }
        }

        observer[event.ObsIndex()]->AssertReady();
    }
}

void
MoNet::LoadMotionODA()
{
    OSYSDEBUG(("LoadMotionODA()\n"));

    OStatus result;
    byte*   addr;
    size_t  size;

    result = OPENR::FindDesignData(MONET_MOTION_KEYWORD,
                                   &motionDataID, &addr, &size);
    if (result != oSUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s %d",
                  "MoNet::LoadMotionODA()",
                  "OPENR::FindDesignData() FAILED", result));
        return;
    }

    motionODA.Set(addr);
}

void
MoNet::LoadSoundODA()
{
    OSYSDEBUG(("LoadSoundODA()\n"));

    OStatus result;
    byte*   addr;
    size_t  size;

    result = OPENR::FindDesignData(MONET_SOUND_KEYWORD,
                                   &soundDataID, &addr, &size);
    if (result != oSUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s %d",
                  "MoNet::LoadSoundODA()",
                  "OPENR::FindDesignData() FAILED", result));
        return;
    }

    soundODA.Set(addr);
}

void
MoNet::ReadMoNetConfig(const char* path)
{
    OSYSDEBUG(("MoNet::ReadMoNetConfig(%s)\n", path));

    char buf[LINEBUFSIZE];
    char agt[MONET_AGENT_NAME_MAX+1];
    char cmd[MONET_AGENT_COMMAND_NAME_MAX+1];
    char idx[NUMBUFSIZE];

    FILE* fp = fopen(path, "r");
    if (fp == 0) {
        OSYSLOG1((osyslogERROR, "%s : %s %s",
                  "MoNet::ReadMoNetConfig()", "can't open", path));
        return;
    }

    while (fgets(buf, LINEBUFSIZE, fp) != 0) {
        if (buf[0] == '#') continue; // skip comment line
        strcpy(agt, strtok(buf, " \t"));
        strcpy(cmd, strtok(0, " \t"));
        strcpy(idx, strtok(0, " \t"));

        MoNetAgentID agent, start, end;
        int index;

        agent = ToMoNetAgentID(agt);
        if (agent == monetagentSOUND) {
            index = ToCommandIndex(soundODA, cmd, idx);
            start = monetpostureANY;
            end   = monetpostureANY;

        } else {
            index = ToCommandIndex(motionODA, cmd, idx);
            start = ToStartPosture(cmd);
            end   = ToEndPosture(cmd);
        }

        MoNetCommandInfo* info = moNetCommandInfoManager.New();
        info->SetAgentCommand(agent, index, start, end);
        if (info->StartPosture() != monetpostureANY &&
            info->StartPosture() != info->EndPosture()) {
            CommandArc arc(start, end, info);
            moNet.Add(arc);
        }
    }

    fclose(fp);

    moNet.Print();
}

void
MoNet::ReadMoNetCommandConfig(const char* path)
{
    OSYSDEBUG(("MoNet::ReadMoNetCommandConfig(%s)\n", path));

    char buf[LINEBUFSIZE];
    char cmdID[NUMBUFSIZE];
    char numagt[NUMBUFSIZE];
    char synckey[NUMBUFSIZE];
    char agt[MONET_AGENT_NAME_MAX+1];
    char cmd[MONET_AGENT_COMMAND_NAME_MAX+1];
    char idx[NUMBUFSIZE];

    FILE* fp = fopen(path, "r");
    if (fp == 0) {
        OSYSLOG1((osyslogERROR, "%s : %s %s",
                  "MoNet::ReadMoNetConfig()", "can't open", path));
        return;
    }

    while (fgets(buf, LINEBUFSIZE, fp) != 0) {

        if (buf[0] == '#') continue; // skip comment line

        strcpy(cmdID,   strtok(buf, " \t"));
        strcpy(numagt,  strtok(0, " \t"));
        strcpy(synckey, strtok(0, " \t"));

        MoNetCommandID id    = (MoNetCommandID)atoi(cmdID);
        int numAgentCommands = atoi(numagt);
        bool useSyncKey      = (synckey[0] == '0') ? false : true;
        OSYSPRINT(("commandID %d numAgentCommands %d useSyncKey %d\n",
                   id, numAgentCommands, useSyncKey));

        MoNetCommandInfo* info = moNetCommandInfoManager.New(id, useSyncKey);
        
        int n = 0;
        while (n != numAgentCommands && fgets(buf, LINEBUFSIZE, fp) != 0) {

            if (buf[0] == '#') continue; // skip comment line

            strcpy(agt, strtok(buf, " \t"));
            strcpy(cmd, strtok(0, " \t"));
            strcpy(idx, strtok(0, " \t"));

            MoNetAgentID agent, start, end;
            int index;

            agent = ToMoNetAgentID(agt);
            if (agent == monetagentSOUND) {
                index = ToCommandIndex(soundODA, cmd, idx);
                start = monetpostureANY;
                end   = monetpostureANY;
            } else {
                index = ToCommandIndex(motionODA, cmd, idx);
                start = ToStartPosture(cmd);
                end   = ToEndPosture(cmd);
            }

            info->SetAgentCommand(agent, index, start, end);
            n++;
        }

        if (info->StartPosture() != monetpostureANY &&
            info->StartPosture() != info->EndPosture()) {
            CommandArc arc(info->StartPosture(), info->EndPosture(), info);
            moNet.Add(arc);
        }
    }

    fclose(fp);

    moNet.Print();
}

MoNetAgentID
MoNet::ToMoNetAgentID(char* agt)
{
    if (strcmp(agt, monetagentMTN_STRNAME) == 0) {
        return monetagentMTN;
    } else if (strcmp(agt, monetagentSOUND_STRNAME) == 0) {
        return monetagentSOUND;
    } else if (strcmp(agt, monetagentNEUTRAL_STRNAME) == 0) {
        return monetagentNEUTRAL;
    } else if (strcmp(agt, monetagentMTNWALK_STRNAME) == 0) {
        return monetagentMTNWALK;
    } else {
        return monetagentUNDEF;
    }
}

int
MoNet::ToCommandIndex(const ODA& oda, char* cmd, char* idx)
{
    int index = atoi(idx);
    if (index >= 0) {
        return index;
    } else {
        for (int i = 0; i < oda.GetNumFiles(); i++) {
            if (oda.GetMagic(i) == ODA_MAGIC_OMTN) {

                MTNFile* mtnfile = (MTNFile*)oda.GetData(i);
                if (strcmp(mtnfile->GetName(), cmd) == 0) {
                    OSYSPRINT(("%s %d\n", cmd, i));
                    return i;
                }

            } else {

                if (strcmp(oda.GetName(i), cmd) == 0) {
                    OSYSPRINT(("%s %d\n", cmd, i));
                    return i;
                }

            }
        }
    }

    OSYSPRINT(("%s -1\n", cmd));
    return -1;
}

MoNetPosture
MoNet::ToStartPosture(char* cmd)
{
    char buf[MONET_AGENT_COMMAND_NAME_MAX+1];
    strcpy(buf, cmd);

    strtok(buf, "_#");
    char* pos = strtok(0, "_#");
    
    return ToPosture(pos);
}

MoNetPosture
MoNet::ToEndPosture(char* cmd)
{
    char buf[MONET_AGENT_COMMAND_NAME_MAX+1];
    strcpy(buf, cmd);

    strtok(buf, "_#");
    strtok(0, "_#");
    char* pos = strtok(0, "_#");

    return ToPosture(pos);
}

MoNetPosture
MoNet::ToPosture(char* pos)
{
    if (strcmp(pos, "stand") == 0) {
        return monetpostureSTAND;
    } else if (strcmp(pos, "sit") == 0) {
        return monetpostureSIT;
    } else if (strcmp(pos, "sleep") == 0) {
        return monetpostureSLEEP;
    } else if (strcmp(pos, "walk") == 0) {
        return monetpostureWALK;
    } else if (strcmp(pos, "any") == 0) {
        return monetpostureANY;
    } else if (strcmp(pos, "nt") == 0) {
        return monetpostureNT;
    } else {
        return monetpostureUNDEF;
    }
}

MoNetCommandID
MoNet::Execute(MoNetCommandID id)
{
    MoNetCommandInfo* cinfo = moNetCommandInfoManager.Find(id);
    if (cinfo == 0) {
        MoNetResult result(id, monetINVALID_ARG, monetagentUNDEF);
        subject[sbjClientResult]->SetData(&result, sizeof(result));
        subject[sbjClientResult]->NotifyObservers();
        return monetcommandID_UNDEF;
    }

    if (cinfo->StartPosture() != currentPosture
        && cinfo->StartPosture() != monetpostureANY) {

        int dist = moNet.Search(currentPosture,
                                cinfo->StartPosture(),
                                commandArcPath);
        if (dist < 0) {
            OSYSLOG1((osyslogERROR,
                      "MoNet::Execute() : moNet.Search(%d -> %d) FAILED",
                      currentPosture, cinfo->StartPosture()));
            return monetcommandID_UNDEF;
        }
    }

    CommandArc arc(cinfo->StartPosture(), cinfo->EndPosture(), cinfo);
    commandArcPath.push_back(arc);
    
    cinfo = commandArcPath.front().CommandInfo();
    ExecuteCommandInfo(cinfo);

    return id;
}

void
MoNet::ExecuteCommandInfo(MoNetCommandInfo* cinfo)
{
    OVRSyncKey dividedSyncKey[ovrsynckeyMAX_NUM_DIVISION];

    if (cinfo->UseSyncKey() == true) {
        NewAndDivideSyncKey(cinfo->NumAgentCommands(), dividedSyncKey);
    } else {
        for (int i = 0; i < cinfo->NumAgentCommands(); i++) {
            dividedSyncKey[i] = ovrsynckeyUNDEF;
        }
    }

    for (int i = 0; i < cinfo->NumAgentCommands(); i++) {

        int sbjidx;
        MoNetAgentCommand* agtcmd = cinfo->AgentCommand(i);
        agtcmd->syncKey = dividedSyncKey[i];
        if (agtcmd->agent == monetagentSOUND) {
            sbjidx = sbjSoundAgentCommand;
        } else {
            sbjidx = sbjMotionAgentCommand;
        }

        OSYSDEBUG(("AGENT COMMAND: agent %d index %d syncKey 0x%08x\n", 
                   agtcmd->agent, agtcmd->index, agtcmd->syncKey));

        subject[sbjidx]->SetData(agtcmd, sizeof(*agtcmd));
        subject[sbjidx]->NotifyObservers();

    }
}

int
MoNet::NewAndDivideSyncKey(int ndivkey, OVRSyncKey* divkey)
{
    OStatus result;

    if (ndivkey > ovrsynckeyMAX_NUM_DIVISION) {
        for (int i = 0; i < ndivkey; i++) divkey[i] = ovrsynckeyUNDEF;
        return -1;
    }

    result = OPENR::NewSyncKey(&divkey[0]);
    if (result != oSUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s %d",
                  "MoNet::DivideSyncKey()",
                  "OPENR::NewSyncKey() FAILED", result));
        for (int i = 0; i < ndivkey; i++) divkey[i] = ovrsynckeyUNDEF;
        return -1;
    }

    int nkey = 1;
    int idxA = 0;
    int idxB = 1;
    OVRSyncKey keyA;
    OVRSyncKey keyB;

    while (nkey < ndivkey) {
        result = OPENR::DivideSyncKey(divkey[idxA], &keyA, &keyB);
        if (result == oSUCCESS) {

            divkey[idxA] = keyA;
            divkey[idxB] = keyB;
            OSYSDEBUG(("divkey[%d] 0x%08x divkey[%d] 0x%08x\n",
                       idxA, keyA, idxB, keyB));
            
            idxB++;
            nkey++;
            OSYSDEBUG(("nkey %d idxA %d idxB %d\n", nkey, idxA, idxB));

        } else {

            //
            // divkey[idxA] can't be divided any more.
            // So, divkey[idxA+1] will be divided in next loop.
            //
            idxA++;
            OSYSDEBUG(("nkey %d idxA %d idxB %d\n", nkey, idxA, idxB));
        }
    }

#ifdef OPENR_DEBUG
    for (int i = 0; i < nkey; i++)
        OSYSDEBUG(("divkey[%d] 0x%08x\n", i, divkey[i]));
#endif

    return nkey;
}

void 
MoNet::ReplyClientResult(MoNetCommandID id, MoNetStatus st, MoNetPosture pos)
{
    MoNetResult result(id, st, pos);
    subject[sbjClientResult]->SetData(&result, sizeof(result));
    subject[sbjClientResult]->NotifyObservers();

}
