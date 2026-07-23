//
// OList.h
//
// Copyright 1999,2000,2001,2003,2004 Sony Corporation
//

#ifndef _OList_h_DEFINED
#define _OList_h_DEFINED

#include <OPENR/ODebug.h>
#include <OPENR/OTLStatus.h>
const int olistNULL = -1;


//
// There are two implementations of OList:
//
//    OList<T,MaxSize> is a primary class template, and
//    OList<T> is a partially specialized class template of OList<T,0>.
//

#ifndef __GNUC__
template <class T, int MaxSize=0>
#else
// OList<T,0> can't be compiled with gcc2.95.3.
template <class T, int MaxSize>
#endif /* __GNUC__ */
class OList {
public:
    OList();
    OList(const OList<T,MaxSize> & list);
    ~OList();

    OList<T,MaxSize>& operator =(const OList<T,MaxSize> & list);

    OTLStatus PushFront(const T& item);
    OTLStatus PushBack(const T& item);
    OTLStatus PopFront();		
    OTLStatus PopBack();		
    T&        Front();
    T&        Back();

    void      Clear();
    int       GetSize();
    int       GetMaxSize();

    class Iterator {
        friend class OList<T, MaxSize>;
    public:
        Iterator() : listp_(0), idx_(olistNULL) {}
        Iterator(OList<T, MaxSize>* l, int idx) : listp_(l), idx_(idx) {}
        ~Iterator() {}

        bool operator==(Iterator& rhs) {
            return (listp_ == rhs.listp_ && idx_ == rhs.idx_) ? true : false;
        }
        bool operator!=(Iterator& rhs) {
            return (listp_ == rhs.listp_ && idx_ == rhs.idx_) ? false : true;
        }
        T& operator*() { 
            if (idx_ != olistNULL) {
                return listp_->data_[idx_];
            } else {
                if (listp_ == 0) {
                    PRINTF(("### FATAL ERROR ### %s\n",
                            "OList<T,MaxSize>::Iterator::operator*()"));
                    PRINTF(("### FATAL ERROR ### %s\n",
                            "UNINITIALIZED ITERATOR IS REFERRED."));
                    PRINTF(("### FATAL ERROR ### EXCEPTION ...\n"));
                }
                listp_->defaultT_ = T();
                return listp_->defaultT_;
            }
        }
        Iterator operator++() {
            if (listp_ == 0) return Iterator();
            idx_ = (idx_ != olistNULL) ? listp_->next_[idx_] : olistNULL;
            return Iterator(listp_, idx_);
        }
        Iterator operator--() {
            if (listp_ == 0) return Iterator();
            idx_ = (idx_ != olistNULL) ? listp_->prev_[idx_] : listp_->tail_;
            return Iterator(listp_, idx_);
        }
        Iterator operator++(int) {
            Iterator tmp = *this;
            ++(*this);
            return tmp;
        }
        Iterator operator--(int) {
            Iterator tmp = *this;
            --(*this);
            return tmp;
        }

        
    private:
        OList<T, MaxSize>* listp_;
        int idx_;
    };
    friend class OList<T, MaxSize>::Iterator;

    Iterator Begin() { return Iterator(this, head_); }
    Iterator End()   { return Iterator(this, olistNULL); }
    OTLStatus Insert(Iterator iter, const T& item) {
        return __Insert__(iter.idx_, item);
    }
    OTLStatus Erase(Iterator iter) {
        return __Erase__(iter.idx_);
    }

#ifdef OPENR_DEBUG
    void Debug();
#endif /* OPENR_DEBUG */

protected:
    T data_[MaxSize];
    int prev_[MaxSize];
    int next_[MaxSize];

    int size_;
    int head_;
    int tail_;
    int free_;
    T   defaultT_;

    void DeepCopy(const OList<T,MaxSize> & list);
    int  NewData();
    void FreeData(int idx);

    OTLStatus __Erase__(int pos);
    OTLStatus __Insert__(int pos, const T& item);
};


template <class T, int MaxSize>
inline
OList<T,MaxSize>::OList()
{
    Clear();
}


template <class T, int MaxSize>
inline
OList<T,MaxSize>::OList(const OList<T,MaxSize>& list)
{
    DeepCopy(list);
}


template <class T, int MaxSize>
inline
OList<T,MaxSize>::~OList()
{
}


template <class T, int MaxSize>
inline OList<T,MaxSize>&
OList<T,MaxSize>::operator =(const OList<T,MaxSize>& list)
{
    if (this != &list) DeepCopy(list);
    return *this;
}


