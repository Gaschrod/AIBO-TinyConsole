//
//  ObjcommMessasges.h
//
//  Copyright (C) 1999-2002 Sony Corporation
//  All Rights Reserved.
//

#ifndef _ObjcommMessages_h_DEFINED
#define _ObjcommMessages_h_DEFINED

#include <Types.h>
#include <OPENR/OPENR.h>
#include <OPENR/OPENRMessages.h>
#include <OPENR/OMemoryLocator.h>
#include <OPENR/ObjcommTypes.h>
#include "SharedMemoryAllocator.h"
#include "RCRegion.h"

//
//   OPENR Message Type
//
//const OMessageType omsgCONNECT         = OMessageSystemBase +11;
const OMessageType omsgCONTROL           = OMessageSystemBase +12;
const OMessageType omsgCONTROL_REPLY     = OMessageSystemBase +13;
const OMessageType omsgREADY             = OMessageSystemBase +14;
const OMessageType omsgNOTIFY            = OMessageSystemBase +15;

//   Message Types of Ver. 2.1
const OMessageType omsgCONTROL_V21       = OMessageSystemBase +256;
const OMessageType omsgCONTROL_REPLY_V21 = OMessageSystemBase +257;
const OMessageType omsgREADY_V21         = OMessageSystemBase +258;
const OMessageType omsgNOTIFY_V21        = OMessageSystemBase +259;
//   Legacy support of SetData(MemID, offset) in ver. 1.0
const OMessageType omsgNOTIFY_L10        = OMessageSystemBase +260;
//   Message Type of Ver. 2.2
const OMessageType omsgREADY_V22         = OMessageSystemBase +261;
const OMessageType omsgNOTIFY_V22        = OMessageSystemBase +262;


struct OControlMessage {
    OMessageType     msgType;
    ObjcommCommand   command;
    ObserverID       observerID;
    size_t           d_skip;
    size_t           d_min;
    size_t           d_max;
    char             subName[oserviceNAME_MAX + 1];

    OControlMessage()
    : msgType(omsgCONTROL), command(objcommCMD_UNDEF), observerID(),
          d_skip(0), d_min(1), d_max(1) {}
    OControlMessage(ObjcommCommand c, ObserverID o, size_t skip, size_t min,
            size_t max, const char* s )
    : msgType(omsgCONTROL), command(c), observerID(o),
          d_skip(skip), d_min(min), d_max(max)
    { strcpy(subName, s); }
};

struct OControlReplyMessage {
    OMessageType    msgType;
    OStatus         status;
    SubjectID       subjectID;
    MemoryRegionID  memID;
    OControlReplyMessage()
    : msgType(omsgCONTROL_REPLY),status(oUNDEF),subjectID(),memID(0)
    {}
    OControlReplyMessage( OStatus s, SubjectID r, MemoryRegionID id )
    : msgType(omsgCONTROL_REPLY),status( s ),subjectID( r ),memID( id )
    {}
};


//
//  Maximun number of data which can be sent at once by Inter-Object
//  Communication
//
#define MAX_NUM_DATA  13

struct OReadyMessage {
    OMessageType          msgType;
    ObjcommCommand        command;
    ObserverID            observerID;
    int                   numOfData;
    RCRegion*             rcRegion[MAX_NUM_DATA];

    OReadyMessage()
    : msgType(omsgREADY_V22),
          command(objcommCOMMAND_UNDEF),
          observerID(),
          numOfData(0)
      {}
};


struct TransferedData {
    RCRegion*      srcRCR;
    MemoryRegionID memID;
    unsigned int   offset;
    size_t         size;
};

//
//  sizeof(ONotifyMessage) should not be greater than 236,
//  which is an requirement from the USS-USM Inter CPU Connection.
//
struct ONotifyMessage {                // total 224 bytes 
                                       //                 
    OMessageType   msgType;            //         4 bytes
    SubjectID      subjectID;          //         8 bytes 
    short          numOfData;          //         2 bytes
    short          numOfNotify;        //         2 bytes
    TransferedData data[MAX_NUM_DATA]; // 16*13=208 bytes

    ONotifyMessage()
    : msgType(omsgNOTIFY_V22), subjectID(), numOfData(0), numOfNotify(0) {}
    ~ONotifyMessage() {}

    OStatus  Write(RCRegion* rc);
    void     Clear() { numOfData = 0; numOfNotify = 0; }
    size_t   GetSizeOfMessage() const;
    int      NumberOfData()   const { return numOfData; }
    int      NumberOfNotify() const { return numOfNotify; }
};

/*
 *   OReadyMessage for version 1.0
 */
#include <OPENR/OMemoryLocator.h>

/*  ReadyMessage  */
const int objcommMAX_NUM_UNLOCK = 29;

struct OReadyMessageV10 {
  OMessageType          msgType;
  ObjcommCommand        command;
  ObserverID            observerID;
  int                   numOfUnlock;
  OMemoryLocator        unlock[objcommMAX_NUM_UNLOCK];
  
  OReadyMessageV10() 
    : msgType(omsgREADY),
      command(objcommCOMMAND_UNDEF), 
      observerID(), 
      numOfUnlock(0)
  {
      for (int i=0; i<objcommMAX_NUM_UNLOCK; i++){
          unlock[i].MakeInvalid();
      }
  }
};


//
//  ONotifyMessage for version 1.0 
//
#define MAX_MESSAGE_SIZE  256
#define HEADER_SIZE       (sizeof(OMessageType) + sizeof(SubjectID) + sizeof(int) + sizeof(size_t))

struct OChunkSHM {
    ObjcommDataType      type;
    OMemoryLocator       locator;
};

struct OChunkMSG {
    ObjcommDataType      type;
    size_t               size;
    char                 data;
};

struct ONotifyMessageV10 {
    
    ObjcommMessageType    msgType;
    SubjectID             subjectID;
    int                   numOfData;
    size_t                dataSize;
    char                  data[MAX_MESSAGE_SIZE - HEADER_SIZE];
    
    ONotifyMessageV10() 
        : msgType(omsgNOTIFY), subjectID(), numOfData(0), dataSize(0) {}
    ~ONotifyMessageV10() {}
        
    OStatus   SetData(void* data, size_t size);
    OStatus   SetData(const OMemoryLocator& l);
    void      ClearData();
    size_t    GetSizeOfMessage() const;
    int       GetNumberOfData() const;
    void*     GetDataAddress() const;
};

#endif // _ObjcommMessages_h_DEFINED
