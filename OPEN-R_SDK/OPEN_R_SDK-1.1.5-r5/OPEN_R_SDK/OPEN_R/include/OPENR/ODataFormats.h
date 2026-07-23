//
// ODataFormats.h
//
// Copyright 1997,1998,1999,2000,2001,2002,2003 Sony Corporation
//

#ifndef _ODataFormats_h_DEFINED
#define _ODataFormats_h_DEFINED

#include <OPENR/OPENR.h>
#include <OPENR/OUnits.h>

//
// ODataType
//
typedef longword ODataType;
const ODataType odataUNDEF                =    0; // UNDEFINED
const ODataType odataNO_DIMENSION         =    1; // No dimension
const ODataType odataLENGTH               =    2; // 10^-6 m
const ODataType odataMASS                 =    3; // 10^-6 kg
const ODataType odataTIME                 =    4; // 10^-6 sec
const ODataType odataDATE                 =    5; // GMT (time_t)
const ODataType odataANGULAR              =    6; // 10^-6 rad
const ODataType odataVELOCITY             =    7; // 10^-6 m/s
const ODataType odataFORCE                =    8; // 10^-6 N
const ODataType odataACCELERATION         =    9; // 10^-6 m/s2
const ODataType odataANGULAR_VELOCITY     =   10; // 10^-6 rad/s
const ODataType odataTEMPERATURE          =   11; // 10^-6 Celsius
const ODataType odataPRESSURE             =   12; // 10^-6 Pa (N/m2)
const ODataType odataPERCENTAGE           =   13; // 10^-6 %
const ODataType odataSWITCH               =   14; // oswitchON or oswitchOFF
const ODataType odataLED                  =   15; // oledON or oledOFF
const ODataType odataSOUND                =   16; // Sound data
const ODataType odataIMAGE                =   17; // Image data
const ODataType odataJOINT_VALUE          =   18; // 10^-6 rad or 10^-6 m
const ODataType odataFBK_YCrCb            =   19; // FBK YCrCb Image
const ODataType odataFBK_CDT              =   20; // FBK CDT Image
const ODataType odataCDT                  =   21; // Color Detection Table
const ODataType odataPRIMITIVE_INFO       =   22; // struct OPrimitiveInfo
const ODataType odataCPC_INFO             =   23; // struct OCPCInfo
const ODataType odataFBK_YCrCb_HPF        =   24; // FBK YCrCb HPF Image

const ODataType odataJOINT_COMMAND        =  256; // 10^-6 rad or 10^-6 m
const ODataType odataCAMERA_COMMAND       =  257; // Camera Command
const ODataType odataLED_COMMAND          =  258; // LED Command
const ODataType odataJOINT_COMMAND2       =  259; // 10^-6 rad or 10^-6 m
const ODataType odataLED_COMMAND2         =  260; // LED2 Command
const ODataType odataJOINT_COMMAND3       =  261; // Joint3 Command
const ODataType odataJOINT_COMMAND4       =  262; // Joint4 Command
const ODataType odataLED_COMMAND3         =  263; // LED3 Command

const ODataType odataRAW_JOINT_COMMAND    =  512; // Raw Joint Command
const ODataType odataRAW_CAMERA_COMMAND   =  513; // Raw Camera Command
const ODataType odataRAW_LED_COMMAND      =  514; // Raw LED Command
const ODataType odataRAW_JOINT_COMMAND2   =  515; // Raw Joint Command

const ODataType odataCOMMAND_VECTOR       = 1024;
const ODataType odataSENSOR_FRAME_VECTOR  = 1025;
const ODataType odataSOUND_VECTOR         = 1026;
const ODataType odataFBKIMAGE_VECTOR      = 1027;
const ODataType odataCDT_VECTOR           = 1028;
const ODataType odataPRIMITIVE_MAP        = 1029;
const ODataType odataCPC_TREE             = 1030;
const ODataType odataDONT_CARE            = 0xffffffff;	// DON'T CARE

typedef longword OSwitchValue;
const OSwitchValue oswitchOFF = 0;
const OSwitchValue oswitchON  = 1;

typedef longword OLEDValue;
const OLEDValue oledOFF = 0;
const OLEDValue oledON  = 1;

typedef word OLED3Mode;
const OLED3Mode oled3_MODE_UNDEF = 0xffff;
const OLED3Mode oled3_MODE_A     = 0;
const OLED3Mode oled3_MODE_B     = 1;

