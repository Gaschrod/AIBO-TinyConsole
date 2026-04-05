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
 * $Date: Mon Feb 25 16:33:07 JST 2002 $
 * $Revision: /main/tokyo/AIBO9/1 $
 */

#ifndef _TID_h_DEFINED
#define _TID_h_DEFINED

#include <OID.h>
#include <Types.h>

#define TID_UNDEF   0

#ifndef  __cplusplus
typedef
#endif
struct BaseTID {
	structOID	mailer;
	longword	value;
}
#ifndef  __cplusplus
BaseTID
#endif
;

#ifdef  __cplusplus
class TID {
protected:
    OID		mailer;
    longword	value;

    TID (OID mlr, longword vl) : mailer (mlr), value (vl) {}
    TID (OID mlr);

    void        SetMailer (OID mailer);
    void        SetValue (longword value);

public:
    TID ();

    bool        IsValid () const;
    void        MakeInvalid ();
    OID         GetMailer ();
    longword    GetValue ();

    bool operator == (const TID& tid) const;
    bool operator != (const TID& tid) const;
};
#else	/* __cplusplus */
typedef	BaseTID	TID;
#endif  /* __cplusplus */

#ifdef  __cplusplus
inline
TID::TID ()
{
    mailer.MakeInvalid ();
    value = TID_UNDEF;
}

inline
TID::TID (OID mlr)
{
    mailer = mlr;
    value = TID_UNDEF;
}

inline bool
TID::IsValid () const
{
    return value != TID_UNDEF;
}

inline void
TID::MakeInvalid ()
{
    mailer.MakeInvalid ();
    value = TID_UNDEF;
}

inline void
TID::SetMailer (OID mlr)
{
    mailer = mlr;
}

inline OID
TID::GetMailer ()
{
    return mailer;
}

inline void
TID::SetValue (longword vl)
{
    value = vl;
}

inline longword
TID::GetValue ()
{
    return value;
}

inline bool
TID::operator == (const TID& tid) const
{
    return (value == tid.value) && (mailer == tid.mailer);
}

inline bool
TID::operator != (const TID& tid) const
{
    return (value != tid.value) || (mailer != tid.mailer);
}
#endif  /* __cplusplus */

#endif  /* _TID_h_DEFINED */
