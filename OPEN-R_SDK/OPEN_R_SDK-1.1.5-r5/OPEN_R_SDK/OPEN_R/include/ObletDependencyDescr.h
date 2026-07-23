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
 * $Date: Mon Feb 25 16:33:58 JST 2002 $
 * $Revision: /main/tokyo/AIBO9/1 $
 * $Date: Mon Feb 25 16:33:58 JST 2002 $
 * $Revision: /main/tokyo/AIBO9/1 $
 */

#ifndef _ObletDependencyDescr_h_
#define _ObletDependencyDescr_h_

#include <Types.h>
#include "ObletSpec.h"

struct  ObletDependencyDescr {
/*
  an Object Class or Oblet Class needs to describe which oblets
  it is dependent upon an ObletDependency describes one such dependency.
 */
public:
  size_t gotOffset;  //  offset GlobalOffsetTable
  ObletSpec spec; // link with oblet matching this specification
#ifndef USE_INITIALIZER_LIST
  ObletDependencyDescr(){}
  ObletDependencyDescr(size_t g, char *nm, word int_ma,
                       word int_mi, word imp_ma, word imp_mi)
    : gotOffset(g), spec(nm, int_ma, int_mi, imp_ma, imp_mi){}
#endif  
#ifdef OBLETMGR_USES_NEWREGION
  void* operator new (size_t size);
  void  operator delete (void* base);
#endif /* OBLETMGR_USES_NEWREGION */
}; 

#ifdef DEBUG
#include <stream.h>

inline ostream& operator << (ostream& s, const ObletDependencyDescr& descr) {
  s << "link oblet : "
    << descr.spec
    << " at offset : "
    << descr.gotOffset;
  return s;
}
#endif

#if 1
// obsolete soon
struct  ObletDependencyDescrRep {
/*
  should have the same memory layout as ObletDependencyDescr.
  use in snapheader.
 */
  size_t gotOffset;  //  offset GlobalOffsetTable
  ObletSpecRep spec; // the got expects to be linked with an oblet matching this specification
};

#endif

#endif // _OBLETMANAGERTYPES_H_
