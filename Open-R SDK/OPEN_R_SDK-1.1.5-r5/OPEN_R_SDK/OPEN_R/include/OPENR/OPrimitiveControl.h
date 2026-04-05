//
// OPrimitiveControl.h
//
// Copyright 2000,2001,2002,2003 Sony Corporation
//

#ifndef _OPrimitiveControl_h_DEFINED
#define _OPrimitiveControl_h_DEFINED

#include <OPENR/OPENR.h>
#include <OPENR/ODataFormats.h>

//
// OPrimitiveRequest
//
const OPrimitiveRequest oprmreqCOMMON_BASE         =   0;
const OPrimitiveRequest oprmreqSPEAKER_BASE        = 128;
const OPrimitiveRequest oprmreqMIC_BASE            = 256;
const OPrimitiveRequest oprmreqCAM_BASE            = 384;
const OPrimitiveRequest oprmreqBT_BASE             = 512;
const OPrimitiveRequest oprmreqJOINT2_BASE         = 640;
const OPrimitiveRequest oprmreqLED2_BASE           = 768;
const OPrimitiveRequest oprmreqSENSOR_BASE         = 896;
const OPrimitiveRequest oprmreqLED3_BASE           = 1024;
const OPrimitiveRequest oprmreqJOINT4_BASE         = 1152;
const OPrimitiveRequest oprmreqPRIVATE_BASE        = 1048576;


//
// Speaker
//
const OPrimitiveRequest oprmreqSPEAKER_MUTE_ON         = oprmreqSPEAKER_BASE+0;
const OPrimitiveRequest oprmreqSPEAKER_MUTE_OFF        = oprmreqSPEAKER_BASE+1;
const OPrimitiveRequest oprmreqSPEAKER_SET_VOLUME      = oprmreqSPEAKER_BASE+2;
const OPrimitiveRequest oprmreqSPEAKER_SET_SOUND_TYPE  = oprmreqSPEAKER_BASE+3;
const OPrimitiveRequest oprmreqSPEAKER_GET_SOUND_TYPE  = oprmreqSPEAKER_BASE+4;
const OPrimitiveRequest oprmreqSPEAKER_SET_VOLUME_MIDI = oprmreqSPEAKER_BASE+5;
const OPrimitiveRequest oprmreqSPEAKER_MASTER_TUNING_MIDI = oprmreqSPEAKER_BASE+6;

typedef longword OSpeakerSoundType;
const OSpeakerSoundType ospksndMONO8K8B      = 0;
const OSpeakerSoundType ospksndMONO11K8B     = 1; // FUTURE RESERVED
const OSpeakerSoundType ospksndMONO16K8B     = 2; // FUTURE RESERVED 
const OSpeakerSoundType ospksndMONO16K16B    = 3;
const OSpeakerSoundType ospksndSTEREO16K16B  = 4; // FUTURE RESERVED 

typedef longword OSpeakerVolume;
const OSpeakerVolume ospkvolinfdB = 0x8000; // Volume 0 (min)
const OSpeakerVolume ospkvol25dB  = 0xe700; // Volume 1
const OSpeakerVolume ospkvol18dB  = 0xee00; // Volume 2
const OSpeakerVolume ospkvol10dB  = 0xf600; // Volume 3 (max)

struct OPrimitiveControl_SpeakerVolume {
    longword volume;

    OPrimitiveControl_SpeakerVolume(longword vol) : volume(vol) {}
};

struct OPrimitiveControl_SpeakerSoundType {
    OSpeakerSoundType type;

    OPrimitiveControl_SpeakerSoundType(OSpeakerSoundType t) : type(t) {}
};

struct OPrimitiveControl_MasterTuningMIDI {
    sword value;

    OPrimitiveControl_MasterTuningMIDI(sword v) : value(v) {}
};


//
// Mic
//
const OPrimitiveRequest oprmreqMIC_UNI          = oprmreqMIC_BASE + 0;
const OPrimitiveRequest oprmreqMIC_OMNI         = oprmreqMIC_BASE + 1;
const OPrimitiveRequest oprmreqMIC_ALC_ON       = oprmreqMIC_BASE + 2;
const OPrimitiveRequest oprmreqMIC_ALC_OFF      = oprmreqMIC_BASE + 3;


//
// Camera
//
const OPrimitiveRequest oprmreqCAM_SET_WHITE_BALANCE = oprmreqCAM_BASE + 0;
const OPrimitiveRequest oprmreqCAM_SET_GAIN          = oprmreqCAM_BASE + 1;
const OPrimitiveRequest oprmreqCAM_SET_SHUTTER_SPEED = oprmreqCAM_BASE + 2;
const OPrimitiveRequest oprmreqCAM_GET_CDT_TABLE     = oprmreqCAM_BASE + 3;
const OPrimitiveRequest oprmreqCAM_AE_ON             = oprmreqCAM_BASE + 4;
const OPrimitiveRequest oprmreqCAM_AE_OFF            = oprmreqCAM_BASE + 5;
const OPrimitiveRequest oprmreqCAM_AWB_ON            = oprmreqCAM_BASE + 6;
const OPrimitiveRequest oprmreqCAM_AWB_OFF           = oprmreqCAM_BASE + 7;

