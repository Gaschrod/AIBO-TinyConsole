//
//  This file selects Shared Memory Allocator for
//     Inter-Object Communication
//
//  Copyright (C) 2000 Sony Corporation
//  All Rights Reserved.
//

#ifndef SharedMemoryAllocator_h_DEFINED
#define SharedMemoryAllocator_h_DEFINED

#if 1
#include <OPENR/ShmUnitAllocator.h>
typedef ShmUnitAllocator SharedMemoryAllocator;
#else
#include <OPENR/ShmNormalAllocator.h>
typedef ShmNormalAllocator SharedMemoryAllocator;
#endif

#endif // SharedMemoryAllocator_h_DEFINED




