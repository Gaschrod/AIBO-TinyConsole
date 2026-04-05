/*
 *  Copyright 2002 Sony Corporation
 */
#ifndef _UDPEndpointMsg_H_
#define _UDPEndpointMsg_H_

#include <antEnvMsg.h>
#include <IPAddress.h>

// ========================================================
//
// error values
//
// ========================================================

enum UDPEndpointError
{
  UDP_SUCCESS = 0,
  UDP_FAIL = 1,

  UDP_CONNECTION_CLOSED = 2,
  UDP_CONNECTION_BUSY = 3,

  UDP_ADDRESSINUSE = 4,
  UDP_ADDRESSERROR = 5,
  
  UDP_PORT_UNREACHABLE = 6,
  UDP_PROTOCOL_UNREACHABLE = 7,
  UDP_HOST_UNREACHABLE = 8,
  UDP_NETWORK_UNREACHABLE = 9,
  UDP_MESSAGE_TOO_LONG = 10,
  UDP_TTL_EXCEEDED = 11,
  UDP_TIME_EXCEEDED = 12,

  UDP_BUFFER_INVALID = 13,

  UDP_OPERATION_INVALID = 14,
  UDP_OPERATION_UNKNOWN = 15,

  UDP_WOULDBLOCK = 16
};

// ========================================================
//
// external messages to the SOCKET module
//
// ========================================================

enum UDPEndpointOperation
{
  UDPEndpoint_Operation_Bind = 0,
  UDPEndpoint_Operation_Connect = 1,
  UDPEndpoint_Operation_Send = 2,
  UDPEndpoint_Operation_Receive = 3,
  UDPEndpoint_Operation_Close = 4
#ifdef USE_MULTICAST
  //
  // multicast extensions to udp endpoints
  //
  ,
  UDPEndpoint_Operation_SetMulticastLoop = 5,
  UDPEndpoint_Operation_SetMulticastTTL = 6,
  UDPEndpoint_Operation_AddMembership = 7,
  UDPEndpoint_Operation_DropMembership = 8
#endif
  ,
  UDPEndpoint_Operation_SetBlockingMode = 9,
  UDPEndpoint_Operation_ReadyForRead = 10,
  UDPEndpoint_Operation_ReadyForWrite = 11,
  UDPEndpoint_Operation_Address = 12,
  UDPEndpoint_Operation_ResetConnect = 13

};

//
// base message
// 

struct UDPEndpointBaseMsg: public antEnvMsg
{
  public:
    UDPEndpointError error;

  public:
    UDPEndpointBaseMsg() : error(UDP_FAIL), antEnvMsg() {};
    UDPEndpointBaseMsg(
                        antModuleRef& _module,
                        UDPEndpointOperation _operation
                      );

    antError Reply(UDPEndpointError _error);
};

//
// bind
//

struct UDPEndpointBindMsg: public UDPEndpointBaseMsg
{
public:
  IPAddress address;
  Port port;
  
public:
  UDPEndpointBindMsg() : UDPEndpointBaseMsg() {};
  UDPEndpointBindMsg(
                      antModuleRef& _module,
                      IPAddress _address, Port _port
                    );
};

//
// connect
//

struct UDPEndpointConnectMsg: public UDPEndpointBaseMsg
{
public:
  IPAddress address;
  Port port;
  
public:
  UDPEndpointConnectMsg() : UDPEndpointBaseMsg() {};
  UDPEndpointConnectMsg(
                        antModuleRef& _module,
                        IPAddress _address, Port _port
                       );
};

//
// send
//

struct UDPEndpointSendMsg: public UDPEndpointBaseMsg
{
public:
  // address info
  IPAddress address;
  Port port;

  // data
  byte* buffer;
  int size;

public:
  UDPEndpointSendMsg() : UDPEndpointBaseMsg() {};
  UDPEndpointSendMsg(
                      antModuleRef& _module,
                      byte* _buffer, int _size
                    );

  UDPEndpointSendMsg(
                      antModuleRef& _module,
                      IPAddress _address, Port _port,
                      byte* _buffer, int _size
                    );
};

//
// receive
//

struct UDPEndpointReceiveMsg: public UDPEndpointBaseMsg
{
public:
  // address info
  IPAddress address;
  Port port;

  // buffer info
  byte* buffer;
  int size;
  
public:
  UDPEndpointReceiveMsg() : UDPEndpointBaseMsg() {};
  UDPEndpointReceiveMsg(
                        antModuleRef& _module,
                        byte* _buffer, int _size
                       );
};

//
// close
//

struct UDPEndpointCloseMsg: public UDPEndpointBaseMsg
{
public:
  bool linger;
  int lingerTime;

public:
  UDPEndpointCloseMsg() : UDPEndpointBaseMsg() {};
  UDPEndpointCloseMsg(antModuleRef& _module,
		      bool _linger = false,
		      int _lingerTime = 0
		     );
};

//
// set blocking mode
//

