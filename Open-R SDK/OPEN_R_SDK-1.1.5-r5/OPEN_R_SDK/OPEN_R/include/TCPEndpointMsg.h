/*
 *  Copyright 2002 Sony Corporation
 */
#ifndef _TCPEndpointMsg_H_
#define _TCPEndpointMsg_H_

#include <antEnvMsg.h>
#include <IPAddress.h>

// ========================================================
//
// error values
//
// ========================================================

enum TCPEndpointError
{
  TCP_SUCCESS = 0,
  TCP_FAIL = 1,

  TCP_CONNECTION_CLOSED = 2,
  TCP_CONNECTION_RESET = 3,
  TCP_CONNECTION_TIMEOUT = 4,
  TCP_CONNECTION_BUSY = 5,

  TCP_ADDRESSINUSE = 6,
  TCP_ADDRESSERROR = 7,

  TCP_PORT_UNREACHABLE = 8,
  TCP_PROTOCOL_UNREACHABLE = 9,
  TCP_HOST_UNREACHABLE = 10,
  TCP_NETWORK_UNREACHABLE = 11,
  TCP_MESSAGE_TOO_LONG = 12,
  TCP_TTL_EXCEEDED = 13,
  TCP_TIME_EXCEEDED = 14,

  TCP_WOULDBLOCK = 15,

  TCP_BUFFER_INVALID = 16,

  TCP_OPERATION_INVALID = 17,
  TCP_OPERATION_UNKNOWN = 18
};

// ========================================================
//
// external messages to the SOCKET module
//
// ========================================================

enum TCPEndpointOperation
{
  TCPEndpoint_Operation_Bind = 1,
  TCPEndpoint_Operation_Listen = 2,
  TCPEndpoint_Operation_Connect = 3,
  TCPEndpoint_Operation_Send = 4,
  TCPEndpoint_Operation_Receive = 5,
  TCPEndpoint_Operation_Close = 6,
  TCPEndpoint_Operation_SetBlockingMode = 7,
  TCPEndpoint_Operation_ReadyForRead = 8,
  TCPEndpoint_Operation_ReadyForWrite = 9,
  TCPEndpoint_Operation_BacklogListen = 10,
  TCPEndpoint_Operation_Accept = 11,
  TCPEndpoint_Operation_BacklogQueues = 12,
  TCPEndpoint_Operation_Address = 13
};

//
// base message
//

struct TCPEndpointBaseMsg: public antEnvMsg
{
public:
  TCPEndpointError error;

public:
  TCPEndpointBaseMsg() : error(TCP_FAIL), antEnvMsg() {};
  TCPEndpointBaseMsg(
                      antModuleRef& _module,
                      TCPEndpointOperation _operation
                    );

  antError Reply(TCPEndpointError _error);
};

//
// backlogQueue
//

struct TCPEndpointBacklogQueueMsg: public TCPEndpointBaseMsg
{
public:
  int16 backlog;
  
public:
  TCPEndpointBacklogQueueMsg() : TCPEndpointBaseMsg() {};
  TCPEndpointBacklogQueueMsg(
                      antModuleRef& _module,
                      int16 _backlog
                    );
};

//
// bind
//

struct TCPEndpointBindMsg: public TCPEndpointBaseMsg
{
public:
  IPAddress lAddress;
  Port lPort;
  
public:
  TCPEndpointBindMsg() : TCPEndpointBaseMsg() {};
  TCPEndpointBindMsg(
                      antModuleRef& _module,
                      IPAddress _lAddress, Port _lPort
                    );
};

//
// listen
//

struct TCPEndpointListenMsg: public TCPEndpointBaseMsg
{
public:
  IPAddress lAddress;
  Port lPort;
  IPAddress fAddress;
  Port fPort;
  
public:
  TCPEndpointListenMsg() : TCPEndpointBaseMsg() {};
  TCPEndpointListenMsg(
                        antModuleRef& _module,
                        IPAddress _lAddress = 0, Port _lPort = 0,
                        IPAddress _fAddress = 0, Port _fPort = 0
                      );
};

struct TCPEndpointBacklogListenMsg: public TCPEndpointBaseMsg
{
public:
  IPAddress lAddress;
  Port lPort;
  IPAddress fAddress;
  Port fPort;
  
public:
  TCPEndpointBacklogListenMsg() : TCPEndpointBaseMsg() {};
  TCPEndpointBacklogListenMsg(
                        antModuleRef& _module,
                        IPAddress _lAddress = 0, Port _lPort = 0,
                        IPAddress _fAddress = 0, Port _fPort = 0
                      );
};

//
// accept
//

