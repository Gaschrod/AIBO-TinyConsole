/*
 *  Copyright 2002 Sony Corporation
 */
#ifndef _DNSEndpointMsg_h_DEFINED
#define _DNSEndpointMsg_h_DEFINED

#include "DNSEndpointErr.h"

#include <antEnvBaseMsg.h>
#include <IPAddress.h>
#include <string.h>

#define MAXNS 3

//
// if we follow the DNS specification this should be 255
// however:
//  the default max message size of Aperios is 255 bytes, so a
//  message containing a 255 bytes domain name would never be delivered
//  

#define MAXDNAME 100

#define DNSENDPOINT_OPERATION_NUM  9


// ========================================================
//
// external messages
//
// ========================================================

enum DNSEndpointOperation
{
  DNSEndpoint_Operation_SetServerAddresses = 0,
  DNSEndpoint_Operation_GetServerAddresses,
  DNSEndpoint_Operation_SetDefaultDomainName,
  DNSEndpoint_Operation_GetDefaultDomainName,
  DNSEndpoint_Operation_GetHostByName,
  DNSEndpoint_Operation_GetHostByAddr,
  DNSEndpoint_Operation_GetAddress,
  DNSEndpoint_Operation_GetAlias,
  DNSEndpoint_Operation_Close
};

//
// base message
// 

struct DNSEndpointBaseMsg: public antEnvMsg
{
public:
  DNSEndpointError error;

public:
  DNSEndpointBaseMsg(): error(DNS_FAIL), antEnvMsg() {};

  DNSEndpointBaseMsg(DNSEndpointBaseMsg& _msg)
    : antEnvMsg(_msg), error(_msg.error) {}

  DNSEndpointBaseMsg(antModuleRef& _module, DNSEndpointOperation _operation)
    : error(DNS_FAIL), antEnvMsg(_module, _operation) {}
  
  antError Reply(DNSEndpointError _error)
  {
    error = _error;
    return antEnvMsg::Reply();
  }
};

//
// set server
// 

struct DNSEndpointSetServerAddressesMsg: public DNSEndpointBaseMsg
{
public:
  int32 nscount;
  IPAddress addrList[MAXNS];

public:
  DNSEndpointSetServerAddressesMsg():DNSEndpointBaseMsg(){}

  DNSEndpointSetServerAddressesMsg(DNSEndpointSetServerAddressesMsg& _msg)
    :nscount(_msg.nscount)
  {
    memcpy(addrList, _msg.addrList, sizeof(IPAddress)*_msg.nscount);
  }
    
  DNSEndpointSetServerAddressesMsg(antModuleRef& _module,
				   int32 _nscount, IPAddress* _addrList)
    :DNSEndpointBaseMsg(_module, DNSEndpoint_Operation_SetServerAddresses),
     nscount(_nscount)
  {
    memcpy(addrList, _addrList, sizeof(IPAddress)*_nscount);
  }
};

//
// get server
//

struct DNSEndpointGetServerAddressesMsg: public DNSEndpointBaseMsg
{
public:
  int32 nscount;
  IPAddress addrList[MAXNS];

public:
  DNSEndpointGetServerAddressesMsg():DNSEndpointBaseMsg(){}

  DNSEndpointGetServerAddressesMsg(DNSEndpointGetServerAddressesMsg& _msg)
    :nscount(_msg.nscount)
  {
    memcpy(addrList, _msg.addrList, sizeof(IPAddress)*_msg.nscount);
  }
    
  DNSEndpointGetServerAddressesMsg(antModuleRef& _module)
    :DNSEndpointBaseMsg(_module, DNSEndpoint_Operation_GetServerAddresses) {}
};

//
// set default domain name
//

struct DNSEndpointSetDefaultDomainNameMsg: public DNSEndpointBaseMsg
{
public:
  char name[MAXDNAME];

public:
  DNSEndpointSetDefaultDomainNameMsg():DNSEndpointBaseMsg(){}

  DNSEndpointSetDefaultDomainNameMsg(DNSEndpointSetDefaultDomainNameMsg& _msg)
    :DNSEndpointBaseMsg(_msg)
  {
    strncpy(name, _msg.name, MAXDNAME);
  }

  DNSEndpointSetDefaultDomainNameMsg(antModuleRef& _module, char* _name)
    :DNSEndpointBaseMsg(_module, DNSEndpoint_Operation_SetDefaultDomainName)
  {
    strncpy(name, _name, MAXDNAME);
  }
};

