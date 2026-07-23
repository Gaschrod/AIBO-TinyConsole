/*
 *  Copyright 2002 Sony Corporation
 */
#ifndef _antEnvInternalMsg_H_
#define _antEnvInternalMsg_H_

#include <antSharedBuffer.h>
#include <antEnvBaseMsg.h>
#include <string.h>


// ========================================================
//
// external messages to the ant Environment
//
// ========================================================

// the top 16 bits are reserved for the Selector
enum
{
  antEnv_Operation_CreateEndpoint = -1,
  antEnv_Operation_CreateSharedBuffer = -2,
  antEnv_Operation_DestroySharedBuffer = -3
};
enum
{
  antEnv_Operation_InitSetParam = -1,
  antEnv_Operation_InitStart = -2,
  antEnv_Operation_InitGetParam = -3
};


//
// to create an endpoint
//

struct antEnvCreateEndpointMsg: public antEnvMsg
{
public:
  antError error;
  int32 protocol;
  int32 poolSize;
  antModuleRef moduleRef;

public:
  // constructors
  antEnvCreateEndpointMsg(): antEnvMsg() {};
  antEnvCreateEndpointMsg(int32 _protocol, int32 _poolSize = 0);

  // reply
  antError Reply(antError _error);
};

//
// to create a shared buffer
//

struct antEnvCreateSharedBufferMsg: public antEnvMsg
{
public:
  antError error;
  uint32 size;
  antSharedBuffer buffer;

public:
  // constructor
  antEnvCreateSharedBufferMsg(): antEnvMsg() {};
  antEnvCreateSharedBufferMsg(uint32 _size);

  // reply
  antError Reply(antError _error);
};

//
// to delete a shared buffer
//

struct antEnvDestroySharedBufferMsg: public antEnvMsg
{
public:
  antError error;
  antSharedBuffer buffer;

public:
  // constructor
  antEnvDestroySharedBufferMsg(): antEnvMsg() {};
  antEnvDestroySharedBufferMsg(antSharedBuffer& _buffer);

  // reply
  antError Reply(antError _error);
};


//
// to set registry parameters
// 

#define ANTENV_KEY_LENGTH_MAX    32 
#define ANTENV_VALUE_LENGTH_MAX  32

enum {
  antEnv_InitParam_Int32  = -1,
  antEnv_InitParam_String = -2
};

struct antEnvInitSetParamMsg: public antEnvMsg
{
public:
  // in 
  int32 paramType;
  char key[ANTENV_KEY_LENGTH_MAX];
  union {
    char  str[ANTENV_VALUE_LENGTH_MAX];
    int32 i32;
  } value;
  
  // out
  antError error;
  
public:
  antEnvInitSetParamMsg() : antEnvMsg() {}
  antEnvInitSetParamMsg(const char*, int32);
  antEnvInitSetParamMsg(const char*, const char*);

  antError Reply(antError _error);

  antEnvInitSetParamMsg& operator=(antEnvInitSetParamMsg& _msg);
};

struct antEnvInitGetParamMsg: public antEnvMsg
{ 
public:
  // in
  int32 paramType;
  char key[ANTENV_KEY_LENGTH_MAX];

  // out
  union {
    char  str[ANTENV_VALUE_LENGTH_MAX];
    int32 i32;
  } value;
  
  antError error;
  
public:
  antEnvInitGetParamMsg() : antEnvMsg() {}
  antEnvInitGetParamMsg(const char*);

  antError Reply(antError _error);

  antEnvInitGetParamMsg& operator=(antEnvInitGetParamMsg& _msg);
};


//
// to start the initialization
//

struct antEnvInitStartMsg: public antEnvMsg
{
public:
  antError error;
  
public:
  antEnvInitStartMsg();

  antError Reply(antError _error);
};




//
// inline definitions
//

// Create endpoint
// ----------------------------------------------------
inline
antEnvCreateEndpointMsg::antEnvCreateEndpointMsg(int32 _protocol, int32 _poolSize):
  antEnvMsg(antEnv_Operation_CreateEndpoint, Entry_antEnv_External)
{
  protocol = _protocol;
  poolSize = _poolSize;
}

