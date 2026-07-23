/*
 *  Copyright 2002 Sony Corporation
 */
#ifndef _IPProtocol_h_DEFINED_
#define _IPProtocol_h_DEFINED_

#include <ant.h>
#include <Util.h>
#include <IPv4Types.h>
#include <antIOStream.h>

// Definitions for internet protocol version 4.
// Per RFC 791, September 1981.

#define IPVERSION   4

// Structure of an internet header, naked of options.
//
// We declare ip_len and ip_off to be short, rather than uint16
// pragmatically since otherwise unsigned comparisons can result
// against negative integers quite easily, and fail in subtle ways.

// WARNING:
// We made ip_len and ip_off uint16 otherwise we can not send packets
// larger then 32 Kb 

struct IPHeader
{
public:
  // we are not sure about the correctness of the return types of these ones
  uint16 ip_vGet() const {return (ip_v_hl & 0xf0) >> 4;};
  uint16 ip_hlGet() const {return (ip_v_hl & 0x0f);};
    
  // extract information
  byte ip_tosGet() const {return ip_tos;};
  uint16 ip_lenGet() const {return byte2short(ip_len);};
  uint16 ip_idGet() const {return byte2short(ip_id);};
  uint16 ip_offGet() const {return byte2short(ip_off);};
  byte ip_ttlGet() const {return ip_ttl;};
  byte ip_pGet() const {return ip_p;};
  uint16 ip_sumGet() const {return byte2short(ip_sum);};
  uint32 ip_srcGet() const {return byte2long(ip_src);};
  uint32 ip_dstGet() const {return byte2long(ip_dst);};

  // set information
  void ip_vSet(byte _val) {ip_v_hl = (ip_v_hl & 0x0f) | (_val << 4);};
  void ip_hlSet(byte _val) {ip_v_hl = (ip_v_hl & 0xf0) | (_val & 0x0f);};
  void ip_tosSet(byte _val) {ip_tos = _val;};
  void ip_lenSet(uint16 _val) {short2byte(_val,ip_len);};
  void ip_idSet(uint16 _val) {short2byte(_val,ip_id);};
  void ip_offSet(uint16 _val) {short2byte(_val,ip_off);};
  void ip_ttlSet(byte _val) {ip_ttl = _val;};
  void ip_pSet(byte _val) {ip_p = _val;};
  void ip_sumSet(uint16 _val) {short2byte(_val,ip_sum);};
  void ip_srcSet(uint32 _val) {long2byte(_val,ip_src);};
  void ip_srcSet(IPAddress _val) {long2byte(_val.Address(),ip_src);}
  void ip_dstSet(uint32 _val) {long2byte(_val,ip_dst);};
  void ip_dstSet(IPAddress _val) {long2byte(_val.Address(),ip_dst);};

  void ClearForCheckSum(uint16 _len);
  void MakeRespond();

  OSTREAM& Print(OSTREAM& os) const;

private:
  byte ip_v_hl;                 // 4-bit version | 4-bit header length
  byte ip_tos;                  // type of service
  byte ip_len[2];               // total length
  byte ip_id[2];                // identification
  byte ip_off[2];               // fragment offset field
#define IP_DF 0x4000            // dont fragment flag
#define IP_MF 0x2000            // more fragments flag
#define IP_OFFMASK 0x1fff       // mask for fragmenting bits
  byte ip_ttl;                  // time to live
  byte ip_p;                    // protocol
  byte ip_sum[2];               // checksum
  byte ip_src[4];               // source address
  byte ip_dst[4];               // destination address */
};

//
// inline definitions
//

// for checksumming of highel level protocols
// See TCP/IP illustrated volume 2 p 765, 766

inline void
IPHeader::ClearForCheckSum(uint16 _len)
{
  ip_v_hl = 0;
  ip_tos = 0;
  ip_len[0] = ip_len[1] = 0;
  ip_id[0] = ip_id[1] = 0;
  ip_off[0] = ip_off[1] = 0;
  ip_ttl = 0;
  short2byte(_len,ip_sum);    // the packet length goes here
}

inline void
IPHeader::MakeRespond()
{
  // swap source and destination address
  byte temp;
  temp = ip_dst[0]; ip_dst[0] = ip_src[0]; ip_src[0] = temp;
  temp = ip_dst[1]; ip_dst[1] = ip_src[1]; ip_src[1] = temp;
  temp = ip_dst[2]; ip_dst[2] = ip_src[2]; ip_src[2] = temp;
  temp = ip_dst[3]; ip_dst[3] = ip_src[3]; ip_src[3] = temp;
}

inline OSTREAM& operator <<(OSTREAM& os, const IPHeader& iph)
{
  return iph.Print(os);
}

#define IP_MAXPACKET    65535   /* maximum packet size */

// Definitions for IP type of service (ip_tos)

#define IPTOS_LOWDELAY      0x10
#define IPTOS_THROUGHPUT    0x08
#define IPTOS_RELIABILITY   0x04

// Internet implementation parameters.

#define MAXTTL      255     /* maximum time to live (seconds) */
#define IPDEFTTL    64      /* default ttl, from RFC 1340 */
#define IPFRAGTTL   60      /* time to live for frags, slowhz */
#define IPTTLDEC    1       /* subtracted when forwarding */

#define IP_MSS      576     /* default maximum segment size */

#endif

