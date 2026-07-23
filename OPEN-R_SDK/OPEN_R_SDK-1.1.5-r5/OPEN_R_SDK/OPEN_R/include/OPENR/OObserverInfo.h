//
// OObserverInfo.h
//
// Copyright 2002 Sony Corporation
//

#ifndef  OObserverInfo_h_DEFINED
#define  OObserverInfo_h_DEFINED

#include <OPENR/ObjcommTypes.h>
#include <OPENR/RCRegion.h>

typedef RCRegion* RCRegion_p;

struct _OObserverInfo {
    ObserverID       id;
    //  Fixed parameters
    size_t           d_skip;
    size_t           d_min;
    size_t           d_max;
    //  Observer status parameters
    int              ready_flag;    // 1:Assert, 0:Sent, -1:Deassert
    size_t           n_notify;      // number of Notify()
    size_t           n_data;        // number of SetData()
    size_t           skip_counter;
    //  Data buffer
    //  Be careful ! This member is allocated dynamically.
    RCRegion_p*      queue;
    size_t           q_size;
    //  libObjectComm version of observer
    ObjcommVersion   version;

    _OObserverInfo()
      :id(), d_skip(0), d_min(0), d_max(0), ready_flag(0), n_notify(0),
       n_data(0), skip_counter(0), queue(0), q_size(0), 
       version(objcommVERSION_UNDEF) {}

    _OObserverInfo(ObserverID i, size_t skip, size_t min, size_t max,
	       ObjcommVersion v)
      : id(i), d_skip(skip), d_min(min), d_max(max), ready_flag(0),
	n_notify(0), n_data(0), skip_counter(0), queue(0), q_size(0),
	version(v) {}

    ~_OObserverInfo() {}

    //
    //  Buffer is created and destroyed by the resposibility of users.
    //
    void CreateBuffer(size_t size){
        if (queue != 0) DestroyBuffer();
        queue = new RCRegion_p[size];
        q_size = size;
    }

    void DestroyBuffer(){
        if (queue != 0){
            delete[] queue;
            queue  = 0;
            q_size = 0;
        }
    }
};

class OObserverInfo {
public:
	const ObserverID& GetObserverID() const { return impl_->id; }
	OObserverInfo() : impl_(0) {}	

private:
	friend class OSubject;

	OObserverInfo(_OObserverInfo* info) : impl_(info) {}
    void CreateBuffer(size_t size){ impl_->CreateBuffer(size); }
	void DestroyBuffer() { impl_->DestroyBuffer(); }
	void Delete() {	delete impl_; }
	
	_OObserverInfo*  impl_;
};

typedef OObserverInfo ObserverInfo; // LEGACY SUPPORT

#endif // OObserverInfo_h_DEFINED
