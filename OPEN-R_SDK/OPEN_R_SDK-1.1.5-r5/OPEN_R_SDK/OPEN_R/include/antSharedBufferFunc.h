/*
 *  Copyright 2002 Sony Corporation
 */
#ifndef _antSharedBufferFunc_H_
#define _antSharedBufferFunc_H_

#include <antTypes.h>

#ifdef ANT_INTERNAL

#include <antSharedBufferManager.h>

ANT_INLINE_COMMAND
antError
antSharedBuffer::ExternalToInternal(antObjectID& _client, byte* _external, byte*& _internal, uint32 _size)
{
  return antSharedBufferManager::ExternalToInternal(_client, _external, _internal, _size);
}

ANT_INLINE_COMMAND
antError
antSharedBuffer::InternalToExternal(byte* _internal, byte*& _external, uint32 _size)
{
  return antSharedBufferManager::InternalToExternal(_internal, _external, _size);
}

#endif

#endif

