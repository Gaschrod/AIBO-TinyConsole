//
// OPENREvent.h
//
// Copyright 1997,1998,1999,2000,2001,2002 Sony Corporation
//

#ifndef _OPENREvent_h_DEFINED
#define _OPENREvent_h_DEFINED

#include <OPENR/OPENR.h>
#include <OPENR/ObjcommEvent.h>

class OSystemEvent {
private:
    OReason  reason_;
    void*    param_;
    size_t   paramSize_;
    
public:
    OSystemEvent(OReason reason, void* param, size_t paramSize); 
    ~OSystemEvent();
    
    OReason      Reason()    const;
    const void*  Param()     const;
    size_t       ParamSize() const;
};

#endif /* _OPENREvent_h */
