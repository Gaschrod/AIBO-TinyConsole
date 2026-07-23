/*
 * The Aperios Software Platform
 *
 * This program is subject to copyright protection in accordance with the
 * applicable law. It must not, except where allowed by law, by any means or
 * in any form be reproduced, distributed or lent. Moreover, no part of the
 * program may be used, viewed, printed, disassembled or otherwise interfered
 * with in any form, except where allowed by law, without the express written
 * consent of the copyright holder.
 *
 * Copyright 1988-2002 Sony Corporation
 * All Rights Reserved.
 *
 */
/*
 * $Date: Mon Feb 25 16:34:01 JST 2002 $
 * $Revision: /main/tokyo/AIBO9/1 $
 */

#ifndef _EventID_h_DEFINED
#define _EventID_h_DEFINED

#include <Types.h>


#define EventID_UNDEF	0x0

#ifdef  __cplusplus

class EventID {
private:
    longword    value;

public:
    EventID () : value (EventID_UNDEF) {}
    EventID (longword vl) : value (vl) {}

    bool        IsValid () const;
    void        MakeInvalid ();

    bool operator == (const EventID& eID) const;
    bool operator != (const EventID& eID) const;

    void        SetValue (longword vl);
    longword    GetValue ();
};

#else  /* __cplusplus */

typedef struct {
    longword    value;
} EventID;

#endif  /* __cplusplus */

#ifdef  __cplusplus
inline bool
EventID::IsValid () const
{
    return value != EventID_UNDEF;
}

inline void
EventID::MakeInvalid ()
{
    value = EventID_UNDEF;
}

inline bool
EventID::operator == (const EventID& eID) const
{
    return value == eID.value;
}

inline bool
EventID::operator != (const EventID& eID) const
{
    return value != eID.value;
}

inline void
EventID::SetValue (longword vl)
{
    value = vl;
}

inline longword
EventID::GetValue ()
{
    return value;
}
#endif  /* __cplusplus */

#endif  /* _EventID_h_DEFINED */
