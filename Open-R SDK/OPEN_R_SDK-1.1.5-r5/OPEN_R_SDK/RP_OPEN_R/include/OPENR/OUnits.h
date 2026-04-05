//
// OUnits.h
//
// Copyright 1997,1998,1999,2000,2001,2002 Sony Corporation
//

#ifndef _OUnits_h_DEFINED
#define _OUnits_h_DEFINED

#include <Types.h>

typedef slongword	Length;			 // 10^-6 m
typedef slongword	Mass;			 // 10^-6 kg (10^-3 g)
typedef slongword	Angle;			 // 10^-6 rad
typedef slongword	Temperature;     // 10^-6 degree(Celcius)
typedef slongword	Velocity;		 // 10^-6 m/s
typedef slongword	Acceleration;    // 10^-6 m/s^2
typedef slongword	AngularVelocity; // 10^-6 rad/s
typedef slongword	Force;			 // 10^-6 N
typedef slongword	Percentage;		 // 10^-6 %

//
// Radian (10^-6 rad)
//
const slongword oradiansPI   = 3141592;
const slongword oradiansPI_2 = 1570796;
const slongword oradiansPI_4 =  785398;

inline slongword
micro(double val)
{
    return (slongword)(val * 1000000.0);
}

inline double
radians(double deg)
{
    return 3.14159265358979323846 * deg / 180.0;
}

inline double
degrees(double rad)
{
    return 180.0 * rad / 3.14159265358979323846; 
}

inline slongword
oradians(double deg)
{
    return micro(radians(deg));
}

#endif /* _OUnits_h_DEFINED */
