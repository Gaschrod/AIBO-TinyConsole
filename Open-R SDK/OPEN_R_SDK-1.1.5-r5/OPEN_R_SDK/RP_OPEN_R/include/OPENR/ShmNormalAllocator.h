//
//  MemoryAllocation Manager for Shared Memory Region
//     (ProtoType)
//

#ifndef ShmNormalAllocator_h_DEFINED
#define ShmNormalAllocator_h_DEFINED

#include <list>
using std::list;
#include <OPENR/SharedMemoryHeader.h>

class ShmNormalAllocator {

public:
    ShmNormalAllocator();
    ~ShmNormalAllocator();

    ShmHdr* New( size_t size );
    void  Delete( ShmHdr* region );

private:
    list<ShmHdr*>::iterator FindUsedRegion( size_t size );
    ShmHdr* AddBuffer(size_t size);
    size_t buffer_size() const { return 4096; }

    static const char* const libname;
    list<ShmHdr*>  regions_;
    list<ShmHdr*>  buffers_;
};


#endif

