/*
 *  Copyright 2002 Sony Corporation
 */
#ifndef _antSharedBufferDef_H_
#define _antSharedBufferDef_H_

#include <antTypes.h>

class antSharedBuffer
{
  friend class antSharedBufferManager;

public:

#ifdef ANT_INTERNAL
  // address translation functions
  static antError ExternalToInternal(antObjectID& _client, byte* _external, byte*& _internal, uint32 _size);
  static antError InternalToExternal(byte* _internal, byte*& _external, uint32 _size);

#else // ANT_INTERNAL
  
  antError Map();               // map buffer in applications address space
  antError UnMap();            // remove buffer from application address space

  void* GetAddress();           // get base address of shared buffer
  uint32 GetSize();             // get size of shared buffer

#endif // ANT_INTERNAL

private:
  MemoryRegionID region;        // shared memory region ID
  void* base;                   // address in applications address space
  uint32 size;                  // size of shared buffer
  void* cookie;                 // needed for mAV
  
};

#ifndef ANT_INTERNAL

inline
void*
antSharedBuffer::GetAddress()
{
  return base;
}

inline
uint32
antSharedBuffer::GetSize()
{
  return size;
}

#endif // ANT_INTERNAL

#endif