const longword ocamparamUNDEF           = 0;
const longword ocamparamWB_INDOOR_MODE  = 1;
const longword ocamparamWB_OUTDOOR_MODE = 2;
const longword ocamparamWB_FL_MODE      = 3; // Fluorescent Lamp

const longword ocamparamGAIN_LOW        = 1;
const longword ocamparamGAIN_MID        = 2;
const longword ocamparamGAIN_HIGH       = 3;

const longword ocamparamSHUTTER_SLOW    = 1;
const longword ocamparamSHUTTER_MID     = 2;
const longword ocamparamSHUTTER_FAST    = 3;

struct OPrimitiveControl_CameraParam {
    longword camparam;

    OPrimitiveControl_CameraParam() : camparam(ocamparamUNDEF) {}
    OPrimitiveControl_CameraParam(longword param) : camparam(param) {}
};

struct OPrimitiveControl_Cdt {
    longword index;
    byte     table[100];

    OPrimitiveControl_Cdt() : index(0) {}
    OPrimitiveControl_Cdt(longword idx) : index(idx) {}
};


//
// Bluetooth
//
const OPrimitiveRequest oprmreqBT_ATTACH = oprmreqBT_BASE + 0;
const OPrimitiveRequest oprmreqBT_DETACH = oprmreqBT_BASE + 1;


//
// Joint2
//
const OPrimitiveRequest oprmreqJOINT2_SET_COMMAND = oprmreqJOINT2_BASE + 0;
const OPrimitiveRequest oprmreqJOINT2_GET_VALUE   = oprmreqJOINT2_BASE + 1;

struct OPrimitiveControl_JointCommandValue2 {
    OJointCommandValue2 command;

    OPrimitiveControl_JointCommandValue2(slongword v) { command.value = v; }
};

struct OPrimitiveControl_JointValue {
    OJointValue jval;

    OPrimitiveControl_JointValue() : jval() {}
    OPrimitiveControl_JointValue(slongword val,
				 word sig,
				 sword pwm,
				 slongword rv,
				 word rs) : jval(val, sig, pwm, rv, rs) {}
};


//
// LED2
//
const OPrimitiveRequest oprmreqLED2_SET_COMMAND = oprmreqLED2_BASE + 0;

struct OPrimitiveControl_LEDCommandValue2 {
    OLEDCommandValue2 command;
    
    OPrimitiveControl_LEDCommandValue2(OLEDValue l) {
        command.led    = l;
        command.period = 0;     // NOT USED
    }
};


//
// Sensor
//
const OPrimitiveRequest oprmreqSENSOR_WATCH_BT_SWITCH = oprmreqSENSOR_BASE + 0;

typedef longword OBtSwitchEvent;
const OBtSwitchEvent obtswitcheventUNDEF          = 0;
const OBtSwitchEvent obtswitcheventVALUE_CHANGED  = 1;

struct OPrimitiveControl_BtSwitchWatcherInfo {
    OBtSwitchEvent event;
    OServiceEntry  entry;

    OPrimitiveControl_BtSwitchWatcherInfo(OBtSwitchEvent ev,
                                          const OServiceEntry& e) {
        event = ev;
        entry = e;
    }
};

struct OPrimitiveControl_BtSwitchEvent {
    OBtSwitchEvent event;
    OSwitchValue   value;

    OPrimitiveControl_BtSwitchEvent(OBtSwitchEvent ev, OSwitchValue val) {
        event = ev;
        value = val;
    }
};


//
// LED3
//
const OPrimitiveRequest oprmreqLED3_SET_COMMAND = oprmreqLED3_BASE + 0;

struct OPrimitiveControl_LEDCommandValue3 {
    OLEDCommandValue3 command;
    
    OPrimitiveControl_LEDCommandValue3(sword b, OLED3Mode m) {
        command.intensity = b;
        command.mode      = m;
        command.period    = 0; // NOT USED
    }
};


//
// Joint4
//
const OPrimitiveRequest oprmreqJOINT4_SET_COMMAND = oprmreqJOINT4_BASE + 0;

struct OPrimitiveControl_JointCommandValue4 {
    OJointCommandValue4 command;

    OPrimitiveControl_JointCommandValue4(OJointValue4 v) {
        command.value  = v;
        command.period = 1;
    }
};

#endif /* _OPrimitiveControl_h_DEFINED */
