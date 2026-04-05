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

#ifndef _OMGsensorData_h_DEFINED
#define _OMGsensorData_h_DEFINED

typedef unsigned int omGsensor;
const omGsensor OMG_UNKNOWN         = 0x00000001;
const omGsensor FALL_DOWN_FRONT     = 0x00000002;
const omGsensor FALL_DOWN_RIGHT     = 0x00000004;
const omGsensor FALL_DOWN_LEFT      = 0x00000008;
const omGsensor FALL_DOWN_REAR      = 0x00000010;
const omGsensor HOLD_UP             = 0x00000020;
const omGsensor HOLD_DOWN           = 0x00000040;
const omGsensor FALL_ON_BEHIND      = 0x00000080;
const omGsensor UNMATCHED           = 0x00000100;
const omGsensor WALKING             = 0x00000200;
const omGsensor STOP_ON_GROUND      = 0x00000400;
const omGsensor STOP_ON_AIR         = 0x00000800;
const omGsensor SHAKE_ON_GROUND     = 0x00001000;
const omGsensor SHAKE_ON_AIR        = 0x00002000;
const omGsensor GET_UP              = 0x00004000;
const omGsensor JOINT_DANGER        = 0x00010000;
const omGsensor JOINT_GAIN_ENABLED  = 0x00020000;
const omGsensor JOINT_GAIN_DISABLED = 0x00040000;
const omGsensor OMG_TIME_UP         = 0x00080000;

class OMGsensorMessage {
friend class OMGsensor;
    unsigned int time_stamp;
    omGsensor    posture_state;
    float        roll_angle;   
    float        pitch_angle;  
    float        yaw_angle;    
public:
    OMGsensorMessage(void):time_stamp(0), posture_state(OMG_UNKNOWN),
    roll_angle(0.0), pitch_angle(0.0), yaw_angle(0.0){}
    ~OMGsensorMessage(){}

    int  Time_Stamp(void)   const { return(time_stamp); }

    bool IsPosture(omGsensor posture) const 
        { return(posture_state & posture); }
    
    float  Roll(void)  const { return(roll_angle); }
    float  Pitch(void) const { return(pitch_angle); }
    float  Yaw(void)   const { return(yaw_angle); }
};

#endif // _OMGsensorData_h_DEFINED



