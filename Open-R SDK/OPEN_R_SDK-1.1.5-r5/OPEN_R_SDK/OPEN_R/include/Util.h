/*
 *  Copyright 2002 Sony Corporation
 */
#ifndef _Util_h_DEFINED_
#define _Util_h_DEFINED_

#include <ant.h>

#ifdef ANT_INTERNAL

//
// checksum routine for an SDU
//

uint16 SDUCheckSum(antSDUPtr& _sdu, uint32 _off, uint32 _len);

//
// get number of complete bytes in an SDU
//

uint32 SDUComplete(antSDUPtr& _sdu);

//
// dump sdu contents
//

void SDUDump(antSDUPtr& _sdu);
void SDUDumpContent(antSDUPtr& _sdu);

#endif

//
// min and max
//

#ifndef MIN
#define MIN(x,y) ((x) < (y) ? (x) : (y))
#endif

#ifndef MAX
#define MAX(x,y) ((x) > (y) ? (x) : (y))
#endif

#ifndef min
#define min(x,y) ((x) < (y) ? (x) : (y))
#endif

#ifndef max
#define max(x,y) ((x) > (y) ? (x) : (y))
#endif

//
// abs
//

#ifndef ABS
#define ABS(x) ((x) < 0 ? -(x) : (x))
#endif

#if 0
#ifndef abs
#define abs(x) ((x) < 0 ? -(x) : (x))
#endif
#endif
//
// uint16, uint32 to and from byte array
// the byte array will be arrange in network byte order
// 

inline
uint32
byte2long(const byte* _b)
{
  return (_b[0] << 24 | _b[1] << 16 | _b[2] << 8 | _b[3]);
}

inline
uint16
byte2short(const byte* _b)
{
  return (_b[0] << 8 | _b[1]);
}

inline
void
long2byte(uint32 _l, byte* _b)
{
  _b[0] = (_l >> 24) & 0xff;
  _b[1] = (_l >> 16) & 0xff;
  _b[2] = (_l >> 8) & 0xff;
  _b[3] = _l & 0xff;
}

inline
void
short2byte(uint16 _s, byte* _b)
{
  _b[0] = (_s >> 8) & 0xff;
  _b[1] = _s & 0xff;
}

#endif
