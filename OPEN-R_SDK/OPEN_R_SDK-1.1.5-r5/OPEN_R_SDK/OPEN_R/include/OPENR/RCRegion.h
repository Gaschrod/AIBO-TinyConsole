//
// RCRegion.h
//
// Copyright 2001,2002 Sony Corporation
//

#if defined(__GNUC__)
#include <iostream>
using std::cout;
using std::endl;
#else
#include <iostream.h>
#endif
#include <OPENR/SharedMemoryHeader.h>
#include <OPENR/SharedMemoryAllocator.h>
#include <OPENR/StackAllocator.h>

typedef SharedMemoryAllocator SHMAllocator ;
typedef unsigned int offset_t;

#ifndef RCRegion_h_DEFINED
#define RCRegion_h_DEFINED

//  Definition for identifying ObjectComm version
#define OBJCOMM_VERSION_21
#define OBJCOMM_VERSION_22

/*
 *  RCRegion class counts references and controls allocation of
 *  shared memory regions. Removing last reference of a region will
 *  automatically destroy the instance of this class and return
 *  shared memory to memory pool.
 */
class RCRegion {

public:
    enum RegionType {
		SELF_ALLOCATED,
		REFERENCE_V20,
		REFERENCE_V10,
		UNDEF
    };

    // Ref=0
    RCRegion();

    // Ref=1:Self Allocation
    explicit RCRegion(size_t size);

    // Ref=1:External NewSharedMemoryRegion()
    // On subject side, baseAddr is not necessary since user has attached
    // shared memory by himself.
    // On observer side, attach is done in NotifyHandler() so that baseAddr
    // can be set at that time.
    RCRegion(MemoryRegionID memID, offset_t offset, void* baseAddr=NULL, size_t size=0);
    //RCRegion(MemoryRegionID memID, offset_t offset, size_t size, void* base);

    void AddReference();
    void RemoveReference();
    int  NumberOfReference() const { return refCounter_; }

    char*           Base() const;
    size_t          Size() const;
    MemoryRegionID  MemID() const;
    offset_t        Offset() const;
    RegionType      GetType() const;

    void SetSize(size_t size);

    /*
     *  Instances of RCRegion class are frequently new and deleted.
     *  Self Allocator is provided and new and delete operator is overriden
     *  to use reserved stack and not to use system heap area.
     */
    static void* operator new(size_t size);
    static void  operator delete(void* ptr, size_t size);

  private:

    bool    IsOwner() const;

    // Allocator for shared memory 
    static SHMAllocator shm_allocator;

    // Prohibit to create instance as auto variables.(from system stack)
    // It is not intended to be a base class
    ~RCRegion();
    // Copy of region is prohibited.
    RCRegion(const RCRegion&);
    RCRegion& operator=(const RCRegion&);

    // These API are no longer available from v2.2.
	// Ref=1:External Allocator 2.0
    explicit RCRegion(ShmHdr* r);
    explicit RCRegion(void* baseAddr);
    ShmHdr* Region()  const;
    ShmHdr* Header()  const;

    int             refCounter_;
    RegionType      status_;
    MemoryRegionID  memID_;
    offset_t        offset_;
    size_t          size_;
    char*           baseAddr_;

    friend class StackAllocator<RCRegion, 20>;
};

/*  Legacy support */
typedef RCRegion RCRegionBase;

#endif