struct UDPEndpointSetBlockingModeMsg: public UDPEndpointBaseMsg
{
public:
  bool blocking;

public:
  UDPEndpointSetBlockingModeMsg() : UDPEndpointBaseMsg() {};
  UDPEndpointSetBlockingModeMsg(
                                antModuleRef& _module,
                                bool _blocking
                               );
};

//
// check if ready for read
//

struct UDPEndpointReadyForReadMsg: public UDPEndpointBaseMsg
{
public:
  boolean ready;

public:
  UDPEndpointReadyForReadMsg() : UDPEndpointBaseMsg() {};
  UDPEndpointReadyForReadMsg(
                              antModuleRef& _module
                            );
};

//
// check if ready for write
//

struct UDPEndpointReadyForWriteMsg: public UDPEndpointBaseMsg
{
public:
  boolean ready;

public:
  UDPEndpointReadyForWriteMsg() : UDPEndpointBaseMsg() {};
  UDPEndpointReadyForWriteMsg(
                              antModuleRef& _module
                             );
};

//
// ask address info
//

struct UDPEndpointAddressMsg: public UDPEndpointBaseMsg
{
public:
  IPAddress lAddress;
  Port lPort;
  IPAddress fAddress;
  Port fPort;

public:
  UDPEndpointAddressMsg() : UDPEndpointBaseMsg() {};
  UDPEndpointAddressMsg(
			antModuleRef& _module
		       );
};

//
// reset a connect
//

struct UDPEndpointResetConnectMsg: public UDPEndpointBaseMsg
{
public:
  UDPEndpointResetConnectMsg() : UDPEndpointBaseMsg() {};
  UDPEndpointResetConnectMsg(
			     antModuleRef& _module
			    );
};

#ifdef USE_MULTICAST

//
// multicast extensions to UDP endpoint
//

struct UDPEndpointSetMulticastLoopMsg: public UDPEndpointBaseMsg
{
public:
  UDPEndpointSetMulticastLoopMsg() : UDPEndpointBaseMsg() {};
  UDPEndpointSetMulticastLoopMsg(antModuleRef& _module,
                                 Boolean enable);
  
  Boolean enable;
};

struct UDPEndpointSetMulticastTTLMsg: public UDPEndpointBaseMsg
{
public:
  UDPEndpointSetMulticastTTLMsg(): UDPEndpointBaseMsg(){} 
  UDPEndpointSetMulticastTTLMsg(antModuleRef& _module,
                                byte ttl);
  byte ttl;
};

struct UDPEndpointAddMembershipMsg: public UDPEndpointBaseMsg
{
public:
  UDPEndpointAddMembershipMsg(): UDPEndpointBaseMsg() {}

  UDPEndpointAddMembershipMsg(antModuleRef& _module,
                              IPAddress multiaddr,
                              IPAddress interface);

  IPAddress multiaddr;
  IPAddress interface;
};

struct UDPEndpointDropMembershipMsg: public UDPEndpointBaseMsg
{
public:
  UDPEndpointDropMembershipMsg(): UDPEndpointBaseMsg() {}

  UDPEndpointDropMembershipMsg(antModuleRef& _module,
                               IPAddress multiaddr,
                               IPAddress interface);

  IPAddress multiaddr;
  IPAddress interface;
};

#endif




//
// inline definitions
//
  

inline
UDPEndpointBaseMsg::UDPEndpointBaseMsg(
                                        antModuleRef& _module,
                                        UDPEndpointOperation _operation
                                     ):
    error(UDP_FAIL),
    antEnvMsg(_module, _operation)
{
}

inline
antError
UDPEndpointBaseMsg::Reply(UDPEndpointError _error)
{
  error = _error;
  return antEnvMsg::Reply();
}

inline
UDPEndpointBindMsg::UDPEndpointBindMsg(
                                        antModuleRef& _module,
                                        IPAddress _address, Port _port
                                      ):
    UDPEndpointBaseMsg(_module, UDPEndpoint_Operation_Bind)
{
  // content
  address = _address;
  port = _port;
}

inline
UDPEndpointConnectMsg::UDPEndpointConnectMsg(
                                              antModuleRef& _module,
                                              IPAddress _address, Port _port 
                                            ):
    UDPEndpointBaseMsg(_module, UDPEndpoint_Operation_Connect)
{
  // content
  address = _address;
  port = _port;
}

inline
UDPEndpointSendMsg::UDPEndpointSendMsg(
                                        antModuleRef& _module,
                                        byte* _buffer, int _size
                                      ):
    UDPEndpointBaseMsg(_module, UDPEndpoint_Operation_Send)
{
  // no address specified
  address = IP_ADDR_ANY;
  port = IP_PORT_ANY;

  // data
  buffer = _buffer;
  size = _size;
}