template <class T, int MaxSize>
inline OTLStatus
OList<T,MaxSize>::PushFront(const T & item)
{
    if (size_ == MaxSize) return otlOVERFLOW;

    int pos = NewData();
    if (size_ == 0) {
        data_[pos] = item;
        prev_[pos] = olistNULL;
        next_[pos] = olistNULL;
        head_ = tail_ = pos;
    } else {
        prev_[head_] = pos;
        data_[pos] = item;
        prev_[pos] = olistNULL;
        next_[pos] = head_;
        head_ = pos;
    }
    ++size_;

    return otlSUCCESS;
}


template <class T, int MaxSize>
inline OTLStatus
OList<T,MaxSize>::PushBack(const T & item)
{
    if (size_ == MaxSize) return otlOVERFLOW;

    int pos = NewData();
    if (size_ == 0) {
        data_[pos] = item;
        prev_[pos] = olistNULL;
        next_[pos] = olistNULL;
        head_ = tail_ = pos;
    } else {
        next_[tail_] = pos;
        data_[pos] = item;
        prev_[pos] = tail_;
        next_[pos] = olistNULL;
        tail_ = pos;
    }
    ++size_;

    return otlSUCCESS;
}


template <class T, int MaxSize>
inline OTLStatus
OList<T,MaxSize>::PopFront()
{
    if (size_ == 0) return otlUNDERFLOW;

    int new_head = next_[head_];
    prev_[new_head] = olistNULL;
    FreeData(head_);
    head_ = new_head;
    --size_;
    if (size_ == 0) {
        head_ = olistNULL;
        tail_ = olistNULL;
    }

    return otlSUCCESS;
}


template <class T, int MaxSize>
inline OTLStatus
OList<T,MaxSize>::PopBack()
{
    if (size_ == 0) return otlUNDERFLOW;

    int new_tail = prev_[tail_];
    next_[new_tail] = olistNULL;
    FreeData(tail_);
    tail_ = new_tail;
    --size_;
    if (size_ == 0) {
        head_ = olistNULL;
        tail_ = olistNULL;
    }

    return otlSUCCESS;
}


template <class T, int MaxSize>
inline T&
OList<T,MaxSize>::Front()
{
    if (size_ == 0) {
        defaultT_ = T();
        return defaultT_;
    }
    return data_[head_];
}


template <class T, int MaxSize>
inline T&
OList<T,MaxSize>::Back()
{
    if (size_ == 0) {
        defaultT_ = T();
        return defaultT_;
    }
    return data_[tail_];
}


template <class T, int MaxSize>
inline void
OList<T,MaxSize>::Clear()
{
    size_ = 0;
    head_ = olistNULL;
    tail_ = olistNULL;

    free_ = 0;
    for (int n = 0; n < MaxSize - 1; ++n) {
        next_[n] = n + 1;
    }
    next_[MaxSize - 1] = olistNULL;
}


template <class T, int MaxSize>
inline int
OList<T,MaxSize>::GetSize()
{
    return size_;
}


template <class T, int MaxSize>
inline int
OList<T,MaxSize>::GetMaxSize()
{
    return MaxSize;
}


#ifdef OPENR_DEBUG
template <class T, int MaxSize>
inline void
OList<T,MaxSize>::Debug()
{
    DPRINTF(("size_ %d\n", size_));
    DPRINTF(("head_ %d\n", head_));
    DPRINTF(("tail_ %d\n", tail_));
    DPRINTF(("free_ %d\n", free_));

    DPRINTF(("prev_[] "));
    for (int i = 0; i < MaxSize; i++) DPRINTF(("%d ", prev_[i]));
    DPRINTF(("\n"));

    DPRINTF(("next_[] "));
    for (int i = 0; i < MaxSize; i++) DPRINTF(("%d ", next_[i]));
    DPRINTF(("\n\n"));
}
#endif /* OPENR_DEBUG */


template <class T, int MaxSize>
inline void
OList<T,MaxSize>::DeepCopy(const OList<T,MaxSize>& list)
{
    size_ = list.size_;
    head_ = list.head_;
    tail_ = list.tail_;
    free_ = list.free_;


    for (int n = 0; n < MaxSize; ++n) {
        data_[n] = list.data_[n];
        prev_[n] = list.prev_[n];
        next_[n] = list.next_[n];
    }
}


template <class T, int MaxSize>
inline int
OList<T,MaxSize>::NewData()
{
    if (size_ == MaxSize) return olistNULL;

    int pos = free_;
    free_ = next_[pos];
    return pos;
}


template <class T, int MaxSize>
inline void
OList<T,MaxSize>::FreeData(int pos)
{
    next_[pos] = free_;
    free_ = pos;
}


