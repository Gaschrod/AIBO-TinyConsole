/*
 * ERA201D1.h
 *
 * Copyright 2002 Sony Corporation
 */

#ifndef _EAR201D1_h_DEFINED
#define _EAR201D1_h_DEFINED

#include <EtherDriverMsg.h>
#include <WLANDriverMsg.h>

#ifdef __cplusplus
extern "C" {
#endif
    EtherStatus ERA201D1_GetMACAddress(EtherDriverGetMACAddressMsg* msg);
    EtherStatus ERA201D1_GetEtherStatistics(EtherDriverGetStatisticsMsg* msg);
    EtherStatus ERA201D1_GetWLANSettings(EtherDriverGetWLANSettingsMsg* msg);
    EtherStatus ERA201D1_GetWLANStatistics(EtherDriverGetWLANStatisticsMsg* msg);
#ifdef __cplusplus
}
#endif

#endif /* _EAR201D1_h_DEFINED */