typedef longword OJointValue3;
const OJointValue3 ojoint3_STATE_UNDEF = 0xffffffff;
const OJointValue3 ojoint3_STATE0      = 0;
const OJointValue3 ojoint3_STATE1      = 1;

typedef longword OJointValue4;
const OJointValue4 ojoint4_STATE_UNDEF = 0xffffffff;
const OJointValue4 ojoint4_STATE0      = 0;
const OJointValue4 ojoint4_STATE1      = 1;

//
// ocamreqAE_OFF
//    param[0] : AGC GAIN MAX (0 - 255)
//    param[1] : AGC GAIN MIN (0 - 255)
//
// ocamreqAE_ON
//    param[0] : AGC GAIN MAX (0 - 255)
//    param[1] : AGC GAIN MIN (0 - 255)
//
// ocamreqSET_AWB_MODE
//    param[0] : ocamparamAWB_XXX_MODE
//
// ocamreqDRAW_BOX
//    param[0] : box (ocamparamBOX1 or ocamparamBOX2)
//    param[1] : x0  (Top Left point)
//    param[2] : y0  (Top Left point)
//    param[3] : width
//    param[4] : height
//
typedef word OCameraRequest;
const OCameraRequest ocamreqAE_OFF       = 0;
const OCameraRequest ocamreqAE_ON        = 1;
const OCameraRequest ocamreqSET_AWB_MODE = 2;
const OCameraRequest ocamreqDRAW_BOX     = 3;

// Auto White Balance Mode
const byte ocamparamAWB_AUTO_MODE    = 0;
const byte ocamparamAWB_INDOOR_MODE  = 1;
const byte ocamparamAWB_OUTDOOR_MODE = 2;
const byte ocamparamAWB_USER_MODE    = 3;
const byte ocamparamAWB_FL_MODE      = 4; // Fluorescent Lamp

// Box ID
const byte ocamparamBOX1 = 1;
const byte ocamparamBOX2 = 2;


//
//----------------------------------------------------------------------
// ODataVectorInfo
//----------------------------------------------------------------------
//
const size_t odataOPTIONAL_MAX = 128;

struct ODataVectorInfo {	                      // 192 bytes (total)
    MemoryRegionID   memRegionID;                 //   4 bytes
    void*            physAddr;	                  //   4 bytes
    size_t           offset;                      //   4 bytes
    size_t           totalSize;                   //   4 bytes
    ODataType        type;                        //   4 bytes
    size_t           infoOffset;                  //   4 bytes
    size_t           infoSize;	                  //   4 bytes
    size_t           maxNumData;                  //   4 bytes
    size_t           numData;	                  //   4 bytes
    OVRSyncKey       syncKey;                     //   4 bytes
    longword         wait;                        //   4 bytes
    size_t           optOffset;                   //   4 bytes
    size_t           optSize;                     //   4 bytes
    longword         padding[3];                  //  12 bytes
    byte             optional[odataOPTIONAL_MAX]; // 128 bytes
};


//
//----------------------------------------------------------------------
// OCommandVectorData
//----------------------------------------------------------------------
//
const size_t ocommandMAX_FRAMES = 16;

struct OCommandValue {
    slongword  value[2];
};


struct OJointCommandValue {
    slongword  value;
    byte       p;
    byte       i;
    byte       d;
    byte       padding;

    void Set(slongword val, byte kp, byte ki, byte kd);
};


struct OJointCommandValue2 {
    slongword  value;
    slongword  padding;
};


struct OJointCommandValue3 {
    OJointValue3  value;
    word          reserved;	// Reserved for OVirtualRobot
    word          padding;	
};


struct OJointCommandValue4 {
    OJointValue4  value;
    word          period;		// 1 period : 1 frame (8ms)
    word          reserved;     // Reserved for OVirtualRobot
};


struct OCameraCommandValue {
    OCameraRequest  request;
    byte            param[6];
};


struct OLEDCommandValue {
    OLEDValue  led;
    word       period;		// 1 period : 16 frames (128ms)
    word       padding;
};


struct OLEDCommandValue2 {
    OLEDValue  led;
    word       period;		// 1 period : 1 frame (8ms)
    word       reserved;	// Reserved for OVirtualRobot
};


