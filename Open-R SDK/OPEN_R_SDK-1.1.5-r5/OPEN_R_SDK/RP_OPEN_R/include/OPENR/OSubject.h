//
// OSubject.h
//
// Copyright 1999,2000,2001,2002 Sony Corporation
//

#ifndef _OSubject_h_DEFINED
#define _OSubject_h_DEFINED

#include <Types.h>
#include <list>
using std::list;
#include <OPENR/OPENR.h>
#include <OPENR/ObjcommTypes.h>
#include <OPENR/ObjcommMessages.h>
#include <OPENR/OObserverInfo.h>
#include <OPENR/OShmPtr.h>

//#define SUPPORT_OBJCOMM_VERSION_10
#ifdef SUPPORT_OBJCOMM_VERSION_10
#if defined(__GNUC__)
#include <map>
using namespace std;
#else
#include <stl/map.h>
#endif
#endif

typedef list<OObserverInfo>::iterator         ObserverIterator;
typedef list<OObserverInfo>::const_iterator   ObserverConstIterator;

//
//  Data transmitter class in OPEN-R Object
//
class OSubject {
    static const size_t DEFAULT_MAX_BUFFER_SIZE = MAX_NUM_DATA+1;

  protected:
    SubjectID           myID_;           // ID of self
    list<OObserverInfo>  observers_;      // list of observers' attributes
    ONotifyMessage      notifyMsg;       // Message area;
    size_t              maxBufferSize_;  // Maximum size of buffer
    size_t              notifyUnitSize_; // Number of SetData() for each notify.

    /* 
     *  SUPPORT V10 COMMON MODE 
     */
    list<RCRegion*>            sentRegions_;    // RCRegion sent to Ver.1.0 Observers
    list<RCRegion*>::iterator  FindRegion(MemoryRegionID id, offset_t offset);
    ONotifyMessageV10          notifyMsgV10_;

#ifdef SUPPORT_OBJCOMM_VERSION_10
    //  Legacy support for SetData(MemID, Offset)
    static map<OMemoryLocator, int, less<OMemoryLocator> > *pRefCounters_;
#endif // SUPPORT_OBJCOMM_VERSION_10
    /*
     *  For internal use
     */
    OStatus  AddObserver_V22(const OControlMessage& msg, OStatus status);
    OStatus  AddObserver_V10(const OControlMessage& msg, OStatus status);
    OStatus  RemoveObserver (const OControlMessage& msg, OStatus status);
    ObserverIterator      findObserver(const ObserverID& id);
    ObserverConstIterator findObserver(const ObserverID& id) const;
    OStatus  ReadyHandler_V22  (const OReadyMessage& msg);
    void     UnlockReturnedData(const OReadyMessage& msg);
    OStatus  ReadyHandler_V10  (const OReadyMessageV10& msg);
    void     UnlockReturnedData(const OReadyMessageV10& msg);
    OStatus  SendNotify(OObserverInfo&);
    OStatus  SendNotifyV22(OObserverInfo&);
    OStatus  SendNotifyV10(OObserverInfo&);

  private:
    //   No copy of instance is allowed !
    OSubject(const OSubject& subject);
    OSubject& operator=(const OSubject& subject);

  protected:
    const SubjectID&  SetID(const SubjectID& entry) { return myID_=entry; }

  public:
    OSubject(void);
    ~OSubject();

    /*
     *   Functions to set or get configurations of OSubject
     */
    OStatus SetReadyEntry(const OServiceEntry& readyEntry);
    const SubjectID& GetID(void) const { return  myID_; }
    OStatus SetBufferSize(size_t size);
    size_t  GetBufferSize(void) const { return maxBufferSize_; }
    OStatus SetNotifyUnitSize(size_t size);
    size_t  GetNotifyUnitSize(void) const { return notifyUnitSize_; }

    /*
     *   SetData() & NotifyObserver()
     */
    OStatus SetData(const OObserverInfo& info, RCRegion* r);
    OStatus SetData(const ObserverID& id, RCRegion* r);
    OStatus SetData(RCRegion* r);
    /*   Legacy Support   */
    OStatus SetData(const OObserverInfo& info, const OShmPtrBase& p);
    OStatus SetData(const ObserverID& id, const OShmPtrBase& p);
    OStatus SetData(const OShmPtrBase& p);
    OStatus SetData(const OObserverInfo& info, const void* buf, size_t size);
    OStatus SetData(const ObserverID& id, const void* buf, size_t size);
    OStatus SetData(const void* buf, size_t size);
    OStatus SetData(const OObserverInfo& info, MemoryRegionID mid, u_int offset);
    OStatus SetData(const ObserverID& id, MemoryRegionID memID, u_int offset);
    OStatus SetData(MemoryRegionID memID, size_t size);
    OStatus SetData(const OObserverInfo& info, const OMemoryLocator& l);
    OStatus SetData(const ObserverID& id, const OMemoryLocator& l);
    OStatus SetData(const OMemoryLocator& l);

    OStatus NotifyObserver(const OObserverInfo& observer);
    OStatus NotifyObserver(const ObserverID& observer);
    OStatus NotifyObservers(void);

	/*
	 *  Buffer control
	 */
    size_t  RemainBuffer(const OObserverInfo& observer) const;
    size_t  RemainBuffer(const ObserverID& id) const;
    size_t  RemainBuffer(void) const;

	OStatus ClearBuffer(const OObserverInfo& info);
	OStatus ClearBuffer(const ObserverID& id);
	OStatus ClearBuffer();
						
    /*
     *   Functions used in Stub
     */
    OStatus ControlHandler(const OControlMessage& msg, OStatus status=oSUCCESS);
    OStatus ReadyHandler(const OReadyMessage& msg);

    //  Legacy support
    OStatus ControlHandler(OControlMessage* msg) { return ControlHandler(*msg); }
    OStatus ReadyHandler  (OReadyMessage* msg)   { return ReadyHandler(*msg); }

    /*
     *   Iterators
     */
    int NumberOfObservers(void)       const;
    ObserverConstIterator begin(void) const { return observers_.begin(); }
    ObserverConstIterator end(void)   const { return observers_.end(); }

    ObserverConstIterator FindObserver(const ObserverID& id) const {
        return findObserver(id);
    }

    int ReadyStatus(const ObserverID& id) const;
    int ReadyStatus(const OObserverInfo& info) const {
        return info.impl_->ready_flag;
    }

    int IsAllReady(void) const;
    int IsAnyReady(void) const;

    /*  Legacy Support */
    int IsReady(void) const { return IsAnyReady(); }
    int IsReady(const ObserverID& id) const;
    int IsReady(const OObserverInfo& info) const {
        return (info.impl_->ready_flag > 0);
    }

#ifdef SUPPORT_OBJCOMM_VERSION_10
    int NumberOfReference(const OMemoryLocator& locator) const;
    int Lock(RCRegion* r);
    int Unlock(RCRegion* r);
#endif // SUPPORT_OBJCOMM_VERSION_10
};

//  For Debug
ostream& operator << (ostream& os, const SubjectID& id);

#endif // _OSubject_h_DEFINED
