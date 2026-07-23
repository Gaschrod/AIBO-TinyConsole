/*
 * The Aperios Operating System
 *
 * This program is subject to copyright protection in accordance with the
 * applicable law. It must not, except where allowed by law, by any means or
 * in any form be reproduced, distributed or lent. Moreover, no part of the
 * program may be used, viewed, printed, disassembled or otherwise interfered
 * with in any form, except where allowed by law, without the express written
 * consent of the copyright holder.
 *
 * Copyright 1988-2002 Sony Corporation
 * All Rights Reserved.
 *
 */
/*
 * $Date: Fri Aug 14 22:02:12 JST 1998 $
 * $Revision: /main/tokyo/9 $
 */

#ifndef EtherDriverMsg_h_DEFINED
#define EtherDriverMsg_h_DEFINED

#include <Types.h>
#include <EtherTypes.h>

//
// info about the ether device
//
struct EtherDeviceInfo
{
  uint32 nrPorts;
  uint32 capabilities;
};

// capabilities
#define ETHER_CAP_MODE_AUTONEGOTIATION 1


//
// etherdevice mode
//

enum EtherDeviceMode
{
  ETHER_MODE_10BASET = 0,
  ETHER_MODE_10BASET_FULLDUPLEX,
  ETHER_MODE_100BASETX,
  ETHER_MODE_100BASETX_FULLDUPLEX,
  ETHER_MODE_100BASET4,
  ETHER_MODE_AUTONEGOTIATION
};

//
// ether statistics
//
struct EtherStatistics
{
  uint32 carrierSenseErrorCount;
  uint32 lateCollisionCount;
  uint32 excessiveCollisionCount;
  uint32 singleCollisionCount;
  uint32 multipleCollisionCount;
  uint32 deferredCount;
  uint32 alignmentErrorCount;
  uint32 fcsErrorCount;
  uint32 frameTooLongCount;
};

//
// ether client handle
//

typedef uint32 EtherClientHandle;

//
// ether promiscous level
//
enum EtherPromiscLevel
{
  ETHER_PROMISC_PHYS  = 0x01,
  ETHER_PROMISC_RAW   = 0x02,
  ETHER_PROMISC_MULTI = 0x04
};

//
// TxmitNotification flag 
//
enum EtherTxmitNotificationFlag
{
  ETHER_NOTIFY_NEVER = 0x01,
  ETHER_NOTIFY_ONCE  = 0x02,
  ETHER_NOTIFY_FREED = 0x04
};

//
// notification messages
//
struct EtherDriverTxmitNotificationMsg
{
  uint32             frameCount;
  EtherClientHandle  client;
  void*              cookie;
  EtherStatus        status;
};

struct EtherDriverRcvNotificationMsg
{
  uint32             frameCount;
  EtherClientHandle  client;
  void*              cookie;
};


//
// API of etherdriver
//
struct EtherDriverGetDeviceInfoMsg
{
  EtherDeviceInfo  deviceInfo;
  EtherStatus      status;
};


struct EtherDriverGetMACAddressMsg
{
  uint32          port;
  EtherMACAddress address;
  EtherStatus     status;
};


struct EtherDriverSetMACAddressMsg
{
  uint32          port;
  EtherMACAddress address;
  EtherStatus     status;
};

struct EtherDriverGetDeviceModeMsg
{
  uint32          port;
  EtherDeviceMode mode;
  EtherStatus     status;
};

struct EtherDriverSetDeviceModeMsg
{
  uint32          port;
  EtherDeviceMode mode;
  EtherStatus     status;
};

struct EtherDriverGetStatisticsMsg
{
  uint32           port;
  EtherStatistics  statistics;
  EtherStatus      status;
};

struct EtherDriverRegisterClientMsg
{
  uint32   port;
  OID      client;
  Selector txmitNotification;
  Selector rcvNotification;
  void*    cookie;
  size_t   txmitBufSize;
  size_t   rcvBufSize;

  EtherClientHandle  clientHandle;
  EtherFrameBufferID bufferID;
  EtherStatus status;
};


struct EtherDriverDeregisterClientMsg
{
  EtherClientHandle client;
  EtherStatus       status;
};

struct EtherDriverTransmitMsg
{
  EtherClientHandle          client;
  uint32                     nrFrames;
  EtherTxmitNotificationFlag flag;
};

struct EtherDriverStartReceiveMsg
{
  EtherClientHandle  client;
  EtherProtocolType  proto;
  EtherStatus        status;
};

