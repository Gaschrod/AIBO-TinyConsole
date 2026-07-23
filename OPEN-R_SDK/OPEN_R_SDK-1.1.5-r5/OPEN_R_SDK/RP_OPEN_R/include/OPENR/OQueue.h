//
// OQueue.h
//
// Copyright 1997,1998,1999,2000 Sony Corporation
//

#ifndef _OQueue_h_DEFINED
#define _OQueue_h_DEFINED

#include <OPENR/OTLStatus.h>

template <class T, int MaxSize>
class OQueue {
public:
    OQueue();
    OQueue(const OQueue<T,MaxSize>& que);
    ~OQueue();

    OQueue<T,MaxSize>& operator =(const OQueue<T,MaxSize>& que);

    OTLStatus PushBack(const T& item);
    OTLStatus PopFront();		
    T&        Front();

    void      Clear();
    int       GetSize();
    int       GetMaxSize();

protected:
    T data_[MaxSize];
    int size_;
    int head_;
    int tail_;

    void DeepCopy(const OQueue<T,MaxSize>& que);
};


template <class T, int MaxSize>
inline void
OQueue<T,MaxSize>::DeepCopy(const OQueue<T,MaxSize>& que)
{
    size_ = que.size_;
    head_  = que.head_;
    tail_  = que.tail_;

    for (int n = 0; n < MaxSize; ++n) data_[n] = que.data_[n];
}


template <class T, int MaxSize>
inline
OQueue<T,MaxSize>::OQueue()
{
    Clear();
}


template <class T, int MaxSize>
inline
OQueue<T,MaxSize>::~OQueue()
{
}


template <class T, int MaxSize>
inline 
OQueue<T,MaxSize>::OQueue(const OQueue<T,MaxSize>& que)
{
    DeepCopy(que);
}


template <class T, int MaxSize>
inline OQueue<T,MaxSize>&
OQueue<T,MaxSize>::operator =(const OQueue<T,MaxSize>& que)
{
    if (this != &que) DeepCopy(que);
    return *this;
}


template <class T, int MaxSize>
inline OTLStatus
OQueue<T,MaxSize>::PushBack(const T& item)
{
    if (size_ == MaxSize) return otlOVERFLOW;

    if (size_ == 0) {

	tail_ = 0;
	head_ = 0;
	data_[0] = item;

    } else {

	++tail_;
	if (tail_ == MaxSize) tail_ = 0;
	data_[tail_] = item;
    }

    ++size_;
    return otlSUCCESS;
}


template <class T, int MaxSize>
inline OTLStatus
OQueue<T,MaxSize>::PopFront()
{
    if (size_ == 0) return otlUNDERFLOW;

    --size_;
    if (size_ != 0) {
	++head_;
	if (head_ == MaxSize) head_ = 0;
    }

    return otlSUCCESS;
}


template <class T, int MaxSize>
inline T&
OQueue<T,MaxSize>::Front()
{
    if (size_ == 0) data_[head_] = T();
    return data_[head_];
}


template <class T, int MaxSize>
inline void
OQueue<T,MaxSize>::Clear()
{
    size_ = 0;
    head_  = 0;
    tail_  = 0;
}


template <class T, int MaxSize>
inline int
OQueue<T,MaxSize>::GetSize()
{
    return size_;
}


template <class T, int MaxSize>
inline int
OQueue<T,MaxSize>::GetMaxSize()
{
    return MaxSize;
}

#endif /* _OQueue_h_DEFINED */

