/*
 *  Copyright 2002 Sony Corporation
 */
#ifndef _antModuleRef_H_
#define _antModuleRef_H_

#include <antTypes.h>

#ifdef ANT_INTERNAL
#include <antModule.h>
#else
class antModuleBase;
#endif

class antModuleRef
{
#ifdef ANT_INTERNAL
  friend class antModuleManager;
#endif

public:
  antModuleRef();
  ~antModuleRef();

  int operator==(const antModuleRef& _ref) const;

#ifdef ANT_INTERNAL
  antModuleRef(antModuleBase* _module);
#else
  antModuleRef(void* _ptr, uint32 _index);
#endif
  OSTREAM& Print(OSTREAM& _os) const;
private:
  antModuleBase* module;
  uint32 index;
};

//
// inline definitions
//

inline
antModuleRef::antModuleRef() : module(0), index(0)
{
}

inline
antModuleRef::~antModuleRef()
{
}

inline
int
antModuleRef::operator==(const antModuleRef& _ref) const
{
  return ((module == _ref.module) && (index == _ref.index));
}

#ifdef ANT_INTERNAL

inline
antModuleRef::antModuleRef(antModuleBase* _module)
{
  module = _module;
  index = _module ? _module->moduleIndex :  0;
}

#else

inline
antModuleRef::antModuleRef(void* _ptr, uint32 _index)
{
  module = (antModuleBase*)_ptr;
  index = _index;
}

#endif // ANT_INTERNAL

// for printing
OSTREAM& operator<<(OSTREAM& _os, const antModuleRef& _ref);

#endif // _antModuleRef_H_

