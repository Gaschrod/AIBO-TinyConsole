/*
 * The Aperios Operating System
 *
 * This program is subject to copyright protection in accordance with the
 * applicable law. It must not, except where allowed by law, by any means or
 * in any form be reproduced, distributed or lent. Moreover, no part of the
 * program may be used, viewed, printed, disassembled or otherwise interfered
 * with in any form, except where allowed by law, without the express written
 * consent of the copyright holder.
 *
 * Copyright (C) 1988-1998 Sony Corporation
 * All Rights Reserved.
 *
 */
/*
 * $Date: 2002/04/11 09:19:31 $
 * $Revision: 1.5 $
 */

#ifndef _OID_h_DEFINED
#define _OID_h_DEFINED

#include <Types.h>
//#include <WellknownOID.h>   /* $(GLOBALDIR)/include/$(CPU_MODEL)/$(PLATFORM) */

#define OID_UNDEF -1
#define OID_ALL     0xCCCCCCCC


#ifdef  __cplusplus
struct Object;
#endif  /* __cplusplus */

#ifndef  __cplusplus
typedef
#endif /* !__cplusplus */
struct structOID {
    //    void *address;
    int msqid;
}
#ifndef  __cplusplus
structOID
#endif /* !__cplusplus */
;

#ifdef  __cplusplus
class OID : public structOID {
  public:
    inline OID ();
    inline OID (int value);

    operator int()const { return msqid; }
    const int operator = (int qid) { return (msqid = qid); }

    inline void  SetAddress (int value);
    inline int   GetAddress () const;
    inline bool  IsValid () const;
    inline void  MakeInvalid ();

    inline bool  operator == (const OID& oid) const;
    inline bool  operator != (const OID& oid) const;
    inline bool  operator <  (const OID& oid) const;
    inline bool  operator <= (const OID& oid) const;
    inline bool  operator >  (const OID& oid) const;
    inline bool  operator >= (const OID& oid) const;

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
    msqid = OID_UNDEF;
}

inline
OID::OID (int value)
{
    msqid = value;
}

inline void
OID::SetAddress (int value)
{
    msqid = value;
}

inline int
OID::GetAddress () const
{
    return msqid;
}

inline bool
OID::IsValid () const
{
    return msqid != OID_UNDEF;
}

inline void
OID::MakeInvalid ()
{
    msqid = OID_UNDEF;
}

inline bool
OID::operator == (const OID& oid) const
{
    return msqid == oid.msqid;
}

inline bool
OID::operator != (const OID& oid) const
{
    return msqid != oid.msqid;
}

inline bool
OID::operator < (const OID& oid) const
{
    return msqid < oid.msqid;
}

inline bool
OID::operator <= (const OID& oid) const
{
    return msqid <= oid.msqid;
}

inline bool
OID::operator > (const OID& oid) const
{
    return msqid > oid.msqid;
}

inline bool
OID::operator >= (const OID& oid) const
{
    return msqid >= oid.msqid;
}

/*
   inline OID
   OID::RegistryManager ()
   {
       return OID ((void*) OID_REGISTRYMGR);
   }

   inline Object*
   OID::GetObject ()
   {
       return (Object*)msqid;
   }
   */
#endif  /* __cplusplus */


struct RID {
    int       msqid;
    longword  type;

    RID(void) : msqid(OID_UNDEF), type(0x7fffffff) {}
    RID(int id, longword t) : msqid(id), type(t) {}
    RID(const RID& rid) : msqid(rid.msqid), type(rid.type) {}
    RID& operator=(const RID& rid) {
        msqid = rid.msqid;
        type  = rid.type;
        return *this;
    }
    bool operator!(void) const { return (msqid == OID_UNDEF); }
    void Invalidate(void) {
        msqid = OID_UNDEF;
        type  = 0x7fffffff;
        return;
    }
};


#endif  /* _OID_h_DEFINED */
