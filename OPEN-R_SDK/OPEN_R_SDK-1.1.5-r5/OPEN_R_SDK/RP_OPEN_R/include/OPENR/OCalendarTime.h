//
// OCalendarTime.h
//
// Copyright 2000,2002 Sony Corporation
//

#ifndef _OCalendarTime_h_DEFINED
#define _OCalendarTime_h_DEFINED

#include <OPENR/OStatus.h>
#include <OPENR/OTime.h>

const int ocldtdowSUNDAY    = 0;
const int ocldtdowMONDAY    = 1;
const int ocldtdowTUESDAY   = 2;
const int ocldtdowWEDNESDAY = 3;
const int ocldtdowTHURSDAY  = 4;
const int ocldtdowFRIDAY    = 5;
const int ocldtdowSATURDAY  = 6;

const int ocldtSTART_DAY_OF_THE_WEEK = ocldtdowSATURDAY;// Saturday(2000/01/01)

class OCalendarTime 
{
public:
    OCalendarTime();
    OCalendarTime(time_t clock, sbyte timeDif);
    OCalendarTime(const OTime& time);
    OCalendarTime(const OCalendarTime& calendarTime);

    OCalendarTime& operator=(const OCalendarTime& ocalendarTime);

    bool operator<(const OCalendarTime& rhs) const;
    bool operator<=(const OCalendarTime& rhs) const;
    bool operator>(const OCalendarTime& rhs) const;
    bool operator>=(const OCalendarTime& rhs) const;
    bool operator==(const OCalendarTime& rhs) const;

    OStatus SetTime(time_t clock, sbyte timeDif);
    OStatus SetTime(const OTime& time);

    OStatus GetTime(time_t* clock, sbyte* timeDif) const;
    OStatus GetTime(OTime* time) const;

    bool ProperDate() const;
    OStatus CorrectDayOfTheWeek();

    void Print() const;
    void Sprint(char* str) const;
    void RFC2822(char* str) const;

private:

public:
    int   year_;	/* year since start year defined in OTime.h */
    int   month_;	/* [1-12]                */
    int   day_;		/* [1-31]                */
    int   hour_;	/* [0-23]                */
    int   min_;		/* [0-59]                */
    int   sec_;		/* [0-59]                */

    int   dow_;         /* Day Of the week. Sunday(0), Monday(1),... */

    sbyte timeDif_;     /* Time difference. From -12 to 12. */
};

inline bool
LeapYear(int year)
{
    return (!(year % 4) && !(!(year % 100) && (year % 400)) ?
	    true : false);
}


#endif  /* _OCalendarTime_h_DEFINED */
