//
//  Memory Allocation Manager for Shared Memory Region
//
//  Copyright (C) 2000 Sony Corporation
//  All Rights Reserved.
//

//
//  Allocation policy :
//     If requested memory size is larger than the unit size,
//     it will allocate multiple of unit size.
//

#ifndef ShmUnitAllocator_h_DEFINED
#define ShmUnitAllocator_h_DEFINED

#include <list>
using std::list;
#include <OPENR/SharedMemoryHeader.h>
#include <OPENR/MemoryMapAgent.h>

class ShmUnitAllocator {

public:
    ShmUnitAllocator();
    ~ShmUnitAllocator();

    ShmHdr* New( size_t size );
    void    Delete( ShmHdr* region );

  private:
    size_t AlignQuantize(size_t size){
	size_t asize = ((size + 7) & 0xfffffff8);
	return (asize > UNIT_SIZE) ?
	    ((asize-1)/UNIT_SIZE + 1)*UNIT_SIZE : asize;
    }

    list<ShmHdr*>::iterator FindUsedRegion( size_t size );
    ShmHdr* AddBuffer(size_t size);
    size_t  buffer_size() const { return 4096; }

    static const char* const libname;
    static const size_t UNIT_SIZE;

	//  regions_ is a list of shared memory regions which are cut
	//  in pieces for user's request and returned by user after its use.
    list<ShmHdr*>  regions_;
	//  buffers_ is a list of shared memory regions acquired by
	//  system call, NewSharedMemoryRegion().
    list<ShmHdr*>  buffers_;

	MemoryMapAgent  memMapAgent_;
};


#endif