struct DNSEndpointGetDefaultDomainNameMsg: public DNSEndpointBaseMsg
{
public:
  char name[MAXDNAME];

public:
  DNSEndpointGetDefaultDomainNameMsg():DNSEndpointBaseMsg(){}

  DNSEndpointGetDefaultDomainNameMsg(DNSEndpointGetDefaultDomainNameMsg& _msg)
    :DNSEndpointBaseMsg(_msg)
  {
    strncpy(name, _msg.name, MAXDNAME);
  }

  DNSEndpointGetDefaultDomainNameMsg(antModuleRef& _module)
    :DNSEndpointBaseMsg(_module, DNSEndpoint_Operation_GetDefaultDomainName){}
};

//
// get host by name
// 

struct DNSEndpointGetHostByNameMsg: public DNSEndpointBaseMsg
{
public:
  char name[MAXDNAME];
  IPAddress server_address;
  IPAddress host_address;
  int n_address;
  int n_alias;
  
public:
  DNSEndpointGetHostByNameMsg():DNSEndpointBaseMsg(){}

  DNSEndpointGetHostByNameMsg(DNSEndpointGetHostByNameMsg& _msg)
    :DNSEndpointBaseMsg(_msg),
     server_address(_msg.server_address),host_address(_msg.host_address),
     n_address(_msg.n_address),n_alias(_msg.n_alias)
  {
    strncpy(name, _msg.name, MAXDNAME);
  }

  DNSEndpointGetHostByNameMsg(antModuleRef& _module, const char* _name)
    :DNSEndpointBaseMsg(_module, DNSEndpoint_Operation_GetHostByName)
  {
    strncpy(name, _name, MAXDNAME);
  }
};

//
// get host by address
// 

struct DNSEndpointGetHostByAddrMsg: public DNSEndpointBaseMsg
{
public:
  char name[MAXDNAME];
  IPAddress server_address;
  IPAddress host_address;
  int n_address;
  int n_alias;
  
public:
  DNSEndpointGetHostByAddrMsg():DNSEndpointBaseMsg(){}

  DNSEndpointGetHostByAddrMsg(DNSEndpointGetHostByAddrMsg& _msg)
    :DNSEndpointBaseMsg(_msg),
     server_address(_msg.server_address),host_address(_msg.host_address),
     n_address(_msg.n_address),n_alias(_msg.n_alias)
  {
    strncpy(name, _msg.name, MAXDNAME);
  }

  DNSEndpointGetHostByAddrMsg(antModuleRef& _module, IPAddress _address)
    :DNSEndpointBaseMsg(_module, DNSEndpoint_Operation_GetHostByAddr),
     host_address(_address){}
};

//
// get address
// 

struct DNSEndpointGetAddressMsg: public DNSEndpointBaseMsg
{
public:
  int index;
  IPAddress address;
  
public:
  DNSEndpointGetAddressMsg():DNSEndpointBaseMsg(){}

  DNSEndpointGetAddressMsg(DNSEndpointGetAddressMsg& _msg)
    :DNSEndpointBaseMsg(_msg), index(_msg.index), address(_msg.address) {}

  DNSEndpointGetAddressMsg(antModuleRef& _module, int _index)
    :DNSEndpointBaseMsg(_module, DNSEndpoint_Operation_GetAddress),index(_index){}
};


struct DNSEndpointGetAliasMsg: public DNSEndpointBaseMsg
{
public:
  int index;
  char name[MAXDNAME];
  
public:
  DNSEndpointGetAliasMsg():DNSEndpointBaseMsg(){}

  DNSEndpointGetAliasMsg(DNSEndpointGetAliasMsg& _msg)
    :DNSEndpointBaseMsg(_msg), index(_msg.index)
  {
    strncpy(name, _msg.name, MAXDNAME);
  }

  DNSEndpointGetAliasMsg(antModuleRef& _module, int _index)
    :DNSEndpointBaseMsg(_module, DNSEndpoint_Operation_GetAlias),index(_index){}
};

//
// close
//

struct DNSEndpointCloseMsg: public DNSEndpointBaseMsg
{
  DNSEndpointCloseMsg():DNSEndpointBaseMsg(){}

  DNSEndpointCloseMsg(DNSEndpointCloseMsg& _msg)
    :DNSEndpointBaseMsg(_msg){}

  DNSEndpointCloseMsg(antModuleRef& _module)
    :DNSEndpointBaseMsg(_module, DNSEndpoint_Operation_Close){}
};


#endif