struct OLEDCommandValue3 {
    sword      intensity;   // 0 - 255
    OLED3Mode  mode;
    word       period;      // 1 period : 1 frame (8ms)
    word       reserved;    // Reserved for OVirtualRobot
};


struct ORawJointCommandValue {
    word       signal;
    byte       p;
    byte       i;
    byte       d;
    byte       padding[3];

    void Set(word sig, byte kp, byte ki, byte kd);
};


struct ORawJointCommandValue2 {
    word       signal;
    word       padding[3];
};


struct ORawCameraCommandValue {
    word       signal;
    word       padding[3];

    void Set(word sig) { signal = sig; }
};


struct ORawLEDCommandValue {
    word       signal;
    word       padding[3];

    void Set(word sig) { signal = sig; }
};


struct OCommandInfo {		    // 32 bytes (total)
    ODataType      type;	    //  4 bytes
    OPrimitiveID   primitiveID;	//  4 bytes
    longword       frameNumber;	//  4 bytes
    size_t         numFrames;	//  4 bytes
    size_t         frameSize;	//  4 bytes
    size_t         dataOffset;	//  4 bytes
    size_t         dataSize;	//  4 bytes
    longword       padding[1];	//  4 bytes

    void Set(ODataType t, OPrimitiveID id, size_t nframes) {
        type        = t;
        primitiveID = id;
        numFrames   = nframes;
    }
};


struct OCommandData {                         //    128 bytes (total)
    OCommandValue  value[ocommandMAX_FRAMES]; // 16 * 8 bytes
};


//
// struct OCommandVectorData {
//     ODataVectorInfo   vectorInfo;
//     OCommandInfo  info[n];
//     OCommandData  data[n];
// };
//
struct OCommandVectorData {
    ODataVectorInfo  vectorInfo;
    OCommandInfo     info[1];

    void SetNumData(size_t ndata)    { vectorInfo.numData = ndata; }
    OCommandInfo* GetInfo(int index) { return &info[index];        }
    OCommandData* GetData(int index) {
        return (OCommandData*)((byte*)&vectorInfo + info[index].dataOffset);
    }
};


//
//----------------------------------------------------------------------
// OSensorFrameVectorData
//----------------------------------------------------------------------
//
const size_t    osensorframeMAX_FRAMES     = 16;
const word      osensorframeINVALID_SIGNAL = 0xffff;
const slongword opsdINVALID_VALUE          = 0;

struct OSensorValue {           // total 16 bytes
    slongword  value;
    word       signal;
    word       padding[5];
};


struct OAcceleration {          // total 16 bytes
    slongword  value;
    word       signal;
    word       padding[5];
};


struct OAngularVelocity {       // total 16 bytes
    slongword  value;
    word       signal;
    word       padding[5];
};


struct OTemperature {           // total 16 bytes
    slongword  value;
    word       signal;
    word       padding[5];
};


struct OForce {              // total 16 bytes
    slongword  value;
    word       signal;
    word       padding[5];
};


struct OPressure {              // total 16 bytes
    slongword  value;
    word       signal;
    word       padding[5];
};


struct OLength {                // total 16 bytes
    slongword  value;
    word       signal;
    word       padding[5];
};


struct OSwitchStatus {          // total 16 bytes
    OSwitchValue  value;
    word          signal;
    word          padding[5];
};


struct OJointValue {            // total 16 bytes
    slongword  value;
    word       signal;
    sword      pwmDuty;
    slongword  refValue;
    word       refSignal;
    word       padding[1];

    OJointValue() {}
    OJointValue(slongword val, word sig, sword pwm, slongword rv, word rs) {
        value     = val;
        signal    = sig;
        pwmDuty   = pwm;
        refValue  = rv;
        refSignal = rs;
    }
};


struct OSensorFrameInfo {	    // 32 bytes (total)
    ODataType      type;	    //  4 bytes
    OPrimitiveID   primitiveID;	//  4 bytes
    longword       frameNumber;	//  4 bytes
    size_t         numFrames;	//  4 bytes
    size_t         frameSize;	//  4 bytes
    size_t         dataOffset;	//  4 bytes
    size_t         dataSize;	//  4 bytes
    longword       padding[1];	//  4 bytes

    void Set(ODataType t, OPrimitiveID id, size_t nframes) {
        type        = t;
        primitiveID = id;
        numFrames   = nframes;
    }
};


struct OSensorFrameData {                         //     256 bytes (total)
    OSensorValue   frame[osensorframeMAX_FRAMES]; // 16 * 16 bytes
};


