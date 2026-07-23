//
// Copyright (C) 2001 Sony Corporation
// All Rights Reserved.
//

#ifndef OShmPtrBase_h_DEFINED
#define OShmPtrBase_h_DEFINED

#include "RCRegion.h"

class OShmPtrBase {

  public:
    OShmPtrBase();                         //  Ref=0
    OShmPtrBase(const OShmPtrBase& rhs);   //  Ref++
    explicit OShmPtrBase(RCRegion* p);          //  Ref++
    virtual ~OShmPtrBase();

    // void Allocate(size_t size);
    // void AllocateCopy(size_t size, void* src);
    void Deallocate();

    const OShmPtrBase& operator=(const OShmPtrBase& rhs);
    char* BaseAddress() const {
        return (pointee_ ? pointee_->Base() : static_cast<char*>(NULL));
    }
    size_t Size() const { return (pointee_ ? pointee_->Size() : 0); }

  protected:
    void init();
    RCRegion*  pointee_;

  public:
    char*   Base()   const { return BaseAddress(); }

    RCRegion*      RCRPtr() const { return pointee_; }
    MemoryRegionID MemID() const { return pointee_->MemID(); }
    unsigned int   Offset() const { return pointee_->Offset(); }

#if 0
	// These API is no longer available in v2.2.
	ShmHdr* Header() const { return pointee_->Header(); }
    ShmHdr* Region() const { return pointee_->Region(); }
#endif

};

//  Non-member Function
void memcpy(void* s1, const OShmPtrBase& s2, size_t);

#endif