template <class T, int MaxSize>
inline OTLStatus
OList<T,MaxSize>::__Erase__(int pos)
{
    if (size_ == 0)       return otlUNDERFLOW;
    if (pos == olistNULL) return otlFAIL;
    if (pos == head_)     return PopFront();
    if (pos == tail_)     return PopBack();

    int prev = prev_[pos];
    int next = next_[pos];
    next_[prev] = next_[pos];
    prev_[next] = prev_[pos];
    FreeData(pos);
    --size_;

    return otlSUCCESS;
}


template <class T, int MaxSize>
inline OTLStatus
OList<T,MaxSize>::__Insert__(int pos, const T& item)
{
    if (size_ == MaxSize) return otlOVERFLOW;
    if (pos == olistNULL) return PushBack(item);
    if (pos == head_)     return PushFront(item);

    int data = NewData();
    int prev = prev_[pos];
    next_[prev] = data;
    prev_[pos] = data;
    prev_[data] = prev;
    next_[data] = pos;
    data_[data] = item;
    ++size_;

    return otlSUCCESS;
}

#ifndef __GNUC__
#include <Types.h>
extern "C" {
    sError  NewRegion (size_t size, void** pBaseAddress);
    sError  DeleteRegion (void* pBaseAddress);
}
template <class T> class OListMemory;

//
//   OList<T> defined below is a partially specialized
//   class template of OList<T,MaxSize>.
//
//   OList<T,0> can't be compiled with gcc2.95.3.
//

template <class T>
class OList<T,0> {
public:
    OList();
    OList(const OList<T> & list);
    ~OList();

    OList<T>& operator =(const OList<T> & list);

    OTLStatus Init(int maxSize);

    OTLStatus PushFront(const T& item);
    OTLStatus PushBack(const T& item);
    OTLStatus PopFront();
    OTLStatus PopBack();
    T&        Front();
    T&        Back();

    void      Clear();
    int       GetSize();
    int       GetMaxSize();

    class Iterator {
        friend class OList<T>;
    public:
        Iterator() : listp_(0), idx_(olistNULL) {}
        Iterator(OList<T>* l, int idx) : listp_(l), idx_(idx) {}
        ~Iterator() {}

        bool operator==(Iterator& rhs) {
            return (listp_ == rhs.listp_ && idx_ == rhs.idx_) ? true : false;
        }
        bool operator!=(Iterator& rhs) {
            return (listp_ == rhs.listp_ && idx_ == rhs.idx_) ? false : true;
        }
        T& operator*() { 
            if (idx_ != olistNULL) {
                return listp_->memory_.Data(idx_);
            } else {
                if (listp_ == 0) {
                    PRINTF(("### FATAL ERROR ### %s\n",
                            "OList<T>::Iterator::operator*()"));
                    PRINTF(("### FATAL ERROR ### %s\n",
                            "UNINITIALIZED ITERATOR IS REFERRED."));
                    PRINTF(("### FATAL ERROR ### EXCEPTION ...\n"));
                }
                listp_->defaultT_ = T();
                return listp_->defaultT_;
            }
        }
        Iterator operator++() {
            if (listp_ == 0) return Iterator();
            idx_ = (idx_ != olistNULL) ? listp_->memory_.Next(idx_) : olistNULL;
            return Iterator(listp_, idx_);
        }
        Iterator operator--() {
            if (listp_ == 0) return Iterator();
            idx_ = (idx_ != olistNULL) ? listp_->memory_.Prev(idx_) : listp_->tail_;
            return Iterator(listp_, idx_);
        }
        Iterator operator++(int) {
            Iterator tmp = *this;
            ++(*this);
            return tmp;
        }
        Iterator operator--(int) {
            Iterator tmp = *this;
            --(*this);
            return tmp;
        }

        
    private:
        OList<T>* listp_;
        int idx_;
    };
    friend class OList<T>::Iterator;

    Iterator Begin() { return Iterator(this, head_); }
    Iterator End()   { return Iterator(this, olistNULL); }
    OTLStatus Insert(Iterator iter, const T& item) {
        return __Insert__(iter.idx_, item);
    }
    OTLStatus Erase(Iterator iter) {
        return __Erase__(iter.idx_);
    }

#ifdef OPENR_DEBUG
    void Debug();
#endif /* OPENR_DEBUG */

protected:
    
    OListMemory<T> memory_;

    int size_;
    int head_;
    int tail_;
    int free_;
    T   defaultT_;

    void DeepCopy(const OList<T> & list);
    int  NewData();
    void FreeData(int idx);