struct TCPEndpointAcceptMsg: public TCPEndpointBaseMsg
{
public:
  IPAddress lAddress;
  Port lPort;
  IPAddress fAddress;
  Port fPort;
  int16 queueNr;
  
public:
  TCPEndpointAcceptMsg() : TCPEndpointBaseMsg() {};
  TCPEndpointAcceptMsg(
                        antModuleRef& _module
                      );
};

//
// connect
//

struct TCPEndpointConnectMsg: public TCPEndpointBaseMsg
{
public:
  IPAddress lAddress;
  Port lPort;
  IPAddress fAddress;
  Port fPort;
  
public:
  TCPEndpointConnectMsg() : TCPEndpointBaseMsg() {};
  TCPEndpointConnectMsg(
                        antModuleRef& _module,
                        IPAddress _lAddress, Port _lPort,
                        IPAddress _fAddress, Port _fPort
                       );
};

//
// send
//

struct TCPEndpointSendMsg: public TCPEndpointBaseMsg
{
public:
  byte* buffer;
  int size;
  int16 queueNr;

public:
  TCPEndpointSendMsg() : TCPEndpointBaseMsg() {};
  TCPEndpointSendMsg(
                      antModuleRef& _module,
                      byte* _buffer, int _size,
		      int16 _queueNr = 0
                    );
};

//
// receive
//

struct TCPEndpointReceiveMsg: public TCPEndpointBaseMsg
{
public:
  byte* buffer;
  int sizeMin;
  int sizeMax;
  int16 queueNr;
  
public:
  TCPEndpointReceiveMsg() : TCPEndpointBaseMsg() {};
  TCPEndpointReceiveMsg(
                        antModuleRef& _module,
                        byte* _buffer,
                        int _sizeMin, int _sizeMax,
			int16 _queueNr = 0
                       );
};

//
// close
//

struct TCPEndpointCloseMsg: public TCPEndpointBaseMsg
{
public:
  bool abort;
  bool linger;
  int lingerTime;
  int16 queueNr;

public:
  TCPEndpointCloseMsg() : TCPEndpointBaseMsg() {};
  TCPEndpointCloseMsg(
                      antModuleRef& _module,
                      bool _linger = false,
                      int _lingerTime = 0,
                      bool _abort = false,
		      int16 _queueNr = -100
                     );
};


//
// set blocking mode
//

struct TCPEndpointSetBlockingModeMsg: public TCPEndpointBaseMsg
{
public:
  bool blocking;
  int16 queueNr;

public:
  TCPEndpointSetBlockingModeMsg() : TCPEndpointBaseMsg() {};
  TCPEndpointSetBlockingModeMsg(
                                antModuleRef& _module,
                                bool _blocking,
				int16 _queueNr = 0
                               );
};

//
// check if ready for read
//

struct TCPEndpointReadyForReadMsg: public TCPEndpointBaseMsg
{
public:
  boolean ready;
  int16 queueNr;

public:
  TCPEndpointReadyForReadMsg() : TCPEndpointBaseMsg() {};
  TCPEndpointReadyForReadMsg(
                              antModuleRef& _module,
			      int16 _queueNr = 0
                            );
};

//
// check if ready for write
//

struct TCPEndpointReadyForWriteMsg: public TCPEndpointBaseMsg
{
public:
  boolean ready;
  int16 queueNr;

public:
  TCPEndpointReadyForWriteMsg() : TCPEndpointBaseMsg() {};
  TCPEndpointReadyForWriteMsg(
                              antModuleRef& _module,
			      int16 _queueNr = 0
                             );
};

//
// ask address info
//

struct TCPEndpointAddressMsg: public TCPEndpointBaseMsg
{
public:
  int16 queueNr;
  IPAddress lAddress;
  Port lPort;
  IPAddress fAddress;
  Port fPort;

public:
  TCPEndpointAddressMsg() : TCPEndpointBaseMsg() {};
  TCPEndpointAddressMsg(
			antModuleRef& _module,
			int16 _queueNr = 0
		       );
};

//
// inline definitions
//

inline
TCPEndpointBaseMsg::TCPEndpointBaseMsg(
                                        antModuleRef& _module,
                                        TCPEndpointOperation _operation
                                      ):
    error(TCP_FAIL), antEnvMsg(_module, _operation)
{
}

inline
antError
TCPEndpointBaseMsg::Reply(TCPEndpointError _error)
{
  error = _error;
  return antEnvMsg::Reply();
}

inline
TCPEndpointBacklogQueueMsg::TCPEndpointBacklogQueueMsg(
                                  antModuleRef& _module,
                                  int16 _backlog
                                 ):
    TCPEndpointBaseMsg(_module, TCPEndpoint_Operation_BacklogQueues)
{
  // content
  backlog = _backlog;
}

