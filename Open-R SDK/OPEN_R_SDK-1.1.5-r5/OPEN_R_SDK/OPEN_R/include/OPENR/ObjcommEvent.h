//
// ObjcommEvent.h
//
// Copyright (C) 2000-2001 ony Corporation
// All Rights Reserved.
//

#ifndef _ObjcommEvent_h_DEFINED
#define _ObjcommEvent_h_DEFINED

#include <OPENR/ObjcommTypes.h>
#include <OPENR/ObjcommMessages.h>


class OConnectEvent {
private:
    int         obsIndex_;
    char*       subName_;
    OConnectCmd command_;

public:
    OConnectEvent(int index, char* name, OConnectCmd command);
    ~OConnectEvent();

    int          ObsIndex()     const;
    const char*  SubName()      const;
    bool         IsConnect()    const;
    bool         IsDisconnect() const;
};


class OControlEvent {
private:
    int            sbjIndex_;
    ObserverID     observerID_;
    char*          subName_;
    ObjcommCommand command_;

public:
    OControlEvent(int index, char* name, ObjcommCommand command);
    OControlEvent(int index, ObserverID id, char* name, ObjcommCommand command);
    ~OControlEvent();

    int          SbjIndex()         const;
    const char*  SubName()          const;
    bool         IsAddObserver()    const;
    bool         IsRemoveObserver() const;
    const ObserverID& SenderID()    const;
};


class OReadyEvent {
private:
    int             sbjIndex_;
    ObserverID      observerID_;
    ObjcommCommand  command_;

public:
    OReadyEvent(int index, ObserverID id, ObjcommCommand command);
    ~OReadyEvent();

    int                SbjIndex()   const;
    const ObserverID&  SenderID()   const;
    bool               IsAssert()   const;
    bool               IsDeassert() const;
};

#include <OPENR/OSubjectInfo.h>

class ONotifyEvent {
private:
    int             obsIndex_;
	OSubjectInfo    subjectInfo_;
    int             nData_;
    int             nNotify_;
    RCRegion*       data_[MAX_NUM_DATA];

public:
    void SetIndex(int i)                      { obsIndex_ = i; }
	void SetSubjectInfo(const OSubjectInfo& s) { subjectInfo_ = s; }
    void SetNumOfData(int n)                  { nData_ = n; }
    void SetNumOfNotify(int n)                { nNotify_ = n; }
    void SetData(int index, RCRegion* r)      { data_[index] = r; }


    ONotifyEvent():obsIndex_(0), subjectInfo_(), nData_(0), nNotify_(0){}
    ~ONotifyEvent() {};

    int                 ObsIndex()    const { return obsIndex_; }
	const OSubjectInfo& SenderID()    const { return subjectInfo_; }
    int                 NumOfData()   const { return nData_; }
    int                 NumOfNotify() const { return nNotify_; }

    const void*         Data(int i) const;
    const void**        Data() const;
    RCRegion*           RCData(int i) const;
};

#endif // _ObjcommEvent_h




