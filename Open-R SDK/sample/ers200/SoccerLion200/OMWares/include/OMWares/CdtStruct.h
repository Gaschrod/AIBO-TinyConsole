//
// Copyright 1999,2002 Sony Corporation 
//
// Permission to use, copy, modify, and redistribute this software for
// non-commercial use is hereby granted.
//
// This software is provided "as is" without warranty of any kind,
// either expressed or implied, including but not limited to the
// implied warranties of fitness for a particular purpose.
//

#ifndef _CdtStruct_h_DEFINED
#define _CdtStruct_h_DEFINED

#include <OPENR/ObjcommMessages.h>
#include <OPENR/ObjcommTypes.h>

const longword  SETTABLE           = 0;
const longword  CLEARTABLE         = 1;
const longword  SETPARTICULARTABLE = 2;
const longword  GETPROPORTION      = 3;
const longword  GETPOSITION        = 4;

const longword TABLE_OFF = 0x80808080;

struct CdtCntrl {
    static const int MaxResolution = 32;
    ObserverID requester;
    longword   command;
    longword   channel;
    longword   table[MaxResolution];
    CdtCntrl(){
        requester.oid = 0;
        requester.selector = 0;
        command   = GETPOSITION;
        channel   = 0;
        int i;
        for (i=0;i<MaxResolution;i++) table[i] = TABLE_OFF;
    }
};

const longword SUCCEEDINSETTING = 0;
const longword CHANNELINUSE     = 1;
const longword COMMANDERROR     = 2;
const longword UNKNOWNCHANNEL   = 3;
const longword PROPORTION       = 4;
const longword POSITION         = 5;
const longword INVALIDDATA      = 6;
const longword NOTENOUGHAREA    = 7;

struct CdtResults {
    ObserverID requester;
    longword   command;
    longword   status;
    longword   timestamp;
    longword   fieldnum;
    longword   npixel;
    longword   position_x,position_y;
    longword   size_x,size_y;
    longword   reserve0;
    longword   reserve1;
    longword   reserve2;
    longword   reserve3;
    CdtResults(){
        requester.oid = 0;
        requester.selector = 0;
        command    = GETPOSITION;
        status     = INVALIDDATA;
        timestamp  = 0;
        fieldnum   = 0;
        npixel     = 0;
        position_x = 0;
        position_y = 0;
        size_x     = 0;
        size_y     = 0;
        reserve0   = 0;
        reserve1   = 0;
        reserve2   = 0;
        reserve3   = 0;
    }
    CdtResults( ObserverID id, longword cmd, longword stat ) {
        requester  = id;
        command    = cmd;
        status     = stat;
        timestamp  = 0;
        fieldnum   = 0;
        npixel     = 0;
        position_x = 0;
        position_y = 0;
        size_x     = 0;
        size_y     = 0;
        reserve0   = 0;
        reserve1   = 0;
        reserve2   = 0;
        reserve3   = 0;
    }
};

#endif  /* _CdtStruct_h_DEFINED */
