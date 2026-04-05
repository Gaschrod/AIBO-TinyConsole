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


#ifndef WLANDriverMsg_h_DEFINED
#define WLANDriverMsg_h_DEFINED

#include <Types.h>
#include <EtherTypes.h>


/*
  WLAN modes
*/

enum EtherWLANMode
{
  ETHER_WLAN_MODE_ADHOC = 0,
  ETHER_WLAN_MODE_AP
};

/*
  WLAN encryption
*/

enum EtherWLANEncryption
{
  ETHER_WLAN_ENC_OFF = 0,
  ETHER_WLAN_ENC_WEP
};

/*
  WLAN rate
*/

enum EtherWLANRate
{
  ETHER_WLAN_RATE_1MB = 0,
  ETHER_WLAN_RATE_2MB,
  ETHER_WLAN_RATE_5_5MB,
  ETHER_WLAN_RATE_11MB,
  ETHER_WLAN_RATE_AUTO
};

/*
  WLAN sensitivity
*/

enum EtherWLANSensitivity
{
  ETHER_WLAN_DENSITY_LOW = 0,
  ETHER_WLAN_DENSITY_MED,
  ETHER_WLAN_DENSITY_HIGH
};


/*
  WLAN statistics
*/
struct EtherWLANStatistics
{
  uint8  link;   // in percentages of max value 
  uint8  signal; // in percentages of max value 
  uint8  noise;  // in percentages of max value 
  uint32 invalidIDCount;
  uint32 invalidEncCount;
  uint32 invalidMiscCount;
};


/*
  WLAN constants 
*/

#define ETHER_WLAN_ESSID_MAX            32 
#define ETHER_WLAN_STATIONNAME_MAX      32
#define ETHER_WLAN_ENC_MAX              5
#define ETHER_WLAN_ENC_128_MAX          13


/*
 API extensions for WLAN driver
*/

struct EtherDriverGetWLANSettingsMsg
{
  uint32                     port;
  EtherWLANMode              mode;
  char                       essid[ETHER_WLAN_ESSID_MAX+1]; // add 1 byte for terminating '\0'.
  char                       stationName[ETHER_WLAN_STATIONNAME_MAX+1]; // add 1 byte for terminating '\0'.
  uint32                     channel;
  EtherWLANEncryption        encryption;
  EtherWLANRate              rate;
  uint32                     rts;
  uint32                     fragmentation;
  EtherWLANSensitivity       sensitivity;
  EtherStatus                status;
};


struct EtherDriverSetWLANModeMsg
{
  uint32          port;
  EtherWLANMode   mode;
  EtherStatus     status;
};


struct EtherDriverSetWLANESSIDMsg
{
  uint32       port;
  char         essid[ETHER_WLAN_ESSID_MAX];
  uint32       length;
  EtherStatus  status;
};

struct EtherDriverSetWLANStationNameMsg
{
  uint32        port;
  char          stationName[ETHER_WLAN_STATIONNAME_MAX];
  uint32        length;
  EtherStatus   status;
};

struct EtherDriverSetWLANChannelMsg
{
  uint32          port;
  uint32          channel;
  EtherStatus     status;
};

struct EtherDriverSetWLANEncryptionMsg
{
  uint32                   port;
  EtherWLANEncryption      type;
  char                     key[ETHER_WLAN_ENC_MAX];
  EtherStatus              status;
};

struct EtherDriverSetWLANEncryption128Msg
{
  uint32                   port;
  EtherWLANEncryption      type;
  char                     key[ETHER_WLAN_ENC_128_MAX];
  EtherStatus              status;
};

struct EtherDriverSetWLANRateMsg
{
  uint32           port;
  EtherWLANRate    rate;
  EtherStatus      status;
};

struct EtherDriverSetWLANMediumReservationMsg
{
  uint32      port;
  uint32      threshold;
  EtherStatus status;
};

struct EtherDriverSetWLANFragmentationMsg
{
  uint32      port;
  uint32      threshold;
  EtherStatus status;
};

struct EtherDriverSetWLANSensitivityMsg
{
  uint32               port;
  EtherWLANSensitivity sensitivity;
  EtherStatus          status;
};

struct EtherDriverGetWLANStatisticsMsg
{
  uint32              port;
  EtherWLANStatistics statistics;
  EtherStatus         status;
};


#endif /* WLANDriverMsg_h_DEFINED */
