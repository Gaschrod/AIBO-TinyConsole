/*
 *  Copyright 2002 Sony Corporation
 */
#ifndef _DNSEndpointErr_h_DEFINED
#define _DNSEndpointErr_h_DEFINED

// ========================================================
//
// error values
//
// ========================================================

enum DNSEndpointError
{
  DNS_SUCCESS = 0,
  DNS_FAIL,

  DNS_HOST_NOT_FOUND,
  DNS_TRY_AGAIN,
  DNS_NO_RECOVERY,
  DNS_NO_DATA,

  DNS_CONNECTION_CLOSED,
  DNS_CONNECTION_BUSY,

  DNS_INDEX_INVALID,
  DNS_BUFFER_INVALID,

  DNS_OPERATION_INVALID,
  DNS_OPERATION_UNKNOWN
};

#endif
