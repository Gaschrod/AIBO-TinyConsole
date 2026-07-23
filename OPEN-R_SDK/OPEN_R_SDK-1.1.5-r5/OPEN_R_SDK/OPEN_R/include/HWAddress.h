/*
 *  Copyright 2002 Sony Corporation
 */
#ifndef _HWAddress_h_DEFINED
#define _HWAddress_h_DEFINED

#include <antTypes.h>
#include <antIOStream.h>

//
// Hardware Address, holds address information related to a certain type
// of network interface
//
// Currently supports:
// Ethernet MAC address
//

#define HWADDRESS_MAX_LEN 32

enum HWAddressType
{
  HWADDRESS_INVALID,
  HWADDRESS_MAC
};

struct HWAddress
{
  //
  // constructor
  // 
  
  HWAddress(): hwType(HWADDRESS_INVALID), hwLen(0) {};
  HWAddress(const HWAddress& _address);
  HWAddress(HWAddressType _type, const byte* _content);

  // 
  // assignment
  // 
  
  HWAddress& operator=(const HWAddress& _hw);

  // 
  // comparison
  // 
  
  boolean operator==(const HWAddress& _hw) const;
   
  //
  // conversion
  // 
  OSTREAM& print(OSTREAM& os) const;

  //
  // init
  //
  void Init(void);

  //
  // content
  //
  void GetContent(int32& len, byte*& content);
  void ToByte(byte* _dest);

private:
  // type of address
  HWAddressType hwType;

  // address content
  int32 hwLen;
  byte hwContent[32];
};

inline
OSTREAM& operator<<(OSTREAM& os, const HWAddress& addr)
{
  return addr.print(os);
}

inline
void
HWAddress::GetContent(int32& len, byte*& content)
{
  len = hwLen;
  content = hwContent;
}

inline
void
HWAddress::ToByte(byte* _dest)
{
  int len = hwLen;
  byte* src = hwContent;

  while(len--) *_dest++ = *src++;
}

inline
void
HWAddress::Init(void)
{
  hwType = HWADDRESS_INVALID;
  hwLen = 0;
}

#endif
