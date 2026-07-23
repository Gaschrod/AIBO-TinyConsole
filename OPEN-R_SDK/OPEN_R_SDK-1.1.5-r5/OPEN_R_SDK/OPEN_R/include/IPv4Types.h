/*
 *  Copyright 2002 Sony Corporation
 */
#ifndef _IPv4Types_h_DEFINED
#define _IPv4Types_h_DEFINED

#include <antTypes.h>
#include <antIOStream.h>
#include <Internet.h>

#include <IPAddress.h>
#include <HWAddress.h>

//
// Error values
//

enum IPv4Error
{
  IPV4_SUCCESS = 0,
  IPV4_FAIL = 1,

  // general errors
  IPV4_NOMEMORY = 2,

  // address - port related
  IPV4_ADDRESSINUSE = 3,
  IPV4_ADDRESSERROR = 4,

  // service related
  IPV4_INVALID_REQUEST = 5,

  // connection related
  IPV4_CONNECTION_CLOSED = 6,
  IPV4_CONNECTION_BUSY = 7,
  IPV4_CONNECTION_RESET = 8,
  IPV4_CONNECTION_TIMEOUT = 9,

  // network related
  IPV4_NETWORK_UNREACHABLE = 10,
  IPV4_HOST_UNREACHABLE = 11,
  IPV4_PROTOCOL_UNREACHABLE = 12,
  IPV4_PORT_UNREACHABLE = 13,
  IPV4_MESSAGE_TOO_LONG = 14,
  IPV4_TIME_EXCEEDED = 15,
  IPV4_TTL_EXCEEDED = 16
};

OSTREAM& operator<<(OSTREAM& os, const IPv4Error error);

#endif

