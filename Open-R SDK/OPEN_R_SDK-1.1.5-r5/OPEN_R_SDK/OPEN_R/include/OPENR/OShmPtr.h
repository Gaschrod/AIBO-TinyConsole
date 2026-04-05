//
// Copyright (C) 2001 Sony Corporation
// All Rights Reserved.
//

#ifndef OShmPtr_h_DEFINED
#define OShmPtr_h_DEFINED

#include <string.h>
#include <OPENR/RCRegion.h>
#include <OPENR/OShmPtrBase.h>
#include <OPENR/SharedMemoryAllocator.h>

template <class T>
class OShmPtr : public OShmPtrBase {
  public:
    /* -------  Proxy class start here ---------*/
    class Proxy {
      public:
        Proxy(OShmPtr<T>& p, int index): ptr(p), index_(index) {}

        //  Proxy& operator=(const Proxy& rhs);
        Proxy& operator=(const T& rhs)
        {
            if (ptr.pointee_->NumberOfReference() > 1){
                ptr.AllocateCopy(ptr.pointee_->Size(), ptr.BaseAddress());
            }
            *( (T *)ptr.pointee_->Base() + index_) = rhs;

            return *this;
        }
        operator T() const { return *( (T *)ptr.BaseAddress() + index_ ); }

      private:
        OShmPtr<T>& ptr;
        int         index_;
    };
    /* -------  Proxy class end here ---------*/

    OShmPtr() : OShmPtrBase() {}
    explicit OShmPtr(size_t size) : OShmPtrBase() { Allocate(size); }
    OShmPtr(const OShmPtrBase& rhs) : OShmPtrBase(rhs) {}
    //explicit OShmPtr(const T& rhs) {AllocateCopy( sizeof(T), (T *)&rhs );}
    explicit OShmPtr(RCRegion* p) : OShmPtrBase(p) {}

    const T& operator*() const { return *( (T *)pointee_->Base() ); }
    const T& operator[](int index) const
        { return *( (T *)pointee_->Base() + index ); }
    Proxy operator*()           { return Proxy(*this, 0); }
    Proxy operator[](int index) { return Proxy(*this, index); }
    const T* operator->()       { return ((T *)pointee_->Base()); }

    /*  number of elements  */
    int    NumOfElement() const { return (pointee_ == 0) ? 0 : pointee_->Size()/sizeof(T); }
    int    Size() const { return NumOfElement(); }

    void   Copy(const void* s2, size_t size);

    void Allocate(int n=1);
    void AllocateCopy(size_t size, void* src);

  private:
    friend class Proxy;

};


template <class T>
void OShmPtr<T>::Copy(const void* src, size_t size)
{
    if ( size > Size() ) size = Size();
    if (pointee_->NumberOfReference() > 1){
        Allocate(Size());   // allocate same size
    }
    ::memcpy(BaseAddress(), src, size);
}

template <class T>
void OShmPtr<T>::Allocate(int n)
{
    if (pointee_ != 0) pointee_->RemoveReference();
    pointee_ = new RCRegion(n*sizeof(T));
#ifdef DEBUG
    cout << "OShmPtr<T>::Allocate(int n)" << endl;
    cout << "\tthis = " << (void *)this << endl;
    cout << "\tpointee_ = " << (void *)pointee_ << endl;
#endif
    //    init();
}

template <class T>
void OShmPtr<T>::AllocateCopy(size_t size, void* src)
{
    if (pointee_ != 0) pointee_->RemoveReference();

    pointee_ = new RCRegion(size);
    ::memcpy(BaseAddress(), src, size);
    //    init();
}

template <class T>
void memcpy(OShmPtr<T>& s1, const void *s2, size_t dataSize)
{
    s1.Copy(s2, dataSize);
}

#endif
