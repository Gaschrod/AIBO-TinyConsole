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
 * $Date: Mon Feb 25 16:33:06 JST 2002 $
 * $Revision: /main/tokyo/AIBO9/1 $
 */

#ifndef _OID_h_DEFINED
#define _OID_h_DEFINED

#include <Types.h>
#include <WellknownOID.h>   /* $(GLOBALDIR)/include/$(CPU_MODEL)/$(PLATFORM) */

#define OID_UNDEF   0xFFFFFFFF

#ifdef  __cplusplus
struct Object;
#endif  /* __cplusplus */

#ifndef  __cplusplus
typedef
#endif /* !__cplusplus */
struct structOID {
    void *address;
}
#ifndef  __cplusplus
structOID
#endif /* !__cplusplus */
;

#ifdef  __cplusplus
class OID : public structOID {
public:
    inline OID ();
    inline OID (void* address);

    inline void     SetAddress (void* value);
    inline void*    GetAddress () const;
    inline bool     IsValid () const;
    inline void     MakeInvalid ();

    inline bool     operator == (const OID& oid) const;
    inline bool     operator != (const OID& oid) const;
    inline bool     operator <  (const OID& oid) const;
    inline bool     operator <= (const OID& oid) const;
    inline bool     operator >  (const OID& oid) const;
    inline bool     operator >= (const OID& oid) const;

    static OID  Namer ();
    static OID  RegistryManager ();

    inline Object*  GetObject ();
};
#else   /* __cplusplus */
typedef structOID   OID;
#endif  /* __cplusplus */

#ifdef  __cplusplus
inline
OID::OID ()
{
    address = (void*) OID_UNDEF;
}

inline
OID::OID (void* value)
{
    address = value;
}

inline void
OID::SetAddress (void* value)
{
    address = value;
}

inline void*
OID::GetAddress () const
{
    return address;
}

inline bool
OID::IsValid () const
{
    return address != (void*) OID_UNDEF;
}

inline void
OID::MakeInvalid ()
{
    address = (void*) OID_UNDEF;
}

inline bool
OID::operator == (const OID& oid) const
{
    return address == oid.address;
}

inline bool
OID::operator != (const OID& oid) const
{
    return address != oid.address;
}

inline bool
OID::operator < (const OID& oid) const
{
    return address < oid.address;
}

inline bool
OID::operator <= (const OID& oid) const
{
    return address <= oid.address;
}

inline bool
OID::operator > (const OID& oid) const
{
    return address > oid.address;
}

inline bool
OID::operator >= (const OID& oid) const
{
    return address >= oid.address;
}

inline OID
OID::RegistryManager ()
{
    return OID ((void*) OID_REGISTRYMGR);
}

inline Object*
OID::GetObject ()
{
    return (Object*)address;
}
#endif  /* __cplusplus */

#endif  /* _OID_h_DEFINED */