inline
antError
antEnvCreateEndpointMsg::Reply(antError _error)
{
  error = _error;
  return antEnvMsg::Reply();
}

// Create shared buffer
// ----------------------------------------------------
inline
antEnvCreateSharedBufferMsg::antEnvCreateSharedBufferMsg(uint32 _size):
    antEnvMsg(antEnv_Operation_CreateSharedBuffer, Entry_antEnv_External)
{
  size = _size;
}

inline
antError
antEnvCreateSharedBufferMsg::Reply(antError _error)
{
  error = _error;
  return antEnvMsg::Reply();
}

// Destroy shared buffer
// ----------------------------------------------------
inline
antEnvDestroySharedBufferMsg::antEnvDestroySharedBufferMsg(antSharedBuffer& _buffer):
    antEnvMsg(antEnv_Operation_DestroySharedBuffer, Entry_antEnv_External)
{
  buffer = _buffer;
}

inline
antError
antEnvDestroySharedBufferMsg::Reply(antError _error)
{
  error = _error;
  return antEnvMsg::Reply();
}

// Set parameters in registry
// ----------------------------------------------------
inline
antEnvInitSetParamMsg::antEnvInitSetParamMsg(const char* _key, int32 _value) :
  antEnvMsg(antEnv_Operation_InitSetParam, Entry_antEnv_Initialize)
{
  paramType = antEnv_InitParam_Int32;
  
  strncpy(key, _key, ANTENV_KEY_LENGTH_MAX);
  value.i32 = _value;
}

inline
antEnvInitSetParamMsg::antEnvInitSetParamMsg(const char* _key, const char* _value) :
  antEnvMsg(antEnv_Operation_InitSetParam, Entry_antEnv_Initialize)
{
  paramType = antEnv_InitParam_String;
  
  strncpy(key, _key, ANTENV_KEY_LENGTH_MAX);
  strncpy(value.str, _value, ANTENV_VALUE_LENGTH_MAX);
}

inline
antError
antEnvInitSetParamMsg::Reply(antError _error)
{
  error = _error;
  return antEnvMsg::Reply();
}

inline
antEnvInitSetParamMsg&
antEnvInitSetParamMsg::operator=(antEnvInitSetParamMsg& _msg)
{
  paramType = _msg.paramType;
  strncpy(key, _msg.key, ANTENV_KEY_LENGTH_MAX);
  if ( antEnv_InitParam_Int32 == paramType ) value.i32 = _msg.value.i32;
  else strncpy(value.str, _msg.value.str, ANTENV_VALUE_LENGTH_MAX);
  error = _msg.error;

  ((antEnvMsg*)this)->operator=(_msg);
  
  return *this;
}

// Get parameters from registry
// ----------------------------------------------------
inline
antEnvInitGetParamMsg::antEnvInitGetParamMsg(const char* _key) :
  antEnvMsg(antEnv_Operation_InitGetParam, Entry_antEnv_Initialize)
{
  strncpy(key, _key, ANTENV_KEY_LENGTH_MAX);
}

inline
antError
antEnvInitGetParamMsg::Reply(antError _error)
{
  error = _error;
  return antEnvMsg::Reply();
}

inline
antEnvInitGetParamMsg&
antEnvInitGetParamMsg::operator=(antEnvInitGetParamMsg& _msg)
{
  paramType = _msg.paramType;
  strncpy(key, _msg.key, ANTENV_KEY_LENGTH_MAX);
  if ( antEnv_InitParam_Int32 == paramType ) value.i32 = _msg.value.i32;
  else strncpy(value.str, _msg.value.str, ANTENV_VALUE_LENGTH_MAX);
  error = _msg.error;

  ((antEnvMsg*)this)->operator=(_msg);

  return *this;
}

// Start ANT application
// ----------------------------------------------------
inline
antEnvInitStartMsg::antEnvInitStartMsg() :
  antEnvMsg(antEnv_Operation_InitStart, Entry_antEnv_Initialize)
{
  
}

inline
antError
antEnvInitStartMsg::Reply(antError _error)
{
  error = _error;
  return antEnvMsg::Reply();
}

#endif