struct EtherDriverStopReceiveMsg
{
  EtherClientHandle  client;
  EtherProtocolType  proto;
  EtherStatus        status;
};

struct EtherDriverStartReceivePromiscMsg
{
  EtherClientHandle   client;
  EtherPromiscLevel   level;
  EtherStatus         status;
};


struct EtherDriverStopReceivePromiscMsg
{
  EtherClientHandle client;
  EtherPromiscLevel level;
  EtherProtocolType proto;
  EtherStatus       status;
};

struct EtherDriverAddAddressMsg
{
  EtherClientHandle  client;
  EtherMACAddress    address;
  EtherStatus        status;
};

struct EtherDriverRemoveAddressMsg
{
  EtherClientHandle client;
  EtherMACAddress   address;
  EtherStatus       status;
};


/*

class FrameBuffer;

//
// initialization
//

struct EtherDriverInitResultMsg 
{
    sError status;

    EtherDriverInitResultMsg () {}
    EtherDriverInitResultMsg (sError st) { status = st; }
};

//
// opening/closing
//

struct EtherDriverOpenMsg 
{
    word      proto;

    OID       client;
    Selector  input;

    EtherDriverOpenMsg () {}
    EtherDriverOpenMsg (word pr, OID self, Selector sel)
        : proto (pr), client (self), input (sel) {}
};

struct EtherDriverOpenResultMsg 
{
  EtherStatus  status;
  FrameBuffer* txmit_buf;
  EtherAddr    addr;
  word         proto;

  EtherDriverOpenResultMsg () {}

  EtherDriverOpenResultMsg (EtherStatus _result, EtherAddr _addr, FrameBuffer* _txmit_buf, word _proto) 
        : addr(_addr), proto(_proto), txmit_buf(_txmit_buf), status (_result) {}

};


struct EtherDriverCloseMsg 
{
  word proto; 

  EtherDriverCloseMsg () {}
  EtherDriverCloseMsg (word _proto) : proto(_proto) {}
};

struct EtherDriverCloseResultMsg 
{
  EtherStatus  status;

  EtherDriverCloseResultMsg () {}
  EtherDriverCloseResultMsg (EtherStatus _status) : status(_status) {}
};




struct EtherDriverToUpperMsg 
{
    word         proto;
    FrameBuffer* txmit_buf;
    uint32       frame_count;

    EtherDriverToUpperMsg () {}
    EtherDriverToUpperMsg (word pr, FrameBuffer* _buf, uint32 count) 
        : proto (pr), txmit_buf(_buf), frame_count(count) {}
};

struct EtherDriverToLowerMsg 
{
    FrameBuffer*  buffer;
    uint32        frame_count;

    EtherDriverToLowerMsg () {}
    EtherDriverToLowerMsg (FrameBuffer* buf, uint32 count) 
        : buffer(buf), frame_count(count) {}
};

struct EtherDriverToLowerResultMsg 
{
    EtherStatus  status;
    uint32       frame_count;
    FrameBuffer* txmit_buf;

    EtherDriverToLowerResultMsg () {}
    EtherDriverToLowerResultMsg (EtherStatus result, uint32 _count, FrameBuffer* _txmit_fb) : 
        status (result), frame_count(_count), txmit_buf(_txmit_fb) {}
};
//
// enabling/disabling reception of multicast frames
//

struct EtherDriverAddAddressMsg
{
  EtherAddr addr;
  
  EtherDriverAddAddressMsg() {}
  EtherDriverAddAddressMsg(EtherAddr _addr) : addr(_addr) {}

};

struct EtherDriverAddAddressResultMsg
{
  EtherStatus status;

  EtherDriverAddAddressResultMsg(EtherStatus _status) : status(_status) {}
};

struct EtherDriverRemoveAddressMsg
{
  EtherAddr addr;

  EtherDriverRemoveAddressMsg() {}
  EtherDriverRemoveAddressMsg(EtherAddr _addr) : addr(_addr) {}
};

struct EtherDriverRemoveAddressResultMsg
{
  EtherStatus status;

  EtherDriverRemoveAddressResultMsg(EtherStatus _status) : status(_status) {}
};


//
// SNMP-like support
//

struct EtherDriverGetStatsResultMsg
{
  EtherDriverStatistics stats;
};

*/









#endif /* EtherDriverMsg_h_DEFINED */