//
// struct OSensorFrameVectorData {
//     ODataVectorInfo   vectorInfo;
//     OSensorFrameInfo  info[n];
//     OSensorFrameData  data[n];
// };
//
struct OSensorFrameVectorData {
    ODataVectorInfo   vectorInfo;
    OSensorFrameInfo  info[1];

    void SetNumData(size_t ndata)        { vectorInfo.numData = ndata; }
    OSensorFrameInfo* GetInfo(int index) { return &info[index];        }
    OSensorFrameData* GetData(int index) { 
        return (OSensorFrameData*)((byte*)&vectorInfo+info[index].dataOffset);
    }
};



//
//----------------------------------------------------------------------
// OSoundVectorData
//----------------------------------------------------------------------
//
typedef word OSoundFormat;
const OSoundFormat osoundformatUNDEF     = 0;
const OSoundFormat osoundformatPCM       = 1;
const OSoundFormat osoundformatUSB_MIDI  = 2;
const OSoundFormat osoundformatSMF       = 3; // STANDARD MIDI FILE


typedef word OSoundChannel;
const OSoundChannel osoundchannelUNDEF  = 0;
const OSoundChannel osoundchannelMONO   = 1;
const OSoundChannel osoundchannelSTEREO = 2;

struct OSoundInfo {		                 // 64 bytes (total)
    ODataType            type;	         //  4 bytes
    OPrimitiveID         primitiveID;    //  4 bytes
    longword             frameNumber;    //  4 bytes
    size_t               frameSize;      //  4 bytes
    size_t               dataOffset;	 //  4 bytes
    size_t               maxDataSize;	 //  4 bytes
    size_t               dataSize;       //  4 bytes
    OSoundFormat         format;         //  2 bytes
    OSoundChannel        channel;        //  2 bytes
    word                 samplingRate;   //  2 bytes
    word                 bitsPerSample;  //  2 bytes
    size_t               actualDataSize; //  4 bytes
    longword             padding[6];	 // 24 bytes

    void Set(ODataType t, OPrimitiveID id, size_t dsize) {
        type        = t;
        primitiveID = id;
        dataSize    = dsize;
    }
};


struct OSoundSampleStereo16 {
    sword L;
    sword R;
};


//
// struct OSoundVectorData {
//     ODataVectorInfo vectorInfo;
//     OSoundInfo      info[n];
//     OSoundData      data[n];
// };
//
struct OSoundVectorData {
    ODataVectorInfo  vectorInfo;
    OSoundInfo       info[1];

    void SetNumData(size_t ndata)  { vectorInfo.numData = ndata; }
    OSoundInfo* GetInfo(int index) { return &info[index];        }
    byte*       GetData(int index) {
        return ((byte*)&vectorInfo + info[index].dataOffset);
    }
};


//
//----------------------------------------------------------------------
// OFbkImageVectorData
//----------------------------------------------------------------------
//
struct OFbkImageInfo {          // 32 bytes (total)
    ODataType     type;         //  4 bytes
    OPrimitiveID  primitiveID;  //  4 bytes
    longword      frameNumber;  //  4 bytes
    size_t        dataOffset;   //  4 bytes
    size_t        dataSize;     //  4 bytes
    size_t        width;        //  4 bytes
    size_t        height;       //  4 bytes
    size_t        padding[1];   //  4 bytes
};

struct OFbkImageVectorData {
    ODataVectorInfo  vectorInfo;
    OFbkImageInfo    info[1];

    void SetPrimitiveID(OPrimitiveID primitiveID) {
        for (size_t i = 0; i < vectorInfo.numData; i++)
            info[i].primitiveID = primitiveID;
    }
    OFbkImageInfo* GetInfo(int index) { return &info[index]; }
    byte*          GetData(int index) {
        return ((byte*)&vectorInfo + info[index].dataOffset);
    }
};

typedef int OFbkImageLayer;
const OFbkImageLayer ofbkimageLAYER_H = 0;
const OFbkImageLayer ofbkimageLAYER_M = 1;
const OFbkImageLayer ofbkimageLAYER_L = 2;
const OFbkImageLayer ofbkimageLAYER_C = 3;


