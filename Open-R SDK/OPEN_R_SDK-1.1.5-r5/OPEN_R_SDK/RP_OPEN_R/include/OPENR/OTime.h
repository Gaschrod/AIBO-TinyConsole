//
// OTime.h
//
// Copyright 2000,2001,2002 Sony Corporation
//

#ifndef _OTime_h_DEFINED
#define _OTime_h_DEFINED

#include <time.h>
#include <Types.h>
#include <OPENR/OStatus.h>

const sbyte otimeTIME_DIF_UNDEF = 0x7F;

const int   otimeSTART_YEAR     = 2000;

class OCalendarTime;

class OTime
{
friend class OCalendarTime;
public:
    OTime();
    OTime(time_t clock, sbyte timeDif);
    OTime(const OTime& time);

    OTime& operator=(const OTime& time);

    bool operator<(const OTime& rhs) const;
    bool operator<=(const OTime& rhs) const;
    bool operator>(const OTime& rhs) const;
    bool operator>=(const OTime& rhs) const;
    bool operator==(const OTime& rhs) const;

    OStatus Set(time_t clock, sbyte timeDif);
    OStatus SetClock(time_t clock);
    OStatus SetLocalTime(time_t localTime);
    OStatus SetTimeDif(sbyte timeDif);

    time_t GetClock() const;
    time_t GetLocalTime() const;
    sbyte  GetTimeDif() const;

private:
    time_t clock_;
    sbyte  timeDif_;
};

#endif  /* _OTime_h_DEFINED */
