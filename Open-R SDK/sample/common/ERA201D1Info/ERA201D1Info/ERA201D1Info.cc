//
// Copyright 2002,2003 Sony Corporation 
//
// Permission to use, copy, modify, and redistribute this software for
// non-commercial use is hereby granted.
//
// This software is provided "as is" without warranty of any kind,
// either expressed or implied, including but not limited to the
// implied warranties of fitness for a particular purpose.
//

#include <OPENR/OSyslog.h>
#include <ant.h>
#include "ERA201D1Info.h"
#include "entry.h"

ERA201D1Info::ERA201D1Info()
{
}

OStatus
ERA201D1Info::DoInit(const OSystemEvent& event)
{
    return oSUCCESS;
}

OStatus
ERA201D1Info::DoStart(const OSystemEvent& event)
{
    EtherStatus status;

    EtherDriverGetMACAddressMsg macAddrMsg;
    status = ERA201D1_GetMACAddress(&macAddrMsg);
    if (status == ETHER_OK) {
        PrintMACAddress(macAddrMsg);
    } else {
        OSYSLOG1((osyslogERROR, "ERA201D1_GetMACAddress() FAILED %d", status));
    }

    EtherDriverGetStatisticsMsg etherStatMsg;
    status = ERA201D1_GetEtherStatistics(&etherStatMsg);
    if (status == ETHER_OK) {
        PrintEtherStatistics(etherStatMsg);
    } else {
        OSYSLOG1((osyslogERROR,
                  "ERA201D1_GetEtherStatistics() FAILED %d", status));
    }

    EtherDriverGetWLANSettingsMsg wlanSettingsMsg;
    status = ERA201D1_GetWLANSettings(&wlanSettingsMsg);
    if (status == ETHER_OK) {
        PrintWLANSettings(wlanSettingsMsg);
    } else {
        OSYSLOG1((osyslogERROR,
                  "ERA201D1_GetWLANSettings() FAILED %d", status));
    }

    EtherDriverGetWLANStatisticsMsg wlanStatMsg;
    status = ERA201D1_GetWLANStatistics(&wlanStatMsg);
    if (status == ETHER_OK) {
        PrintWLANStatistics(wlanStatMsg);
    } else {
        OSYSLOG1((osyslogERROR,
                  "ERA201D1_GetWLANStatistics() FAILED %d", status));
    }

    antStackRef ipstackRef = antStackRef("IPStack");
    antEnvInitGetParamMsg getParamMsg("ETHER_IP");
    getParamMsg.Call(ipstackRef, sizeof(getParamMsg));
    if (getParamMsg.error == ANT_SUCCESS &&
        getParamMsg.paramType == antEnv_InitParam_String) {
        OSYSPRINT(("[ETHER_IP] %s\n", getParamMsg.value.str));
    } else {
        OSYSLOG1((osyslogERROR,
                  "getParamMsg.Call() FAILED %d", getParamMsg.error));
    }

    return oSUCCESS;
}    

OStatus
ERA201D1Info::DoStop(const OSystemEvent& event)
{
    return oSUCCESS;
}

OStatus
ERA201D1Info::DoDestroy(const OSystemEvent& event)
{
    return oSUCCESS;
}

void
ERA201D1Info::PrintMACAddress(const EtherDriverGetMACAddressMsg& msg)
{
    OSYSPRINT(("[MAC Address] "));
    OSYSPRINT(("%02x:%02x:%02x:%02x:%02x:%02x\n",
               msg.address.octet[0], msg.address.octet[1],
               msg.address.octet[2], msg.address.octet[3], 
               msg.address.octet[4], msg.address.octet[5]));
}