    OTLStatus __Erase__(int pos);
    OTLStatus __Insert__(int pos, const T& item);
};


template <class T>
inline
OList<T>::OList()
{
    Clear();
}


template <class T>
inline
OList<T>::OList(const OList<T>& list)
{
    DeepCopy(list);
}


template <class T>
inline
OList<T>::~OList()
{
}


template <class T>
inline OList<T>&
OList<T>::operator =(const OList<T>& list)
{
    if (this != &list) DeepCopy(list);
    return *this;
}


template <class T>
inline OTLStatus
OList<T>::Init(int maxSize)
{
    if (size_ != 0) return otlFAIL;
    
    OTLStatus result = memory_.Init(maxSize);
    Clear();
    return result;
}


template <class T>
inline OTLStatus
OList<T>::PushFront(const T & item)
{
    if (size_ == memory_.MaxSize()) return otlOVERFLOW;

    int pos = NewData();
    if (size_ == 0) {
        memory_.Data(pos) = item;
        memory_.Prev(pos) = olistNULL;
        memory_.Next(pos) = olistNULL;
        head_ = tail_ = pos;
    } else {
        memory_.Prev(head_) = pos;
        memory_.Data(pos)   = item;
        memory_.Prev(pos)   = olistNULL;
        memory_.Next(pos)   = head_;
        head_ = pos;
    }
    ++size_;

    return otlSUCCESS;
}


template <class T>
inline OTLStatus
OList<T>::PushBack(const T & item)
{
    if (size_ == memory_.MaxSize()) return otlOVERFLOW;

    int pos = NewData();
    if (size_ == 0) {
        memory_.Data(pos) = item;
        memory_.Prev(pos) = olistNULL;
        memory_.Next(pos) = olistNULL;
        head_ = tail_ = pos;
    } else {
        memory_.Next(tail_) = pos;
        memory_.Data(pos)   = item;
        memory_.Prev(pos)   = tail_;
        memory_.Next(pos)   = olistNULL;
        tail_ = pos;
    }
    ++size_;

    return otlSUCCESS;
}


template <class T>
inline OTLStatus
OList<T>::PopFront()
{
    if (size_ == 0) return otlUNDERFLOW;

    int new_head = memory_.Next(head_);
    memory_.Prev(new_head) = olistNULL;
    FreeData(head_);
    head_ = new_head;
    --size_;
    if (size_ == 0) {
        head_ = olistNULL;
        tail_ = olistNULL;
    }

    return otlSUCCESS;
}


template <class T>
inline OTLStatus
OList<T>::PopBack()
{
    if (size_ == 0) return otlUNDERFLOW;

    int new_tail = memory_.Prev(tail_);
    memory_.Next(new_tail) = olistNULL;
    FreeData(tail_);
    tail_ = new_tail;
    --size_;
    if (size_ == 0) {
        head_ = olistNULL;
        tail_ = olistNULL;
    }

    return otlSUCCESS;
}


template <class T>
inline T&
OList<T>::Front()
{
    if (size_ == 0) {
        defaultT_ = T();
        return defaultT_;
    }
    return memory_.Data(head_);
}


template <class T>
inline T&
OList<T>::Back()
{
    if (size_ == 0) {
        defaultT_ = T();
        return defaultT_;
    }
    return memory_.Data(tail_);
}


template <class T>
inline void
OList<T>::Clear()
{
    size_ = 0;
    head_ = olistNULL;
    tail_ = olistNULL;

    if (memory_.MaxSize() == 0) {
        free_ = olistNULL;
    } else {
        free_ = 0;
        for (int n = 0; n < memory_.MaxSize() - 1; ++n) {
            memory_.Next(n) = n + 1;
        }
        memory_.Next(memory_.MaxSize() - 1) = olistNULL;
    }
}


template <class T>
inline int
OList<T>::GetSize()
{
    return size_;
}


template <class T>
inline int
OList<T>::GetMaxSize()
{
    return memory_.MaxSize();
}


#ifdef OPENR_DEBUG
template <class T>
inline void
OList<T>::Debug()
{
    DPRINTF(("size_ %d\n", size_));
    DPRINTF(("head_ %d\n", head_));
    DPRINTF(("tail_ %d\n", tail_));
    DPRINTF(("free_ %d\n", free_));

    DPRINTF(("prev_[] "));
    for (int i = 0; i < memory_.MaxSize(); i++)
        DPRINTF(("%d ", memory_.Prev(i)));
    DPRINTF(("\n"));

    DPRINTF(("next_[] "));
    for (int i = 0; i < memory_.MaxSize(); i++)
        DPRINTF(("%d ", memory_.Next(i)));
    DPRINTF(("\n\n"));
}
#endif /* OPENR_DEBUG */


