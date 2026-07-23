//
// OSubjectInfo.h
//
// Copyright 2002 Sony Corporation
//

#ifndef  OSubjectInfo_h_DEFINED
#define  OSubjectInfo_h_DEFINED

#include <OPENR/ObjcommTypes.h>

class OSubjectInfo : public SubjectID {
public:
	OSubjectInfo() : SubjectID(), version_(objcommVERSION_UNDEF) {}
	OSubjectInfo(const SubjectID& id, ObjcommVersion v)
		: SubjectID(id), version_(v) {}
	
	const SubjectID& GetSubjectID() const { return *this;    }
	ObjcommVersion   GetVersion() const   { return version_; }
	
private:
	friend class OObserver;
	void SetSubjectID(const SubjectID& id) { *(SubjectID *)this = id; }
	void SetVersion(ObjcommVersion v)      { version_ = v;            }
	
	ObjcommVersion version_;
};

#endif
