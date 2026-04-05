/*
 *  Copyright 2002 Sony Corporation
 */
#ifndef EtherTypes_h_defined_
#define EtherTypes_h_defined_

#include <OID.h>
#include <Types.h>
#include <string.h>

#ifdef __GNUC__
#include <iostream>
#define OSTREAM std::ostream
#else /* __GNUC__ */
#ifndef OSTREAM
#include <iostream.h>
#define OSTREAM ostream
#endif
#endif /* __GNUC__ */

struct EtherMACAddress
{
  struct Opt
  {
    uint16 w1;
    uint16 w2;
    uint16 w3;
  };
  union
  {
    uint8 octet[6];
    Opt opt;
  };
#ifdef __cplusplus
  EtherMACAddress();
  EtherMACAddress(const EtherMACAddress& _addr);
  EtherMACAddress(byte* b);
  EtherMACAddress(byte b0, byte b1, byte b2, byte b3, byte b4, byte b5);

  bool operator==(const EtherMACAddress& _addr);
  bool IsBroadcast();
  bool IsMulticast();

  void ToByte(byte* _dst) const;
#endif
};

#ifdef __cplusplus

inline
EtherMACAddress::EtherMACAddress()
{
}

inline
EtherMACAddress::EtherMACAddress(const EtherMACAddress& _addr)
{
  opt.w1 = _addr.opt.w1;
  opt.w2 = _addr.opt.w2;
  opt.w3 = _addr.opt.w3;
}

inline
EtherMACAddress::EtherMACAddress(byte b0, byte b1, byte b2, byte b3, byte b4, byte b5) 
{
  octet[0] = b0;
  octet[1] = b1;
  octet[2] = b2;
  octet[3] = b3;
  octet[4] = b4;
  octet[5] = b5;
}

inline
EtherMACAddress::EtherMACAddress(byte* b)
{
  memcpy(octet, b, 6);
}

inline
bool
EtherMACAddress::operator==(const EtherMACAddress& _addr)
{
  return (
          (opt.w1 == _addr.opt.w1) &&
          (opt.w2 == _addr.opt.w2) &&
          (opt.w3 == _addr.opt.w3)
         );
}

inline
bool
EtherMACAddress::IsBroadcast()
{
  return ((0xffff == opt.w1) && (0xffff == opt.w2) && (0xffff == opt.w3));
}

inline
bool
EtherMACAddress::IsMulticast()
{
  return ((octet[0] & 0x1) && !IsBroadcast());
}

inline
void
EtherMACAddress::ToByte(byte* _dst) const
{
  _dst[0] = octet[0];
  _dst[1] = octet[1];
  _dst[2] = octet[2];
  _dst[3] = octet[3];
  _dst[4] = octet[4];
  _dst[5] = octet[5];
}



inline
void
print_byte_in_hex(OSTREAM& os, byte b)
{
  char*  digits = "0123456789abcdef";
  os << digits[b / 16];
  os << digits[b % 16];
}

inline
OSTREAM& operator<<(OSTREAM& os, const EtherMACAddress& addr)
{
  print_byte_in_hex(os, addr.octet[0]); os << ":";
  print_byte_in_hex(os, addr.octet[1]); os << ":";
  print_byte_in_hex(os, addr.octet[2]); os << ":";
  print_byte_in_hex(os, addr.octet[3]); os << ":";
  print_byte_in_hex(os, addr.octet[4]); os << ":";
  print_byte_in_hex(os, addr.octet[5]); 

  return os;
}

#endif


enum EtherFrameType
{
  ETHER_FRAMETYPE_ETHERNET_II,
  ETHER_FRAMETYPE_IEEE_802_3
};

typedef uint16 EtherProtocolType;

#define ETHER_P_LOOP   0x0060
#define ETHER_P_ECHO   0x0200
#define ETHER_P_PUP    0x0400
#define ETHER_P_IP     0x0800
#define ETHER_P_X25    0x0805
#define ETHER_P_ARP    0x0806
#define ETHER_P_BPQ    0x08FF
#define ETHER_P_DEC    0x6000
#define ETHER_P_DNA_DL 0x6001
#define ETHER_P_DNA_RC 0x6002
#define ETHER_P_DNA_RT 0x6003
#define ETHER_P_LAT    0x6004
#define ETHER_P_DIAG   0x6005
#define ETHER_P_CUST   0x6006
#define ETHER_P_SCA    0x6007
#define ETHER_P_RARP   0x8035
#define ETHER_P_ATALK  0x809B
#define ETHER_P_AARP   0x80F3
#define ETHER_P_IPX    0x8137
#define ETHER_P_IPV6   0x86DD

struct EtherFrameBufferID
{
  MemoryRegionID rcvID;
  size_t   rcvsize;
  void*    rcvbase;

  MemoryRegionID txID;
  size_t   txsize;
  void*    txbase;
};




#define ETHER_ADDR_LEN 6

struct  ether_header 
{
  u_char  ether_dhost[ETHER_ADDR_LEN];
  u_char  ether_shost[ETHER_ADDR_LEN];
  u_short ether_type;
};

struct ether_802_3_header
{
  u_char  ether_dhost[ETHER_ADDR_LEN];
  u_char  ether_shost[ETHER_ADDR_LEN];
  u_short ether_size;
  u_char  dsap;
  u_char  ssap;
  u_char  ctrl;
  u_char  code[3];
  u_short ether_type; 
};

enum EtherStatus
{
  ETHER_OK = 0,
  ETHER_UNSUPPORTED,
  ETHER_NO_MEMORY,
  ETHER_INVALID_PROTOCOL,
  ETHER_INVALID_ADDRESS,
  ETHER_INVALID_PORT,
  ETHER_INVALID_ATTR,
  ETHER_NO_CLIENT_ENTRY,
  ETHER_INVALID_HANDLE,
  ETHER_RCV_IDLE,
  ETHER_RCV_BUSY,
  ETHER_NO_ADDRESS_ENTRY,
  ETHER_NO_DEVICE
};

#ifndef NDEBUG
static char* _etherStatusStr[] =
{ 
  "ETHER_OK",
  "ETHER_UNSUPPORTED",
  "ETHER_NO_MEMORY",
  "ETHER_INVALID_PROTOCOL",
  "ETHER_INVALID_ADDRESS",
  "ETHER_INVALID_PORT",
  "ETHER_INVALID_ATTR",
  "ETHER_NO_CLIENT_ENTRY",
  "ETHER_INVALID_HANDLE",
  "ETHER_RCV_IDLE",
  "ETHER_RCV_BUSY",
  "ETHER_NO_ADDRESS_ENTRY",
  "ETHER_NO_DEVICE"
};

#define etherStatusStr(s) _etherStatusStr[s]

#else  // NDEBUG

#define etherStatusStr(s) ""

#endif // NDEBUG


struct EtherDriverStatistics
{
 uint32 carrier_sense_error_count;
 uint32 late_collision_count;
 uint32 excessive_collision_count;

 uint32 single_collision_count;
 uint32 multiple_collision_count;
 uint32 deferred_count;

 uint32 alignment_error_count;
 uint32 fcs_error_count;
 uint32 frame_too_long_count;

};

#endif

