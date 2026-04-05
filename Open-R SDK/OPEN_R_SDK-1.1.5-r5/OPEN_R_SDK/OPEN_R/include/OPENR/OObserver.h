//
// OObserver.h
//
// Copyright 1999,2000,2001,2002 Sony Corporation
//

#ifndef _OObserver_h_DEFINED
#define _OObserver_h_DEFINED

#if defined(__GNUC__)
#include <iostream>
using std::cout;
using std::endl;
#else
#include <iostream.h>
#endif
#include <list>
using std::list;
#include <Types.h>
#include <OPENR/OPENR.h>
#include <OPENR/ObjcommTypes.h>
#include <OPENR/ObjcommMessages.h>
#include <OPENR/ObjcommEvent.h>
#include <OPENR/DataManager.h>
#include <OPENR/MemoryMapAgent.h>
#include <OPENR/OSubjectInfo.h>

// Iterators
typedef list<OSubjectInfo>::iterator            SubjectIterator;
typedef list<OSubjectInfo>::const_iterator      SubjectConstIterator;

//
//  Proxy class on obsever side
//
class OObserver {

  public:
    OObserver(void);
    virtual ~OObserver();

    //
    //  Parameter Initialization Method
    //
    OStatus SetNotifyEntry(const OServiceEntry& entry);
    void    SetSkip(size_t d) { d_skip_ = d; return; }
    void    SetBufCtrlParam(size_t skip, size_t min, size_t max) {
        d_skip_ = skip; d_min_  = min; d_max_  = max;
        return;
    }

    //
    //  Method used in Stub functions
    //
    OStatus ConnectHandler(const OConnectMessage& msg, OStatus s = oSUCCESS);
    OStatus NotifyHandler(const ONotifyMessage& msg, ONotifyEvent* pEvent);

    //  Legacy support
    OStatus ConnectHandler(OConnectMessage* msg) { return ConnectHandler(*msg); }
    OStatus NotifyHandler(ONotifyMessage* msg, ONotifyEvent* pEvent) {
        return NotifyHandler(*msg, pEvent);
    }

    //
    //  Ready functions
    //
    OStatus AssertReady(void)   { return SendReadyAll(objcommCMD_ASSERT_READY); }
    OStatus DeassertReady(void) { return SendReadyAll(objcommCMD_DEASSERT_READY); }
    OStatus AssertReady(const SubjectID& id) {
        return SendReady(objcommCMD_ASSERT_READY, id);
    }
    OStatus DeassertReady(const SubjectID& id) {
        return SendReady(objcommCMD_DEASSERT_READY, id);
    }
    OStatus AssertReady(const OSubjectInfo& info) {
        return SendReady(objcommCMD_ASSERT_READY, info);
    }
    OStatus DeassertReady(const OSubjectInfo& info) {
        return SendReady(objcommCMD_DEASSERT_READY, info);
    }

    //
    //   Iterator for subject selection
    //
    SubjectConstIterator begin(void) const { return subjects_.begin(); }
    SubjectConstIterator end(void)   const { return subjects_.end(); }

    //
    //   Internal status
    //
    SubjectID FindSender(RCRegion* region) const {
        return dataManager_.FindSender(region);
    }
    int NumberOfSubjects(void) const;
    const ObserverID&  GetObserverID(void) const { return myID_; } // LEGACY
    const ObserverID&  GetID(void) const         { return myID_; }

#ifdef DEBUG
    void Dump(unsigned int* address, size_t size);
#endif

  protected:
    //
    //   Internal use
    //
    void    SetObserverID(const ObserverID& entry) { myID_ = entry; return; }
	OStatus HandleAddObserver(const OConnectMessage& msg);
	OStatus HandleRemoveObserver(const OConnectMessage& msg);
	OStatus SendControlMessage(const OConnectMessage& msg,
							   ObjcommCommand command,
							   OControlReplyMessage* replyMsg);
	OStatus SendSelfControlMessage(const OConnectMessage& msg,
								   ObjcommCommand command,
								   OControlReplyMessage* replyMsg);
    OStatus SendReadyAll(int cmd);
    OStatus SendReady(int cmd, const SubjectID& subject);
    OStatus SendReady(int cmd, const OSubjectInfo& info);
	OStatus SendReadyV22(int cmd, const SubjectID& subject);
	OStatus SendReadyV10(int cmd, const SubjectID& subject);
	OStatus NotifyHandlerV22(const ONotifyMessage& msg,    ONotifyEvent* pEvent);
	OStatus NotifyHandlerV10(const ONotifyMessageV10& msg, ONotifyEvent* pEvent);
	RCRegion* ExtractRegisterData(char** rptr_p , const SubjectID& subjectID);
	SubjectConstIterator  findSubject(const SubjectID& id) const;
	SubjectIterator       findSubject(const SubjectID& id);

    //
    //  Observer's attribute
    //
    ObserverID        myID_;
    size_t            d_skip_;
    size_t            d_min_;
    size_t            d_max_;

    //
    //  Subjects' attribute and Data control
    //
    list<OSubjectInfo> subjects_;
    DataManager        dataManager_;    //  RCRegion control
    MemoryMapAgent     memMapAgent_;    //  attach & detach control

    //
    //  Message Area
    //
    OReadyMessage     readyMsg;

	//
	//  Support for version 1 common mode
	//
	//map<SubjectID, ObjcommVersion, less<SubjectID> >  versionMap_;
	OReadyMessageV10  readyMsgV10_;
	
  private:
    //  No copy allowed.
    OObserver(const OObserver&);
    OObserver& operator=(const OObserver&);
};

::ostream& operator << (::ostream& os, const ObserverID& observer);

#endif // _OObserver_h_DEFINED