template <class T>
inline void
OList<T>::DeepCopy(const OList<T>& list)
{
    memory_ = list.memory_;
    
    size_ = list.size_;
    head_ = list.head_;
    tail_ = list.tail_;
    free_ = list.free_;
}


template <class T>
inline int
OList<T>::NewData()
{
    if (free_ == olistNULL) return olistNULL;

    int pos = free_;
    free_ = memory_.Next(pos);
    return pos;
}


template <class T>
inline void
OList<T>::FreeData(int pos)
{
    memory_.Next(pos) = free_;
    free_ = pos;
}


template <class T>
inline OTLStatus
OList<T>::__Erase__(int pos)
{
    if (size_ == 0)        return otlUNDERFLOW;
    if (pos == olistNULL)  return otlFAIL;
    if (pos == head_)      return PopFront();
    if (pos == tail_)      return PopBack();

    int prev = memory_.Prev(pos);
    int next = memory_.Next(pos);
    memory_.Next(prev) = next;
    memory_.Prev(next) = prev;
    FreeData(pos);
    --size_;

    return otlSUCCESS;
}


template <class T>
inline OTLStatus
OList<T>::__Insert__(int pos, const T& item)
{
    if (size_ == memory_.MaxSize()) return otlOVERFLOW;
    if (pos == olistNULL)           return PushBack(item);
    if (pos == head_)               return PushFront(item);

    int data = NewData();
    int prev = memory_.Prev(pos);
    memory_.Next(prev) = data;
    memory_.Prev(pos)  = data;
    memory_.Prev(data) = prev;
    memory_.Next(data) = pos;
    memory_.Data(data) = item;
    ++size_;

    return otlSUCCESS;
}


template <class T>
class OListMemory {
public:
    OListMemory() {
        maxSize_ = 0;
        data_    = 0;
        prev_    = 0;
        next_    = 0;
    }
    OListMemory(int maxSize) { Allocate_(maxSize); }
    OListMemory(const OListMemory<T> & memory) { DeepCopy_(memory); }
    ~OListMemory() { Release_(); }
    
    OListMemory<T>& operator =(const OListMemory<T> & memory) {
        if (this != &memory) {
            Release_();
            DeepCopy_(memory);
        }
        return *this;
    }
    
    OTLStatus Init(int maxSize) {
        Release_();
        return Allocate_(maxSize);
    }
    
    int  MaxSize()   const {return maxSize_;}
    T&   Data(int i) const {return data_[i];}
    int& Prev(int i) const {return prev_[i];}
    int& Next(int i) const {return next_[i];}
    
protected:
    int maxSize_;
    T   *data_;
    int *prev_;
    int *next_;
    
    // struct {
    //    T   data_[maxSize_];
    //    int prev_[maxSize_];
    //    int next_[maxSize_];
    // };
    
private:
    OTLStatus Allocate_(int maxSize);
    void      DeepCopy_(const OListMemory<T> & memory);
    void      Release_();
};

template <class T>
inline OTLStatus
OListMemory<T>::Allocate_(int maxSize)
{
    maxSize_ = maxSize;
    
    if (maxSize_ == 0) {
        data_ = 0;
    } else {
        const int allocSize = (sizeof(T) + sizeof(int) * 2) * maxSize_;
        
        if (sSUCCESS != NewRegion(allocSize, (void**)&data_)) {
            PRINTF(("### MEMORY ALLOCATION FAILED ### %s\n",
                    "OListMemory<T>::Allocate_()"));
            
            maxSize_ = 0;
            data_    = 0;
            prev_    = 0;
            next_    = 0;
            
            return otlNOMEMORY;
        }
    }
    
    prev_    = reinterpret_cast<int*>(data_ + maxSize_);
    next_    = prev_ + maxSize_;
    
    return otlSUCCESS;
}

template <class T>
inline void
OListMemory<T>::DeepCopy_(const OListMemory<T> & memory)
{
    if (otlSUCCESS == Allocate_(memory.maxSize_)) {
        for (int n = 0; n < maxSize_; ++n) {
            data_[n] = memory.data_[n];
            prev_[n] = memory.prev_[n];
            next_[n] = memory.next_[n];
        }
    }
}

template <class T>
inline void
OListMemory<T>::Release_()
{
    if (data_) {
        DeleteRegion(data_);
        
        maxSize_ = 0;
        data_    = 0;
        prev_    = 0;
        next_    = 0;
    }
}
#endif /* __GNUC__ */

#endif /* _OList_h_DEFINED */
