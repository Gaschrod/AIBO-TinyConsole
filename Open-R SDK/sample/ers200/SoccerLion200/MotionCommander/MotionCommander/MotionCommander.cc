//
// Copyright 2002 Sony Corporation 
//
// Permission to use, copy, modify, and redistribute this software for
// non-commercial use is hereby granted.
//
// This software is provided "as is" without warranty of any kind,
// either expressed or implied, including but not limited to the
// implied warranties of fitness for a particular purpose.
//

#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <OPENR/core_macro.h>
#include <OPENR/ODataFormats.h>
#include <OPENR/OPENRAPI.h>
#include <OPENR/OSyslog.h>
#include "MotionCommander.h"

static double rad(double x) { return (x * M_PI / 180.0); }

MotionCommander::MotionCommander()
{
    haveNewMotion = false;
    waitResult = true;
}

OStatus
MotionCommander::DoInit(const OSystemEvent& event)
{
    NEW_ALL_SUBJECT_AND_OBSERVER;
    REGISTER_ALL_ENTRY;
    SET_ALL_READY_AND_NOTIFY_ENTRY;

    return oSUCCESS;
}

OStatus
MotionCommander::DoStart(const OSystemEvent& event)
{
    ENABLE_ALL_SUBJECT;
    ASSERT_READY_TO_ALL_OBSERVER;

    OPENR::SetMotorPower(opowerON);
    OPENR::SetDefaultJointGain(oprimitiveUNDEF);
    OPENR::EnableJointGain(oprimitiveUNDEF);
    sendNewMotion();

    return oSUCCESS;
}

OStatus
MotionCommander::DoStop(const OSystemEvent& event)
{
    OPENR::SetMotorPower(opowerOFF);

    DISABLE_ALL_SUBJECT;
    DEASSERT_READY_TO_ALL_OBSERVER;

    return oSUCCESS;
}

OStatus
MotionCommander::DoDestroy(const OSystemEvent& event)
{
    DELETE_ALL_SUBJECT_AND_OBSERVER;
    return oSUCCESS;
}

void
MotionCommander::notifyMotionResult(const ONotifyEvent& event)
{
    OSYSPRINT(("motion finished.\n"));
    if (waitResult) {
        sendNewMotion();
    }
    observer[obsMotionResult]->AssertReady();
}

void
MotionCommander::readySetMotion(const OReadyEvent &event)
{
    if (haveNewMotion || !waitResult) {
        sendNewMotion();
    }
}

void
MotionCommander::sendNewMotion()
{
    haveNewMotion = true;
    OSubject *sbj = subject[sbjSetMotion];
    if (sbj->IsReady()) {
        bool result = readCommand();
        if (result == false) { // Shutdown ...
            haveNewMotion = false;
            return;
        }
        sbj->SetData(mmm.ProvideMoNetMessage(), sizeof(OMoNetMessage));
        sbj->NotifyObservers();
        haveNewMotion = false;
    }
}