//
//----------------------------------------------------------------------
// OCdtVectorData
//----------------------------------------------------------------------
//
const int ocdtNUM_CHANNELS  =  8;
typedef longword OCdtChannel;
const OCdtChannel ocdtCHANNEL_UNDEF = 0x00;
const OCdtChannel ocdtCHANNEL0      = 0x01;
const OCdtChannel ocdtCHANNEL1      = 0x02;
const OCdtChannel ocdtCHANNEL2      = 0x04;
const OCdtChannel ocdtCHANNEL3      = 0x08;
const OCdtChannel ocdtCHANNEL4      = 0x10;
const OCdtChannel ocdtCHANNEL5      = 0x20;
const OCdtChannel ocdtCHANNEL6      = 0x40;
const OCdtChannel ocdtCHANNEL7      = 0x80;

const int ocdtMAX_Y_SEGMENT    = 32;
const longword ocdtINIT        = 0x80808080;
const longword ocdtCr_MAX_MASK = 0x0000ff00;
const longword ocdtCr_MIN_MASK = 0x000000ff;
const longword ocdtCb_MAX_MASK = 0xff000000;
const longword ocdtCb_MIN_MASK = 0x00ff0000;

struct OCdtInfo {		                    // 144 bytes
    ODataType     type;                     //   4 bytes
    OPrimitiveID  primitiveID;              //   4 bytes
    OCdtChannel   channel;		            //   4 bytes
    longword      table[ocdtMAX_Y_SEGMENT]; // 128 bytes
    longword      padding;		            //   4 bytes
    
    void Init(OPrimitiveID prmID, OCdtChannel chan) {
        type        = odataCDT;
        primitiveID = prmID;
        channel     = chan;
        for (int i = 0; i < ocdtMAX_Y_SEGMENT; i++) table[i] = ocdtINIT;
    }
    void Set(int y_segment,
             byte cr_max, byte cr_min, byte cb_max, byte cb_min) {
        longword crMax = (longword)cr_max;
        longword crMin = (longword)cr_min;
        longword cbMax = (longword)cb_max;
        longword cbMin = (longword)cb_min;
        crMax = (crMax <<  8) & ocdtCr_MAX_MASK;
        crMin = (crMin      ) & ocdtCr_MIN_MASK;
        cbMax = (cbMax << 24) & ocdtCb_MAX_MASK;
        cbMin = (cbMin << 16) & ocdtCb_MIN_MASK;
        table[y_segment] =  crMax | crMin | cbMax | cbMin;
    }
};

struct OCdtVectorData{
    ODataVectorInfo  vectorInfo;
    OCdtInfo         info[ocdtNUM_CHANNELS];

    void SetNumData(size_t ndata) { vectorInfo.numData = ndata; }
    OCdtInfo* GetInfo(int index)  { return &info[index];        }
};


//
//----------------------------------------------------------------------
// OPrimitiveMapData
//----------------------------------------------------------------------
//
struct OPrimitiveInfo {	                        // 160 bytes (total)
    ODataType         type;                     //   4 bytes
    OPrimitiveID      primitiveID;              //   4 bytes
    OPrimitiveType    primitiveType;            //   4 bytes
    OSensorType       sensorType;               //   4 bytes
    OStatus           primitiveStatus;	        //   4 bytes
    longword          padding[3];               //  12 bytes
    char              locator[ovrPATH_MAX + 1];	// 128 bytes
};


//
// struct OPrimitiveMapData {
//     ODataVectorInfo   vectorInfo;
//     OPrimitiveInfo    info[n];
// };
//
struct OPrimitiveMapData {
    ODataVectorInfo  vectorInfo;
    OPrimitiveInfo   info[1];

    OPrimitiveInfo* GetInfo(int index) { return &info[index]; }
};


//
//----------------------------------------------------------------------
// OCPCTreeData
//----------------------------------------------------------------------
//
struct OCPCInfo {            // 12 bytes (total)
    ODataType  type;	     //  4 bytes
    word       cpcClass;     //  2 bytes
    word       cpcSubClass;  //  2 bytes
    word       cpcType;	     //  2 bytes
    word       numReceptors; //  2 bytes
};


//
// struct OCPCTreeData {
//     ODataVectorInfo   vectorInfo;
//     OPrimitiveInfo    info[n];
// };
//
struct OCPCTreeData {
    ODataVectorInfo  vectorInfo;
    OCPCInfo         info[1];

    OCPCInfo* GetInfo(int index) { return &info[index]; }
};

#endif /* _ODataFormats_h_DEFINED */