inline
UDPEndpointSendMsg::UDPEndpointSendMsg(
                                        antModuleRef& _module,
                                        IPAddress _address, Port _port,
                                        byte* _buffer, int _size
                                      ):
    UDPEndpointBaseMsg(_module, UDPEndpoint_Operation_Send)
{
  // no address specified
  address = _address;
  port = _port;

  // data
  buffer = _buffer;
  size = _size;
}

inline
UDPEndpointReceiveMsg::UDPEndpointReceiveMsg(
                                              antModuleRef& _module,
                                              byte* _buffer, int _size
                                            ):
    UDPEndpointBaseMsg(_module, UDPEndpoint_Operation_Receive)
{
  buffer = _buffer;
  size = _size;
}

inline
UDPEndpointCloseMsg::UDPEndpointCloseMsg(antModuleRef& _module, bool _linger, int _lingerTime):
    UDPEndpointBaseMsg(_module, UDPEndpoint_Operation_Close)
{
  linger = _linger;
  lingerTime = _lingerTime;
}

inline
UDPEndpointSetBlockingModeMsg::UDPEndpointSetBlockingModeMsg(
                                antModuleRef& _module,
                                bool _blocking
                              ):
    UDPEndpointBaseMsg(_module, UDPEndpoint_Operation_SetBlockingMode)
{
  blocking = _blocking;
}


inline
UDPEndpointReadyForReadMsg::UDPEndpointReadyForReadMsg(
                                antModuleRef& _module
                              ):
    UDPEndpointBaseMsg(_module, UDPEndpoint_Operation_ReadyForRead)
{
}

inline
UDPEndpointReadyForWriteMsg::UDPEndpointReadyForWriteMsg(
                                antModuleRef& _module
                              ):
    UDPEndpointBaseMsg(_module, UDPEndpoint_Operation_ReadyForWrite)
{
}

inline
UDPEndpointAddressMsg::UDPEndpointAddressMsg(
					     antModuleRef& _module
					    ):
    UDPEndpointBaseMsg(_module, UDPEndpoint_Operation_Address)
{
}

inline
UDPEndpointResetConnectMsg::UDPEndpointResetConnectMsg(
						       antModuleRef& _module
						      ):
    UDPEndpointBaseMsg(_module, UDPEndpoint_Operation_ResetConnect)
{
}

#ifdef USE_MULTICAST
inline
UDPEndpointSetMulticastLoopMsg::UDPEndpointSetMulticastLoopMsg(antModuleRef& _module,
                                                               Boolean _enable):
  UDPEndpointBaseMsg(_module, UDPEndpoint_Operation_SetMulticastLoop) 
{
  enable = _enable;
}

inline
UDPEndpointSetMulticastTTLMsg::UDPEndpointSetMulticastTTLMsg(antModuleRef& _module,
                                                             byte _ttl):
  UDPEndpointBaseMsg(_module, UDPEndpoint_Operation_SetMulticastTTL)
{
  ttl = _ttl;
}

inline
UDPEndpointAddMembershipMsg::UDPEndpointAddMembershipMsg(antModuleRef& _module,
                                                         IPAddress _multiaddr,
                                                         IPAddress _interface):
  UDPEndpointBaseMsg(_module, UDPEndpoint_Operation_AddMembership)
{
  multiaddr = _multiaddr;
  interface = _interface;
}

inline
UDPEndpointDropMembershipMsg::UDPEndpointDropMembershipMsg(antModuleRef& _module,
                                                           IPAddress _multiaddr,
                                                           IPAddress _interface):
  UDPEndpointBaseMsg(_module, UDPEndpoint_Operation_DropMembership)
{
  multiaddr = _multiaddr;
  interface = _interface;
} 

#endif





//
// inline printing routines
//

#ifdef DEBUG

#define PERRORCASE(a) case a: _os << #a; break

inline
OSTREAM&
operator<<(OSTREAM& _os, UDPEndpointError _error)
{
  switch(_error)
  {
    PERRORCASE(UDP_SUCCESS);
    PERRORCASE(UDP_FAIL);
    PERRORCASE(UDP_CONNECTION_CLOSED);
    PERRORCASE(UDP_CONNECTION_BUSY);
    PERRORCASE(UDP_ADDRESSINUSE);
    PERRORCASE(UDP_ADDRESSERROR);
    PERRORCASE(UDP_PORT_UNREACHABLE);
    PERRORCASE(UDP_PROTOCOL_UNREACHABLE);
    PERRORCASE(UDP_HOST_UNREACHABLE);
    PERRORCASE(UDP_NETWORK_UNREACHABLE);
    PERRORCASE(UDP_MESSAGE_TOO_LONG);
    PERRORCASE(UDP_TTL_EXCEEDED);
    PERRORCASE(UDP_TIME_EXCEEDED);
    PERRORCASE(UDP_BUFFER_INVALID);
    PERRORCASE(UDP_OPERATION_INVALID);
    PERRORCASE(UDP_OPERATION_UNKNOWN);

    default:
      _os << "Unknow(" << (int)_error << ")";
  }

  return _os;
}

#endif

#endif
