#ifndef _ICMPProtocol_h_DEFINED_
#define _ICMPProtocol_h_DEFINED_

#include <ant.h>
#include <Util.h>
#include <IPv4Types.h>
#include <antIOStream.h>

//
// definitiions for tye and code fileds
//

#define ICMP_TYPE_ECHOREPLY    0                // echo reply
#define ICMP_TYPE_UNREACH    3                  // dest unreachable, codes:
#define   ICMP_CODE_UNREACH_NET  0              //      bad net
#define   ICMP_CODE_UNREACH_HOST 1              //      bad host
#define   ICMP_CODE_UNREACH_PROTOCOL 2          //      bad protocol
#define   ICMP_CODE_UNREACH_PORT 3              //      bad port
#define   ICMP_CODE_UNREACH_NEEDFRAG 4          //      IP_DF caused drop
#define   ICMP_CODE_UNREACH_SRCFAIL  5          //      src route failed
#define   ICMP_CODE_UNREACH_NET_UNKNOWN 6       //      unknown net
#define   ICMP_CODE_UNREACH_HOST_UNKNOWN 7      //      unknown host
#define   ICMP_CODE_UNREACH_ISOLATED 8          //      src host isolated
#define   ICMP_CODE_UNREACH_NET_PROHIB 9        //      prohibited access
#define   ICMP_CODE_UNREACH_HOST_PROHIB 10      //      ditto
#define   ICMP_CODE_UNREACH_TOSNET 11           //      bad tos for net
#define   ICMP_CODE_UNREACH_TOSHOST  12         //      bad tos for host
#define ICMP_TYPE_SOURCEQUENCH 4                // packet lost, slow down
#define ICMP_TYPE_REDIRECT   5                  // shorter route, codes:
#define   ICMP_CODE_REDIRECT_NET 0              //      for network
#define   ICMP_CODE_REDIRECT_HOST  1            //      for host
#define   ICMP_CODE_REDIRECT_TOSNET  2          //      for tos and net
#define   ICMP_CODE_REDIRECT_TOSHOST 3          //      for tos and host
#define ICMP_TYPE_ECHO   8                      // echo service
#define ICMP_TYPE_ROUTERADVERT 9                // router advertisement
#define ICMP_TYPE_ROUTERSOLICIT  10             // router solicitation
#define ICMP_TYPE_TIMXCEED   11                 // time exceeded, code:
#define   ICMP_CODE_TIMXCEED_INTRANS 0          //      ttl==0 in transit
#define   ICMP_CODE_TIMXCEED_REASS 1            //      ttl==0 in reass
#define ICMP_TYPE_PARAMPROB    12               // ip header bad
#define   ICMP_CODE_PARAMPROB_OPTABSENT 1       //      req. opt. absent
#define ICMP_TYPE_TSTAMP   13                   // timestamp request
#define ICMP_TYPE_TSTAMPREPLY  14               // timestamp reply
#define ICMP_TYPE_IREQ   15                     // information request
#define ICMP_TYPE_IREQREPLY    16               // information reply
#define ICMP_TYPE_MASKREQ    17                 // address mask request
#define ICMP_TYPE_MASKREPLY    18               // address mask reply

#define ICMP_TYPE_MAX    18

//
// header
//

struct ICMPHeader
{
public:
  byte icmp_typeGet() const {return icmp_type;};
  byte icmp_codeGet() const {return icmp_code;};
  uint16 icmp_cksumGet() const {return byte2short(icmp_cksum);};

  void icmp_typeSet(byte _val) {icmp_type = _val;};
  void icmp_codeSet(byte _val) {icmp_code = _val;};
  void icmp_cksumSet(uint16 _val) {short2byte(_val,icmp_cksum);};

  OSTREAM& Print(OSTREAM& os) const;

private:
  byte icmp_type;       // type of message, see below
  byte icmp_code;       // type sub code
  byte icmp_cksum[2];   // ones complement cksum of struct
};

//
// Further contents, depending on type and code
//

struct ICMPEcho: public ICMPHeader
{
public:
  byte icmp_idGet() const {return byte2short(icmp_id);};
  byte icmp_seqGet() const {return byte2short(icmp_seq);};

  void icmp_idSet(uint16 _val) {short2byte(_val, icmp_id);};
  void icmp_seqSet(uint16 _val) {short2byte(_val, icmp_seq);};

  OSTREAM& Print(OSTREAM& os) const;

private:
  byte icmp_id[2];
  byte icmp_seq[2];
};

struct ICMPError: public ICMPHeader
{
public:
  uint16 icmp_nextmtuGet() const {return byte2short(icmp_void + 2);};
  byte icmp_pptrGet() const {return icmp_void[0];};

  void icmp_nextmtuSet(uint16 _val) {icmp_void[0] = icmp_void[1] = 0; short2byte(_val, icmp_void + 2);};
  void icmp_pptrSet(byte _val) {icmp_void[0] = _val; icmp_void[1] = icmp_void[2] = icmp_void[3] = 0;};

  OSTREAM& Print(OSTREAM& os) const;

private:
  byte icmp_void[4];
};

//
// inline functions
//


inline
OSTREAM&
operator <<(OSTREAM& os, const ICMPHeader& icmph)
{   
  return icmph.Print(os); 
}   

inline
OSTREAM&
operator <<(OSTREAM& os, const ICMPEcho& icmph)
{   
  return icmph.Print(os); 
}   

inline
OSTREAM&
operator <<(OSTREAM& os, const ICMPError& icmph)
{   
  return icmph.Print(os); 
}   

#endif // _ICMPProtocol_h_DEFINED_
