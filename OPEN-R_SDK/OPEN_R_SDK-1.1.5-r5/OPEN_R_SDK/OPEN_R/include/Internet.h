/*
 *  Copyright 2002 Sony Corporation
 */
#ifndef Internet_h_DEFINED
#define Internet_h_DEFINED

// Protocols

#define IPPROTO_IP      0       /* dummy for IP */
#define IPPROTO_ICMP    1       /* control message protocol */
#define IPPROTO_IGMP    2       /* group mgmt protocol */
#define IPPROTO_GGP     3       /* gateway^2 (deprecated) */
#define IPPROTO_TCP     6       /* tcp */
#define IPPROTO_EGP     8       /* exterior gateway protocol */
#define IPPROTO_PUP     12      /* pup */
#define IPPROTO_UDP     17      /* user datagram protocol */
#define IPPROTO_IDP     22      /* xns idp */
#define IPPROTO_TP      29      /* tp-4 w/ class negotiation */
#define IPPROTO_EON     80      /* ISO cnlp */
#define IPPROTO_ENCAP   98      /* encapsulation header */

#define IPPROTO_RAW     255     /* raw IP packet */

#define IPPROTO_MAX     256

typedef uint16 Protocol;

// Local port number conventions:
// Ports < IPPORT_RESERVED are reserved for
// privileged processes (e.g. root).
// Ports > IPPORT_USERRESERVED are reserved
// for servers, not necessarily privileged.

#define IPPORT_RESERVED     1024
#define IPPORT_USERRESERVED 5000

#endif