inline
TCPEndpointBindMsg::TCPEndpointBindMsg(
                                  antModuleRef& _module,
                                  IPAddress _lAddress,
                                  Port _lPort
                                 ):
    TCPEndpointBaseMsg(_module, TCPEndpoint_Operation_Bind)
{
  // content
  lAddress = _lAddress;
  lPort = _lPort;
}

inline
TCPEndpointListenMsg::TCPEndpointListenMsg(
                                  antModuleRef& _module,
                                  IPAddress _lAddress,
                                  Port _lPort, 
                                  IPAddress _fAddress,
                                  Port _fPort
                                 ):
    TCPEndpointBaseMsg(_module, TCPEndpoint_Operation_Listen)
{
  // content
  lAddress = _lAddress;
  lPort = _lPort;
  fAddress = _fAddress;
  fPort = _fPort;
}

inline
TCPEndpointBacklogListenMsg::TCPEndpointBacklogListenMsg(
                                  antModuleRef& _module,
                                  IPAddress _lAddress,
                                  Port _lPort, 
                                  IPAddress _fAddress,
                                  Port _fPort
                                 ):
    TCPEndpointBaseMsg(_module, TCPEndpoint_Operation_BacklogListen)
{
  // content
  lAddress = _lAddress;
  lPort = _lPort;
  fAddress = _fAddress;
  fPort = _fPort;
}

inline
TCPEndpointAcceptMsg::TCPEndpointAcceptMsg(
                                  antModuleRef& _module
				  ):
    TCPEndpointBaseMsg(_module, TCPEndpoint_Operation_Accept)
{
}

inline
TCPEndpointConnectMsg::TCPEndpointConnectMsg(
                                    antModuleRef& _module,
                                    IPAddress _lAddress,
                                    Port _lPort, 
                                    IPAddress _fAddress,
                                    Port _fPort
                                  ):
    TCPEndpointBaseMsg(_module, TCPEndpoint_Operation_Connect)
{
  // content
  lAddress = _lAddress;
  lPort = _lPort;
  fAddress = _fAddress;
  fPort = _fPort;
}

inline
TCPEndpointSendMsg::TCPEndpointSendMsg(
                              antModuleRef& _module,
                              byte* _buffer,
                              int _size,
			      int16 _queueNr
                             ):
    TCPEndpointBaseMsg(_module, TCPEndpoint_Operation_Send)
{
  buffer = _buffer;
  size = _size;
  queueNr = _queueNr;
}

inline
TCPEndpointReceiveMsg::TCPEndpointReceiveMsg(
                                  antModuleRef& _module,
                                  byte* _buffer,
                                  int _sizeMin,
                                  int _sizeMax,
				  int16 _queueNr
                                ):
    TCPEndpointBaseMsg(_module, TCPEndpoint_Operation_Receive)
{
  buffer = _buffer;
  sizeMin = _sizeMin;
  sizeMax = _sizeMax;
  queueNr = _queueNr;
}

inline
TCPEndpointCloseMsg::TCPEndpointCloseMsg(
                                antModuleRef& _module,
                                bool _linger,
                                int _lingerTime,
                                bool _abort,
				int16 _queueNr
                              ):
    TCPEndpointBaseMsg(_module, TCPEndpoint_Operation_Close)
{
  abort = _abort;
  linger = _linger;
  lingerTime = _lingerTime;
  queueNr = _queueNr;
}

inline
TCPEndpointSetBlockingModeMsg::TCPEndpointSetBlockingModeMsg(
                                antModuleRef& _module,
                                bool _blocking,
				int16 _queueNr
                              ):
    TCPEndpointBaseMsg(_module, TCPEndpoint_Operation_SetBlockingMode)
{
  blocking = _blocking;
  queueNr = _queueNr;
}


inline
TCPEndpointReadyForReadMsg::TCPEndpointReadyForReadMsg(
                                antModuleRef& _module,
				int16 _queueNr
                              ):
    TCPEndpointBaseMsg(_module, TCPEndpoint_Operation_ReadyForRead)
{
  queueNr = _queueNr;
}

inline
TCPEndpointReadyForWriteMsg::TCPEndpointReadyForWriteMsg(
                                antModuleRef& _module,
				int16 _queueNr
                              ):
    TCPEndpointBaseMsg(_module, TCPEndpoint_Operation_ReadyForWrite)
{
  queueNr = _queueNr;
}

inline
TCPEndpointAddressMsg::TCPEndpointAddressMsg(
					     antModuleRef& _module,
					     int16 _queueNr
					    ):
    TCPEndpointBaseMsg(_module, TCPEndpoint_Operation_Address)
{
  queueNr = _queueNr;
}

#endif

