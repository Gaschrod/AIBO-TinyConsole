/*
 *  Copyright 2002 Sony Corporation
 */
#ifndef _IPAddress_h_DEFINED
#define _IPAddress_h_DEFINED

#include <antTypes.h>
#include <antIOStream.h>
#include <Internet.h>

//
// IP address
//

struct IPAddress
{
  //
  // constructor
  // 
  
  IPAddress(): address(0) {}
  IPAddress(const IPAddress& _address): address(_address.address) {}
  IPAddress(uint32 _address): address(_address) {}
  IPAddress(int _address): address((uint32)_address) {}
  IPAddress(byte _b1, byte _b2, byte _b3, byte _b4);
  IPAddress(const char* str);

  // 
  // comparison
  // 
  
  boolean operator==(const IPAddress& _ip) const {return address == _ip.address;}
  boolean operator!=(const IPAddress& _ip) const {return address != _ip.address;}

  boolean operator<(const IPAddress& _ip) const {return address < _ip.address;}
  boolean operator<(const uint32 _address) const {return address < _address;}
   
  //
  // logical operators
  //
  
  IPAddress operator&(const IPAddress& _ip) const {return IPAddress(address & _ip.address);}
  IPAddress operator^(const IPAddress& _ip) const {return IPAddress(address ^ _ip.address);}

  //
  // network operators
  //
  boolean InMulticast() const;
  boolean InBroadcast() const;
  boolean InLoop() const;
  boolean AllHostsGroup() const { return (0xe0000001 == address); } 

  //
  // conversion
  // 
 
  uint32 Address() const {return address;}
  char * GetAsString(char*) const; // Size of buffer must at least 16
  
  OSTREAM& print(OSTREAM& os) const;
private:
  // Private methods
  char* intToAscii(char *buffer, int number, bool first=true) const;
  
  // Private attributes
  uint32 address;
};

//
// Port
//

typedef uint16 Port;

//
// some constants
//

extern const IPAddress IP_ADDR_ANY;
extern const IPAddress IP_ADDR_BROADCAST;
#define IP_PORT_ANY 0

inline
OSTREAM& operator<<(OSTREAM& os, const IPAddress& addr)
{
  return addr.print(os);
}

inline
boolean
IPAddress::InMulticast() const
{
  return (0xe0000000 == (address & 0xf0000000));
}

inline
boolean
IPAddress::InBroadcast() const
{
  return (0xffffffff == address);
}

inline
boolean
IPAddress::InLoop() const
{
  return (0x7f000000 == (address & 0xff000000));
}

#endif

