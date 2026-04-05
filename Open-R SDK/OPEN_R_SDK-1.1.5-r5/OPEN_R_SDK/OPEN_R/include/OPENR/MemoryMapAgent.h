//
// MemoryMapAgent.h
//
// Copyright 2001 Sony Corporation
//

#ifndef _MemoryMapAgent_h_DEFINED
#define _MemoryMapAgent_h_DEFINED

#include <Types.h>
#include <map>
using std::map;
using std::less;
#include <OPENR/OPENR.h>

class MemoryMapAgent {

private:
    static int numOfInstance_;
    static map<MemoryRegionID, void*, less<MemoryRegionID> > baseAddrs_;

public:
    MemoryMapAgent();
    ~MemoryMapAgent();

    OStatus GetBaseAddress(MemoryRegionID, void**);
	OStatus Register(MemoryRegionID memID, void* baseAddr);
    OStatus DetachAll();
#ifdef DEBUG
    void Print();
#endif

};

#endif /* _MemoryMapAgent_h_DEFINED */



