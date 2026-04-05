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
 * $Date: Mon Feb 25 16:33:27 JST 2002 $
 * $Revision: /main/tokyo/AIBO9/2 $
 */

#ifndef _ObletDependencyDescrList_h_
#define _ObletDependencyDescrList_h_

#include <Types.h>
#include "ObletDependencyDescr.h"
#include "ObletManagerTypes.h"

struct ObletDependencyDescrList {
  /*
    a ObletDependencyDescrList is a list of offsets/addresses of methods
    */
#ifndef USE_INITIALIZER_LIST
public:
  /*
    the ctors will make a deep copy of their arguments.
    in case of failure, the constructed objet is ! IsValid()
    */
  ObletDependencyDescrList(size_t sze = 0,
                           const ObletDependencyDescr *tab = 0);
  ObletDependencyDescrList(const ObletDependencyDescrList &);
  ~ObletDependencyDescrList();

  ObletDependencyDescrList &operator=(const ObletDependencyDescrList&);
  
  inline bool IsValid() const;
  
  inline size_t GetSize() const;
  inline const ObletDependencyDescr* GetTable() const;
  inline ObletDependencyDescr* GetTable();

protected:
  void cloneTable(size_t sze, const ObletDependencyDescr *templ);
  inline void invalidate();
  static ObletDependencyDescr* allocateArrayOfObletDependencyDescr(size_t);
  sError deleteArrayOfObletDependencyDescr(void);
private:
  
#ifdef DEBUG
  friend inline
  ostream& operator << (ostream& s, const ObletDependencyDescrList& list);
  
#endif /* DEBUG */

#endif /* USE_INITIALIZER_LIST */

  size_t size;
  ObletDependencyDescr *table;
};

#ifndef USE_INITIALIZER_LIST
//
// Inline code
//

inline bool
ObletDependencyDescrList::IsValid() const{
  return ObletUndef != size;
}

inline void
ObletDependencyDescrList::invalidate() {
  size = ObletUndef;
}
  
inline size_t
ObletDependencyDescrList::GetSize() const {
  return size;
}

inline const ObletDependencyDescr*
ObletDependencyDescrList::GetTable() const {
  return table;
}

inline ObletDependencyDescr*
ObletDependencyDescrList::GetTable(){
  return table;
}

//
// Debug aids
//

#ifdef DEBUG

inline ostream& operator << (ostream& s, const ObletDependencyDescrList& list) {
  s << " dependent on " << list.size << " oblets  : " << endl;
  const ObletDependencyDescr *next = list.table;
  for (int i = 0 ; i < list.size; i++, next++)
    s << *next << endl;
  return s;
}
#endif

#endif /* USE_INITIALIZER_LIST */

#if 1
// obsolete soon
struct ObletDependencyDescrListRep {
  /*
    should have the same memory layout as ObletDependencyDescrList.
    use for snapheader.
 */
  size_t size;
  ObletDependencyDescrRep *table;

};
#endif

#endif /* _ObletDependencyDescrList_h_ */