void
MotionCommander::help()
{
    OSYSPRINT(("Available commands:\n"));
    OSYSPRINT(("h|help|?                    - this screen\n"));
    OSYSPRINT(("q                           - quit\n\n"));

    OSYSPRINT(("s n                         - stop normal\n"));
    OSYSPRINT(("s e                         - stop emergency\n"));

    OSYSPRINT(("w s                         - walk stop\n"));
    OSYSPRINT(("w t l [steps]               - walk turn left\n"));
    OSYSPRINT(("w t r [steps]               - walk turn right\n"));
    OSYSPRINT(("w t b [steps]               - walk skip back\n\n"));

    OSYSPRINT(("w d f [steps]               - walk default forward\n"));
    OSYSPRINT(("w d lf [steps]              - walk default left-forward\n"));
    OSYSPRINT(("w d rf [steps]              - walk default right-forward\n"));
    OSYSPRINT(("w d l [steps]               - walk default left\n"));
    OSYSPRINT(("w d r [steps]               - walk default right\n"));
    OSYSPRINT(("w d b [steps]               - walk default back\n\n"));

    OSYSPRINT(("w e f [steps]               - walk stable forward\n"));
    OSYSPRINT(("w e lf [steps]              - walk stable left-forward\n"));
    OSYSPRINT(("w e rf [steps]              - walk stable right-forward\n"));
    OSYSPRINT(("w e l [steps]               - walk stable left\n"));
    OSYSPRINT(("w e r [steps]               - walk stable right\n"));
    OSYSPRINT(("w e b [steps]               - walk stable back\n\n"));

    OSYSPRINT(("w s f [steps]               - walk slow forward\n"));
    OSYSPRINT(("w s lf [steps]              - walk slow left-forward\n"));
    OSYSPRINT(("w s rf [steps]              - walk slow right-forward\n"));
    OSYSPRINT(("w s l [steps]               - walk slow left\n"));
    OSYSPRINT(("w s r [steps]               - walk slow right\n"));
    OSYSPRINT(("w s b [steps]               - walk slow back\n\n"));

    OSYSPRINT(("w o f [steps]               - walk off-load forward\n"));
    OSYSPRINT(("w o lf [steps]              - walk off-load left-forward\n"));
    OSYSPRINT(("w o rf [steps]              - walk off-load right-forward\n"));
    OSYSPRINT(("w o l [steps]               - walk off-load left\n"));
    OSYSPRINT(("w o r [steps]               - walk off-load right\n"));
    OSYSPRINT(("w o b [steps]               - walk off-load back\n\n"));

    OSYSPRINT(("w f f [steps]               - walk fast forward\n"));
    OSYSPRINT(("w f lf [steps]              - walk fast left-forward\n"));
    OSYSPRINT(("w f rf [steps]              - walk fast right-forward\n"));
    OSYSPRINT(("w f l [steps]               - walk fast left\n"));
    OSYSPRINT(("w f r [steps]               - walk fast right\n"));
    OSYSPRINT(("w f b [steps]               - walk fast back\n\n"));

    OSYSPRINT(("w l f [steps]               - walk lizard forward\n"));
    OSYSPRINT(("w l lf [steps]              - walk lizard left-forward\n"));
    OSYSPRINT(("w l rf [steps]              - walk lizard right-forward\n"));
    OSYSPRINT(("w l l [steps]               - walk lizard left\n"));
    OSYSPRINT(("w l r [steps]               - walk lizard right\n"));
    OSYSPRINT(("w l b [steps]               - walk lizard back\n\n"));

    OSYSPRINT(("w b f [steps]               - walk baby forward\n"));
    OSYSPRINT(("w b lf [steps]              - walk baby left-forward\n"));
    OSYSPRINT(("w b rf [steps]              - walk baby right-forward\n"));
    OSYSPRINT(("w b l [steps]               - walk baby left\n"));
    OSYSPRINT(("w b r [steps]               - walk baby right\n"));
    OSYSPRINT(("w b b [steps]               - walk baby back\n\n"));

    OSYSPRINT(("w c f [steps]               - walk creep forward\n"));
    OSYSPRINT(("w c lf [steps]              - walk creep left-forward\n"));
    OSYSPRINT(("w c rf [steps]              - walk creep right-forward\n"));
    OSYSPRINT(("w c l [steps]               - walk creep left\n"));
    OSYSPRINT(("w c r [steps]               - walk creep right\n"));
    OSYSPRINT(("w c b [steps]               - walk creep back\n\n"));

    OSYSPRINT(("w r f [steps]               - walk trot forward\n"));
    OSYSPRINT(("w r lf [steps]              - walk trot left-forward\n"));
    OSYSPRINT(("w r rf [steps]              - walk trot right-forward\n"));
    OSYSPRINT(("w r l [steps]               - walk trot left\n"));
    OSYSPRINT(("w r r [steps]               - walk trot right\n"));
    OSYSPRINT(("w r b [steps]               - walk trot back\n\n"));

    OSYSPRINT(("w a f [steps]               - walk adaptive forward\n"));
    OSYSPRINT(("w a lf [steps]              - walk adaptive left-forward\n"));
    OSYSPRINT(("w a rf [steps]              - walk adaptive right-forward\n"));
    OSYSPRINT(("w a l [steps]               - walk adaptive left\n"));
    OSYSPRINT(("w a r [steps]               - walk adaptive right\n"));
    OSYSPRINT(("w a b [steps]               - walk adaptive back\n\n"));

    OSYSPRINT(("h h                         - head home\n"));
    OSYSPRINT(("h p tilt pan roll [time]    - head pos\n"));
    OSYSPRINT(("h r tilt pan roll [time]    - head rel pos\n\n"));

    OSYSPRINT(("t h                         - tail home\n"));
    OSYSPRINT(("t p tilt pan [time]         - tail pos\n"));
    OSYSPRINT(("t s [time]                  - tail swing\n"));
}

