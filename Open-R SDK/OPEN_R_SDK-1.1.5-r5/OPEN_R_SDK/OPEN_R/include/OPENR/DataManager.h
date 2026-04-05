//
// DataManager.h
//
// Copyright (C) 1997-2001 Sony Corporation
// All Rights Reserved.
//

#ifndef _DataManager_h_DEFINED
#define _DataManager_h_DEFINED

#include <list>
using std::list;
#include <Types.h>
#include <OPENR/OPENR.h>
#include <OPENR/ObjcommTypes.h>
#include <OPENR/RCRegion.h>

struct DataInfo {
    SubjectID  subject;
    RCRegion*  myRCRegion;
    RCRegion*  srcRCRegion;

    DataInfo() : subject(), myRCRegion(0), srcRCRegion(0) {}
    DataInfo(const SubjectID& s, RCRegion* r, RCRegion* o)
      : subject(s), myRCRegion(r), srcRCRegion(o) {}
};

class DataManager {

protected:
    list<DataInfo>  dataInfo_;
    list<DataInfo>::iterator iter_crrnt;

private:
    DataManager( const DataManager& );			// disable
    DataManager& operator=( const DataManager& );	// disable

public:
    DataManager();
    ~DataManager();

    void Register(const SubjectID&, RCRegion*, RCRegion* );
    void InitUnregistration();
    DataInfo Unregister(const SubjectID&);
    void Clear(const SubjectID& subject);
    void ClearAll();

    SubjectID FindSender(RCRegion* ptr) const;

};

#endif // _DataManager_h_DEFINED