void
ERA201D1Info::PrintEtherStatistics(const EtherDriverGetStatisticsMsg& msg)
{
    OSYSPRINT(("[Ether Statistics]\n"));
    OSYSPRINT(("  carrierSenseErrorCount  : %d\n",
               msg.statistics.carrierSenseErrorCount));
    OSYSPRINT(("  lateCollisionCount      : %d\n",
               msg.statistics.lateCollisionCount));
    OSYSPRINT(("  excessiveCollisionCount : %d\n",
               msg.statistics.excessiveCollisionCount));
    OSYSPRINT(("  singleCollisionCount    : %d\n",
               msg.statistics.singleCollisionCount));
    OSYSPRINT(("  multipleCollisionCount  : %d\n",
               msg.statistics.multipleCollisionCount));
    OSYSPRINT(("  deferredCount           : %d\n",
               msg.statistics.deferredCount));
    OSYSPRINT(("  alignmentErrorCount     : %d\n",
               msg.statistics.alignmentErrorCount));
    OSYSPRINT(("  fcsErrorCount           : %d\n",
               msg.statistics.fcsErrorCount));
    OSYSPRINT(("  frameTooLongCount       : %d\n",
               msg.statistics.frameTooLongCount));
}

void
ERA201D1Info::PrintWLANSettings(const EtherDriverGetWLANSettingsMsg& msg)
{
    OSYSPRINT(("[WLAN Settings]\n"));
    OSYSPRINT(("  port          : %d\n", msg.port));
    OSYSPRINT(("  mode          : "));

    switch (msg.mode) {
    case ETHER_WLAN_MODE_AP:
        OSYSPRINT(("ETHER_WLAN_MODE_AP\n"));
        break;
    case ETHER_WLAN_MODE_ADHOC: 
        OSYSPRINT(("ETHER_WLAN_MODE_ADHOC\n"));
        break;
    default: 
        OSYSPRINT(("invalid value\n")); 
    }

    OSYSPRINT(("  essid         : %s\n", msg.essid));
    OSYSPRINT(("  channel       : %d\n", msg.channel));
    OSYSPRINT(("  encryption    : ", msg.encryption));

    switch (msg.encryption) {
    case ETHER_WLAN_ENC_OFF:
        OSYSPRINT(("ETHER_WLAN_ENC_OFF\n"));
        break;
    case ETHER_WLAN_ENC_WEP:
        OSYSPRINT(("ETHER_WLAN_ENC_WEP\n"));
        break;
    default:
        OSYSPRINT(("invalid value\n"));
    }

    OSYSPRINT(("  rate          : "));

    switch (msg.rate) {
    case ETHER_WLAN_RATE_1MB:
        OSYSPRINT(("ETHER_WLAN_RATE_1MB\n"));
        break;
    case ETHER_WLAN_RATE_2MB:
        OSYSPRINT(("ETHER_WLAN_RATE_2MB\n"));
        break;
    case ETHER_WLAN_RATE_5_5MB: 
        OSYSPRINT(("ETHER_WLAN_RATE_5_5MB\n"));
        break;
    case ETHER_WLAN_RATE_11MB:
        OSYSPRINT(("ETHER_WLAN_RATE_11MB\n"));
        break;
    default:
        OSYSPRINT(("invalid value\n"));
    }

    OSYSPRINT(("  rts           : %d\n", msg.rts));
    OSYSPRINT(("  fragmentation : %d\n", msg.fragmentation));
    OSYSPRINT(("  sensitivity   : "));

    switch (msg.sensitivity) {
    case ETHER_WLAN_DENSITY_LOW:
        OSYSPRINT(("ETHER_WLAN_DENSITY_LOW\n"));
        break;
    case ETHER_WLAN_DENSITY_MED: 
        OSYSPRINT(("ETHER_WLAN_DENSITY_MED\n"));
        break;
    case ETHER_WLAN_DENSITY_HIGH:
        OSYSPRINT(("ETHER_WLAN_DENSITY_HIGH\n"));
        break;
    default:
        OSYSPRINT(("invalid value\n"));
    }
}

void
ERA201D1Info::PrintWLANStatistics(const EtherDriverGetWLANStatisticsMsg& msg)
{
    OSYSPRINT(("[WLAN Statistics]\n"));
    OSYSPRINT(("  link             : %d\n", msg.statistics.link));
    OSYSPRINT(("  signal           : %d\n", msg.statistics.signal));
    OSYSPRINT(("  noise            : %d\n", msg.statistics.noise));
    OSYSPRINT(("  invalidIDCount   : %d\n", msg.statistics.invalidIDCount));
    OSYSPRINT(("  invalidEncCount  : %d\n", msg.statistics.invalidEncCount));
    OSYSPRINT(("  invalidMiscCount : %d\n", msg.statistics.invalidMiscCount));
}