static int num(char *s)
{
    while (*s && !isdigit((unsigned char)*s)) s++;
    int n = atoi(s);
    if (n < 1) n = 1;
    return n;
}

bool
MotionCommander::readCommand()
{
    waitResult = true;

    for (;;) {
        OSYSPRINT(("MotionCommander> "));

        char buffer[200], *s;
        fgets(buffer, sizeof(buffer) - 1, stdin);

        s = strchr(buffer, '\n');
        if (s) *s = 0;
        s = buffer;

        float tilt, pan, roll;
        unsigned int time;

        if (!strcasecmp(s, "h") || !strcasecmp(s, "help") || !strcmp(s, "?")) {
            help();
            continue;
        } else if (!strcasecmp(s, "q")) {
            OSYSPRINT(("SHUTDOWN ...\n"));
            OBootCondition bootCond(obcbPAUSE_SW);
            OPENR::Shutdown(bootCond);
            waitResult = false;
            return false;
        } else if (!strcmp(s, "s n")) {
            mmm.MakeWalkingCommand(mmmmcStopMoveNormal, mmmscWalkStop);
            waitResult = false;
        } else if (!strcmp(s, "s e")) {
            mmm.MakeWalkingCommand(mmmmcStopMoveEmergency, mmmscWalkStop);
            waitResult = false;
        } else if (!strcmp(s, "w s")) {
            mmm.MakeWalkingCommand(mmmmcMoveNow, mmmscWalkStop);
        } else if (!strncmp(s, "w t l", 5)) {
            mmm.MakeStepWalkingCommand(mmmmcMoveNow, mmmscWalkTurnLeft, num(s));
        } else if (!strncmp(s, "w t r", 5)) {
            mmm.MakeStepWalkingCommand(mmmmcMoveNow, mmmscWalkTurnRight, num(s));
        } else if (!strncmp(s, "w t b", 5)) {
            mmm.MakeStepWalkingCommand(mmmmcMoveNow, mmmscSkipBack, num(s));
        } else if (!strncmp(s, "w d f", 5)) {
            mmm.MakeStepWalkingCommand(mmmmcMoveNow, mmmscWalkForward, num(s));
        } else if (!strncmp(s, "w d lf", 6)) {
            mmm.MakeStepWalkingCommand(mmmmcMoveNow, mmmscWalkLeftForward, num(s));
        } else if (!strncmp(s, "w d rf", 6)) {
            mmm.MakeStepWalkingCommand(mmmmcMoveNow, mmmscWalkRightForward, num(s));
        } else if (!strncmp(s, "w d l", 5)) {
            mmm.MakeStepWalkingCommand(mmmmcMoveNow, mmmscWalkLeft, num(s));
        } else if (!strncmp(s, "w d r", 5)) {
            mmm.MakeStepWalkingCommand(mmmmcMoveNow, mmmscWalkRight, num(s));
        } else if (!strncmp(s, "w d b", 5)) {
            mmm.MakeStepWalkingCommand(mmmmcMoveNow, mmmscWalkBackward, num(s));
        } else if (!strncmp(s, "w e f", 5)) {
            mmm.MakeStepWalkingCommand(mmmmcMoveNow, mmmscStableWalkForward, num(s));
        } else if (!strncmp(s, "w e lf", 6)) {
            mmm.MakeStepWalkingCommand(mmmmcMoveNow, mmmscStableWalkLeftForward, num(s));
        } else if (!strncmp(s, "w e rf", 6)) {
            mmm.MakeStepWalkingCommand(mmmmcMoveNow, mmmscStableWalkRightForward, num(s));
        } else if (!strncmp(s, "w e l", 5)) {
            mmm.MakeStepWalkingCommand(mmmmcMoveNow, mmmscStableWalkLeft, num(s));
        } else if (!strncmp(s, "w e r", 5)) {
            mmm.MakeStepWalkingCommand(mmmmcMoveNow, mmmscStableWalkRight, num(s));
        } else if (!strncmp(s, "w e b", 5)) {
            mmm.MakeStepWalkingCommand(mmmmcMoveNow, mmmscStableWalkBackward, num(s));
        } else if (!strncmp(s, "w s f", 5)) {
            mmm.MakeStepWalkingCommand(mmmmcMoveNow, mmmscSlowWalkForward, num(s));
        } else if (!strncmp(s, "w s lf", 6)) {
            mmm.MakeStepWalkingCommand(mmmmcMoveNow, mmmscSlowWalkLeftForward, num(s));
        } else if (!strncmp(s, "w s rf", 6)) {
            mmm.MakeStepWalkingCommand(mmmmcMoveNow, mmmscSlowWalkRightForward, num(s));
        } else if (!strncmp(s, "w s l", 5)) {
            mmm.MakeStepWalkingCommand(mmmmcMoveNow, mmmscSlowWalkLeft, num(s));
        } else if (!strncmp(s, "w s r", 5)) {
            mmm.MakeStepWalkingCommand(mmmmcMoveNow, mmmscSlowWalkRight, num(s));
        } else if (!strncmp(s, "w s b", 5)) {
            mmm.MakeStepWalkingCommand(mmmmcMoveNow, mmmscSlowWalkBackward, num(s));
        } else if (!strncmp(s, "w o f", 5)) {
            mmm.MakeStepWalkingCommand(mmmmcMoveNow, mmmscOffLoadWalkForward, num(s));
        } else if (!strncmp(s, "w o lf", 6)) {
            mmm.MakeStepWalkingCommand(mmmmcMoveNow, mmmscOffLoadWalkLeftForward, num(s));
        } else if (!strncmp(s, "w o rf", 6)) {
            mmm.MakeStepWalkingCommand(mmmmcMoveNow, mmmscOffLoadWalkRightForward, num(s));
        } else if (!strncmp(s, "w o l", 5)) {
            mmm.MakeStepWalkingCommand(mmmmcMoveNow, mmmscOffLoadWalkLeft, num(s));
        } else if (!strncmp(s, "w o r", 5)) {
            mmm.MakeStepWalkingCommand(mmmmcMoveNow, mmmscOffLoadWalkRight, num(s));
        } else if (!strncmp(s, "w o b", 5)) {
            mmm.MakeStepWalkingCommand(mmmmcMoveNow, mmmscOffLoadWalkBackward, num(s));
        } else if (!strncmp(s, "w f f", 5)) {
            mmm.MakeStepWalkingCommand(mmmmcMoveNow, mmmscFastWalkForward, num(s));
        } else if (!strncmp(s, "w f lf", 6)) {
            mmm.MakeStepWalkingCommand(mmmmcMoveNow, mmmscFastWalkLeftForward, num(s));
        } else if (!strncmp(s, "w f rf", 6)) {
            mmm.MakeStepWalkingCommand(mmmmcMoveNow, mmmscFastWalkRightForward, num(s));
        } else if (!strncmp(s, "w f l", 5)) {
            mmm.MakeStepWalkingCommand(mmmmcMoveNow, mmmscFastWalkLeft, num(s));
        } else if (!strncmp(s, "w f r", 5)) {
            mmm.MakeStepWalkingCommand(mmmmcMoveNow, mmmscFastWalkRight, num(s));
        } else if (!strncmp(s, "w f b", 5)) {
            mmm.MakeStepWalkingCommand(mmmmcMoveNow, mmmscFastWalkBackward, num(s));
        } else if (!strncmp(s, "w l f", 5)) {
            mmm.MakeStepWalkingCommand(mmmmcMoveNow, mmmscLizardWalkForward, num(s));
        } else if (!strncmp(s, "w l lf", 6)) {
            mmm.MakeStepWalkingCommand(mmmmcMoveNow, mmmscLizardWalkLeftForward, num(s));
        } else if (!strncmp(s, "w l rf", 6)) {
            mmm.MakeStepWalkingCommand(mmmmcMoveNow, mmmscLizardWalkRightForward, num(s));
        } else if (!strncmp(s, "w l l", 5)) {
            mmm.MakeStepWalkingCommand(mmmmcMoveNow, mmmscLizardWalkLeft, num(s));
        } else if (!strncmp(s, "w l r", 5)) {
            mmm.MakeStepWalkingCommand(mmmmcMoveNow, mmmscLizardWalkRight, num(s));
        } else if (!strncmp(s, "w l b", 5)) {
            mmm.MakeStepWalkingCommand(mmmmcMoveNow, mmmscLizardWalkBackward, num(s));
        } else if (!strncmp(s, "w b f", 5)) {
            mmm.MakeStepWalkingCommand(mmmmcMoveNow, mmmscBabyWalkForward, num(s));
        } else if (!strncmp(s, "w b lf", 6)) {
            mmm.MakeStepWalkingCommand(mmmmcMoveNow, mmmscBabyWalkLeftForward, num(s));
        } else if (!strncmp(s, "w b rf", 6)) {
            mmm.MakeStepWalkingCommand(mmmmcMoveNow, mmmscBabyWalkRightForward, num(s));
        } else if (!strncmp(s, "w b l", 5)) {
            mmm.MakeStepWalkingCommand(mmmmcMoveNow, mmmscBabyWalkLeft, num(s));
        } else if (!strncmp(s, "w b r", 5)) {
            mmm.MakeStepWalkingCommand(mmmmcMoveNow, mmmscBabyWalkRight, num(s));
        } else if (!strncmp(s, "w b b", 5)) {
            mmm.MakeStepWalkingCommand(mmmmcMoveNow, mmmscBabyWalkBackward, num(s));
        } else if (!strncmp(s, "w c f", 5)) {
            mmm.MakeStepWalkingCommand(mmmmcMoveNow, mmmscCreepWalkForward, num(s));
        } else if (!strncmp(s, "w c lf", 6)) {
            mmm.MakeStepWalkingCommand(mmmmcMoveNow, mmmscCreepWalkLeftForward, num(s));
        } else if (!strncmp(s, "w c rf", 6)) {
            mmm.MakeStepWalkingCommand(mmmmcMoveNow, mmmscCreepWalkRightForward, num(s));
        } else if (!strncmp(s, "w c l", 5)) {
            mmm.MakeStepWalkingCommand(mmmmcMoveNow, mmmscCreepWalkLeft, num(s));
        } else if (!strncmp(s, "w c r", 5)) {
            mmm.MakeStepWalkingCommand(mmmmcMoveNow, mmmscCreepWalkRight, num(s));
        } else if (!strncmp(s, "w c b", 5)) {
            mmm.MakeStepWalkingCommand(mmmmcMoveNow, mmmscCreepWalkBackward, num(s));
        } else if (!strncmp(s, "w r f", 5)) {
            mmm.MakeStepWalkingCommand(mmmmcMoveNow, mmmscTrotWalkForward, num(s));
        } else if (!strncmp(s, "w r lf", 6)) {
            mmm.MakeStepWalkingCommand(mmmmcMoveNow, mmmscTrotWalkLeftForward, num(s));
        } else if (!strncmp(s, "w r rf", 6)) {
            mmm.MakeStepWalkingCommand(mmmmcMoveNow, mmmscTrotWalkRightForward, num(s));
        } else if (!strncmp(s, "w r l", 5)) {
            mmm.MakeStepWalkingCommand(mmmmcMoveNow, mmmscTrotWalkLeft, num(s));
        } else if (!strncmp(s, "w r r", 5)) {
            mmm.MakeStepWalkingCommand(mmmmcMoveNow, mmmscTrotWalkRight, num(s));
        } else if (!strncmp(s, "w r b", 5)) {
            mmm.MakeStepWalkingCommand(mmmmcMoveNow, mmmscTrotWalkBackward, num(s));
        } else if (!strncmp(s, "w a f", 5)) {
            mmm.MakeStepWalkingCommand(mmmmcMoveNow, mmmscAdaptiveWalkForward, num(s));
        } else if (!strncmp(s, "w a lf", 6)) {
            mmm.MakeStepWalkingCommand(mmmmcMoveNow, mmmscAdaptiveWalkLeftForward, num(s));
        } else if (!strncmp(s, "w a rf", 6)) {
            mmm.MakeStepWalkingCommand(mmmmcMoveNow, mmmscAdaptiveWalkRightForward, num(s));
        } else if (!strncmp(s, "w a l", 5)) {
            mmm.MakeStepWalkingCommand(mmmmcMoveNow, mmmscAdaptiveWalkLeft, num(s));
        } else if (!strncmp(s, "w a r", 5)) {
            mmm.MakeStepWalkingCommand(mmmmcMoveNow, mmmscAdaptiveWalkRight, num(s));
        } else if (!strncmp(s, "w a b", 5)) {
            mmm.MakeStepWalkingCommand(mmmmcMoveNow, mmmscAdaptiveWalkBackward, num(s));
        } else if (!strcmp(s, "h h")) {
            mmm.MakeHeadCommand(mmmmcMoveNow, mmmscHeadHome);
        } else if (sscanf(s, "h p %f %f %f %u", &tilt, &pan, &roll, &time)==4) {
            tilt = rad(tilt);
            pan = rad(pan);
            roll = rad(roll);
            mmm.MakeHeadCommand(mmmmcMoveNow, mmmscHeadMove,
                                tilt, pan, roll, time);
        } else if (sscanf(s, "h p %f %f %f", &tilt, &pan, &roll)==3) {
            tilt = rad(tilt);
            pan = rad(pan);
            roll = rad(roll);
            mmm.MakeHeadCommand(mmmmcMoveNow, mmmscHeadMove, tilt, pan, roll);
        } else if (sscanf(s, "h r %f %f %f %u", &tilt, &pan, &roll, &time)==4) {
            tilt = rad(tilt);
            pan = rad(pan);
            roll = rad(roll);
            mmm.MakeHeadCommand(mmmmcMoveNow, mmmscHeadMoveDiff,
                                tilt, pan, roll, time);
        } else if (sscanf(s, "h r %f %f %f", &tilt, &pan, &roll)==3) {
            tilt = rad(tilt);
            pan = rad(pan);
            roll = rad(roll);
            mmm.MakeHeadCommand(mmmmcMoveNow, mmmscHeadMoveDiff,
                                tilt, pan, roll);
        } else if (!strcmp(s, "t h")) {
            mmm.MakeTailCommand(mmmmcMoveNow, mmmscTailHome);
        } else if (sscanf(s, "t p %f %f %u", &tilt, &pan, &time) == 3) {
            tilt = rad(tilt);
            pan = rad(pan);
            mmm.MakeTailCommand(mmmmcMoveNow, mmmscTailMove, tilt, pan, time);
        } else if (sscanf(s, "t p %f %f", &tilt, &pan) == 2) {
            tilt = rad(tilt);
            pan = rad(pan);
            mmm.MakeTailCommand(mmmmcMoveNow, mmmscTailMove, tilt, pan);
        } else if (sscanf(s, "t s %u", &time) == 1) {
            mmm.MakeTailCommand(mmmmcMoveNow, mmmscTailSwing, time);
        } else if (!strcmp(s, "t s")) {
            mmm.MakeTailCommand(mmmmcMoveNow, mmmscTailSwing);
        } else {
            OSYSPRINT(("Unknown command.  Type '?' for help.\n"));
            continue;
        }
        break;
    }

    return true;
}
