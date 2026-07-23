/*
 *  Copyright 2002 Sony Corporation
 */
#ifndef _IPEndpointMsg_H_
#define _IPEndpointMsg_H_

#include <antEnvMsg.h>
#include <Internet.h>

//
// error value
//

enum IPEndpointError
{
  IP_SUCCESS = 0,
  IP_FAIL = 1,

  IP_INVALID_PROTOCOL = 2,
  IP_PACKETSIZE = 3,
  IP_CONNECTION_CLOSED = 4,
  IP_CONNECTION_BUSY = 5,

  IP_BUFFER_INVALID = 6,

  IP_OPERATION_INVALID = 7,
  IP_OPERATION_UNKNOWN = 8
};

//
// Packet Types
//

enum IPPacketType
{
  IP_DATA = 1,
  IP_NETWORK_UNREACHABLE = 2,
  IP_HOST_UNREACHABLE = 3,
  IP_PROTOCOL_UNREACHABLE = 4,
  IP_PORT_UNREACHABLE = 5,
  IP_MESSAGE_TOO_LONG = 6,
  IP_TTL_EXCEEDED = 7,
  IP_TIME_EXCEEDED = 8,

  IP_PACKETTYPE_MAX = 9
};

//
// external messages
//
enum IPEndpointOperation
{
  IPEndpoint_Operation_Bind = 1,
  IPEndpoint_Operation_Send = 2,
  IPEndpoint_Operation_Receive = 3,
  IPEndpoint_Operation_Close = 4
};

//
// base
//
struct IPEndpointBaseMsg: public antEnvMsg
{
public:
  IPEndpointError error;

public:
  IPEndpointBaseMsg() : error(IP_FAIL), antEnvMsg() {};

  IPEndpointBaseMsg(IPEndpointBaseMsg& _msg)
    : antEnvMsg(_msg), error(_msg.error){}

  IPEndpointBaseMsg(antModuleRef& _module, IPEndpointOperation _operation)
    : error(IP_FAIL), antEnvMsg(_module, _operation) {}
  
  antError Reply(IPEndpointError _error)
  {
    error = _error;
    return antEnvMsg::Reply();
  }
};


//
// bind
//

struct IPEndpointBindMsg: public IPEndpointBaseMsg
{
public:
  Protocol protocol; // IP protocol number
  
public:
  IPEndpointBindMsg():IPEndpointBaseMsg(){}

  IPEndpointBindMsg(IPEndpointBindMsg& _msg)
    :IPEndpointBaseMsg(_msg), protocol(_msg.protocol){}

  IPEndpointBindMsg(antModuleRef& _module, Protocol _protocol)
    : IPEndpointBaseMsg(_module, IPEndpoint_Operation_Bind), protocol(_protocol){}

};

//
// send
//

struct IPEndpointSendMsg: public IPEndpointBaseMsg
{
public:
  IPPacketType type; // type of packet
  byte* buffer;      // buffer holding IP packet
  int size;          // size of buffer

public:
  IPEndpointSendMsg():IPEndpointBaseMsg(){}

  IPEndpointSendMsg(IPEndpointSendMsg& _msg)
    :IPEndpointBaseMsg(_msg), type(_msg.type), buffer(_msg.buffer), size(_msg.size){}

  IPEndpointSendMsg(antModuleRef& _module, IPPacketType _type, byte* _buffer, int _size)
    : IPEndpointBaseMsg(_module, IPEndpoint_Operation_Send),
      type(_type), buffer(_buffer), size(_size){}
};

//
// receive
//
struct IPEndpointReceiveMsg: public IPEndpointBaseMsg
{
public:
  IPPacketType type; // type of packet
  byte* buffer;      // buffer we want to receive the IP packet in
  int size;          // number of bytes to receive

public:
  IPEndpointReceiveMsg():IPEndpointBaseMsg(){}

  IPEndpointReceiveMsg(IPEndpointReceiveMsg& _msg)
    :IPEndpointBaseMsg(_msg), type(_msg.type), buffer(_msg.buffer), size(_msg.size){}

  IPEndpointReceiveMsg(antModuleRef& _module, byte* _buffer, int _size)
  : IPEndpointBaseMsg(_module, IPEndpoint_Operation_Receive),
    buffer(_buffer), size(_size){}
};

//
// close
//

struct IPEndpointCloseMsg: public IPEndpointBaseMsg
{
public:
  IPEndpointCloseMsg():IPEndpointBaseMsg(){}

  IPEndpointCloseMsg(IPEndpointCloseMsg& _msg)
    :IPEndpointBaseMsg(_msg){}

  IPEndpointCloseMsg(antModuleRef& _module)
    : IPEndpointBaseMsg(_module, IPEndpoint_Operation_Close){}
};

#endif

