//
// OPower.h
//
// Copyright 1998,1999,2000,2001,2002 Sony Corporation
//

#ifndef _OPower_h_DEFINED
#define _OPower_h_DEFINED

#include <Types.h>
#include <time.h>
#include <OPENR/OTime.h>

typedef bool OPower;
const OPower opowerON  = true;
const OPower opowerOFF = false;

//
// Robot Status
//

// Robot Status Bitmap

const longword orsbPAUSE                         = 0x00000001;
const longword orsbMOTOR_POWER                   = 0x00000002;
const longword orsbVIBRATION_DETECT              = 0x00000004;
const longword orsbEX_PORT_CONNECTED             = 0x00000008;
const longword orsbSTATION_CONNECTED             = 0x00000010;
const longword orsbEX_POWER_CONNECTED            = 0x00000020;
const longword orsbBATTERY_CONNECTED             = 0x00000040;
const longword orsbBATTERY_CHARGING              = 0x00000080;
const longword orsbBATTERY_CAPACITY_FULL         = 0x00000100;
const longword orsbBATTERY_CAPACITY_LOW          = 0x00000200;
const longword orsbBATTERY_OVER_CURRENT          = 0x00000400;
const longword orsbBATTERY_OVER_TEMP_DISCHARGING = 0x00000800;
const longword orsbBATTERY_OVER_TEMP_CHARGING    = 0x00001000;
const longword orsbBATTERY_ERROR_OF_CHARGING     = 0x00002000;
const longword orsbERROR_OF_PLUNGER              = 0x00004000;
const longword orsbOPEN_R_POWER_GOOD             = 0x00008000;
const longword orsbERROR_OF_FAN                  = 0x00010000;
const longword orsbDATA_STREAM_FROM_STATION      = 0x00020000;
const longword orsbREGISTER_UPDATED_BY_STATION   = 0x00040000;
const longword orsbRTC_ERROR                     = 0x00080000;
const longword orsbRTC_OVERFLOW                  = 0x00100000;
const longword orsbRTC_RESET                     = 0x00200000;
const longword orsbRTC_SET                       = 0x00400000;
const longword orsbSPECIAL_MODE                  = 0x00800000;
const longword orsbBMN_DEBUG_MODE                = 0x01000000;
const longword orsbCHARGER_STATUS                = 0x02000000;
const longword orsbPLUNGER                       = 0x04000000;
const longword orsbSUSPENDED                     = 0x08000000;
const longword orsbSPECIAL_DATA_READ_REQ         = 0x10000000;

const longword orsbALL                           = 0x1FFFFFFF;


//
// Boot Condition
//

// Boot Condition Bitmap
const word obcbBOOT_TIMER                = 0x0001;
const word obcbVIBRATION_DETECTED        = 0x0002;
const word obcbPAUSE_SW                  = 0x0004;
const word obcbSTATION_CONNECTED         = 0x0008;
const word obcbSTATION_DISCONNECTED      = 0x0010;
const word obcbBATTERY_CAPACITY_FULL     = 0x0020;
const word obcbREQ_FROM_STATION          = 0x0040; // FUTURE RESERVED

const word obcbALL                       = 0x007F;


//
// Battery Status
//

// Battery Status Bitmap
const word obsbERROR_CODE_MASK              = 0x000F;
const word obsbFULLY_DISCHARGED             = 0x0010;
const word obsbFULLY_CHARGED                = 0x0020;
const word obsbDISCHARGING                  = 0x0040;
const word obsbINITIALIZED                  = 0x0080;
const word obsbREMAINING_TIME_ALARM         = 0x0100;
const word obsbREMAINING_CAPACITY_ALARM     = 0x0200;
const word obsbRESERVED0                    = 0x0400;
const word obsbTERMINATED_DISCHARGING_ALARM = 0x0800;
const word obsbOVER_TEMP_ALARM              = 0x1000;
const word obsbRESERVED1                    = 0x2000;
const word obsbTERMINATED_CHARGING_ALARM    = 0x4000;
const word obsbOVER_CHARGED_ALARM           = 0x8000;

const word obsbALL                          = 0xFFFF;


// Battery Status Error Code
const longword obsecOK                  = 0;
const longword obsecBUSY                = 1;
const longword obsecNOT_READY           = 2;
const longword obsecUNSUPPORTED_COMMAND = 3;
const longword obsecACCESS_DENIED       = 4;
const longword obsecOVER_OR_UNDER_FLOW  = 5;
const longword obsecBAD_SIZE            = 6;
const longword obsecUNKNOWN             = 7;

// Data Stream Type
// BMN Data Stream Type
const byte odstSOUND                = 0x00;
const byte odstLCD                  = 0x01;
const byte odstSTATION_WRITE        = 0x02;
const byte odstSTATION_READ         = 0x03;
const byte odstBATTERY_DEVICE_NAME  = 0x04;
const byte odstSPECIAL_DATA         = 0x05;

//
const word opsoREMAINING_CAPACITY_NOTIFY_EVERY_CHANGE = 0xFFFF;
const word opsoREMAINING_CAPACITY_NOT_NOTIFY          = 0xFFFE;

const word opsoTEMPERATURE_NOTIFY_EVERY_CHANGE        = 0xFFFF;
const word opsoTEMPERATURE_NOT_NOTIFY                 = 0xFFFE;

const sbyte opsoTIME_DIF_NOTIFY_EVERY_CHANGE          = 0x7F;
const sbyte opsoTIME_DIF_NOT_NOTIFY                   = 0x7E;

const byte opsoVOLUME_NOTIFY_EVERY_CHANGE             = 0xFF;
const byte opsoVOLUME_NOT_NOTIFY                      = 0xFE;

struct OPowerStatus {
    longword robotStatus;
    word     batteryStatus;
    word     remainingCapacity;
    word     temperature;

    word     fullyChargedCapacity;
    word     voltage;
    sword    current;

    sbyte    timeDif;
    byte     volume;

    OPowerStatus();
    OPowerStatus(longword rs, word bs, word rc, word tmpr, word fc,
                 word vlt, word crnt, sbyte td, byte vlm);
    OPowerStatus(const OPowerStatus& ps);

    void Set(longword rs, word bs, word rc, word tp, sbyte td, byte v);

    OPowerStatus& operator=(const OPowerStatus& ps);

    void Print() const;
    void PrintDetail() const;
};

// Boot Time Type
const longword obcbttRELATIVE = 0x00000000;
const longword obcbttABSOLUTE = 0x80000000;

struct OBootCondition {
    word     bitmap;
    time_t   bootTime;          // relative or absolute time.
    longword bootTimeType;      // relative or absolute
    byte     vibrationLevel;

    OBootCondition();
    OBootCondition(word bm);
    OBootCondition(word bm, time_t bt, longword btt);
    OBootCondition(word bm, time_t bt, longword btt, byte vl);

    OBootCondition(const OBootCondition& bc);

    OBootCondition& operator=(const OBootCondition& bc);
};

#endif // _OPower_h_DEFINED
