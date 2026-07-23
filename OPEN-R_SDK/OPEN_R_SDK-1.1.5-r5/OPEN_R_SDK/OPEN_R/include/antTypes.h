/*
 *  Copyright 2002 Sony Corporation
 */
#ifndef _antTypes_H_
#define _antTypes_H_

#include <OID.h>
#include <antIOStream.h>


// ===================================================
//
// basic types
//
// ===================================================

// 32 bit integers
typedef long int32;
typedef unsigned long uint32;

// 16 bit integers
typedef short int16;
typedef unsigned short uint16;

typedef int boolean;
typedef unsigned char byte;

#ifndef FALSE
#define FALSE 0
#define TRUE 1
#endif

// ===================================================
//
// object ids
//
// ===================================================

typedef OID antObjectID;

// ===================================================
//
// return codes
//
// ===================================================

enum antError
{
  ANT_SUCCESS = 0,       // all is ok


  //
  // general
  // 
  
  ANT_FAIL = 1,                 // something failed
  ANT_NOMEMORY = 2,             // we ran out of memory
  ANT_INVALIDPTR = 3,           // we tried to dereference am invalid antPtr
  ANT_NO_STATE_TRANSITION = 4,  // the applied method had no state transition effect

  // for SDUs
  ANT_SDU_BADRANGE = 5,         // Bad range
  ANT_SDU_NOMORE_DATACELL = 6,  // No more data cell is in the SDU (for iterator)
  ANT_SDU_NOOWNER = 7,          // No owner of the SDU
    
  // for SDUPools
  ANT_SDUPOOL_ALLOCATED = 8,    // Still SDUs are allocated in this pool
  ANT_SDUPOOL_NOTASSIGNED = 9,  // SDUPool is not assigned to the current module

  // for Queues
  ANT_QUEUE_EMPTY = 10,         // the queue is empty
  ANT_QUEUE_STATE = 11,         // the queue is not in the proper state to perform the action
  ANT_QUEUE_CLOSED = 12,        // the queue is closed
  ANT_QUEUE_REJECTED = 13,      // the queue is rejected by the other side
  ANT_QUEUE_BLOCKED = 14,       // the queue is blocked

  // for modules
  ANT_INVALIDMODULEID = 15,     // the moduleID supplied is invalid

  // for Timers
  ANT_TMR_TYPE_INVALID = 16,    // Invalid antTimer Type definition
  ANT_TMR_MAXNMBR_REACHED = 17, // Maximum number of timers has been reached

  // for message pools
  ANT_NOMESSAGES = 18,          // no more messages left in message pool

  // for ant Names
  ANT_NAMING_ENTRY_EXISTS = 19, // Name and/or ModuelID already in registry
  ANT_NAMING_NO_SUCH_NAME = 20, // Name not found in registry
  ANT_NAMING_NAME_INVALID = 21, // Name has invalid syntax

  // for registry
  ANT_REGISTRY_DUPLICATE_KEY = 22,
  ANT_REGISTRY_NO_KEY = 23,
  ANT_REGISTRY_WRONG_TYPE = 24,
  ANT_REGISTRY_INVALID_KEY = 25,
  ANT_REGISTRY_INVALID_VALUE = 26,

  // for endpoints
  ANT_ENDPOINT_NOFACTORY = 27,

  // for ant applications initialization
  ANT_NOT_INITIALIZED = 28,
  ANT_INITIALIZING = 29
};

// notification flags

enum antNotification
{
  ANT_SDUMEMORY_AVAILABLE = (1<< 0)
};

// ===================================================
//
// print out error value
//
// ===================================================

const char* antErrorString(antError error);
OSTREAM& operator<<(OSTREAM& os, antError error);

#ifdef ANT_INTERNAL

extern "C" void antMain();

// the queue open call back function
typedef void (*antQueueCallback) (antError, void*);

#endif // ANT_INTERNAL

#endif
