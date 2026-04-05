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

#ifndef _OMTouchSensorData_h_DEFINED
#define _OMTouchSensorData_h_DEFINED

#include <OPENR/OPENR.h>
#include <OPENR/ObjcommTypes.h>  // for ObserverID

const unsigned int MAX_NUMBER_OF_SWITHES = 32;

typedef unsigned int omTouchSensor;
extern const omTouchSensor HEAD_SW;
extern const omTouchSensor RF_LEG_SW;
extern const omTouchSensor LF_LEG_SW;
extern const omTouchSensor RR_LEG_SW;
extern const omTouchSensor LR_LEG_SW;

enum Event_Mode{
    OFF_ON,
    ON_OFF
};

struct Param{
    omTouchSensor off_on;
    omTouchSensor on_off;
    Param(void) : off_on(0), on_off(0) {}
};
    
class OMTouchSensorParam{
    ObserverID observerID;
    Param      param;

public:
    OMTouchSensorParam(void) : param(), observerID(){}
    ~OMTouchSensorParam(){}

    OStatus All_SetParam();
    OStatus SetParam(Event_Mode event, omTouchSensor touch_sw);
    OStatus UnsetParam(Event_Mode event, omTouchSensor touch_sw);
    OStatus All_UnsetParam();
    OStatus SetID( ObserverID id );

    omTouchSensor Off_On(void) const;
    omTouchSensor On_Off(void) const;
    const Param&  Get_Param(void) const;
    const ObserverID& Observer_ID(void) const;
};

enum Touch_State{
    mode_OFF_ON,                     
    mode_ON_OFF,                     
    mode_ON_ON,                      
    mode_OFF_OFF,                    
    mode_UNKNOWN                     
};

struct State{
    unsigned int  push_time;
    float         avg_pressure;
    float         max_pressure;
    Touch_State   touch_state;

    State(void) : push_time(0), avg_pressure(0), touch_state(mode_OFF_OFF){}
};

class OMTouchSensorResult{
friend class OMTouchSensor;
    unsigned int TimeStamp;
    unsigned int num_of_switches;
    State state[MAX_NUMBER_OF_SWITHES];

public:
    OMTouchSensorResult(void):TimeStamp(0), num_of_switches(0){}
    ~OMTouchSensorResult(){}

    unsigned int Get_Time_Stamp(void) const;
    int Get_Push_Time(omTouchSensor touch_sw) const; 
    float Get_Avg_Pressure(omTouchSensor touch_sw) const; 
    float Get_Max_Pressure(omTouchSensor touch_sw) const; 
    Touch_State Get_Touch_State(omTouchSensor touch_sw) const; 
    void Show_Result(void);
};

#endif // _OMTouchSensorData_h_DEFINED
