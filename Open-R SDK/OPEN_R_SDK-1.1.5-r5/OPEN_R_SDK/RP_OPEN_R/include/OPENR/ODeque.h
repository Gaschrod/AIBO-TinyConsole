//
// ODeque.h
//
// Copyright 1997,1998,1999,2000 Sony Corporation
//

#ifndef _ODeque_h_DEFINED
#define _ODeque_h_DEFINED

#include <OPENR/OTLStatus.h>

template <class T, int MaxSize>
class ODeque {
public:
    ODeque();
    ODeque(const ODeque<T,MaxSize>& deque);
    ~ODeque();

    ODeque<T,MaxSize>& operator =(const ODeque<T,MaxSize>& deque);

    OTLStatus PushFront(const T& item);
    OTLStatus PushBack(const T& item);
    OTLStatus PopFront();		
    OTLStatus PopBack();		
    T&        Front();
    T&        Back();

    void      Clear();
    int       GetSize();
    int       GetMaxSize();

protected:
    T data_[MaxSize];

    int size_;
    int head_;
    int tail_;

    void DeepCopy(const ODeque<T,MaxSize>& deque);
};


template <class T, int MaxSize>
inline void
ODeque<T,MaxSize>::DeepCopy(const ODeque<T,MaxSize>& deque)
{
    size_ = deque.size_;
    head_  = deque.head_;
    tail_  = deque.tail_;

    for (int n = 0; n < MaxSize; ++n) data_[n] = deque.data_[n];
}


template <class T, int MaxSize>
inline
ODeque<T,MaxSize>::ODeque()
{
    Clear();
}


template <class T, int MaxSize>
inline
ODeque<T,MaxSize>::ODeque(const ODeque<T,MaxSize>& deque)
{
    DeepCopy(deque);
}


template <class T, int MaxSize>
inline
ODeque<T,MaxSize>::~ODeque()
{
}


template <class T, int MaxSize>
inline ODeque<T,MaxSize>&
ODeque<T,MaxSize>::operator =(const ODeque<T,MaxSize>& deque)
{
    if (this != &deque) DeepCopy(deque);
    return *this;
}


template <class T, int MaxSize>
inline OTLStatus
ODeque<T,MaxSize>::PushFront(const T& item)
{
    if (size_ == MaxSize) return otlOVERFLOW;

    if (size_ == 0) {

	tail_ = 0;
	head_ = 0;
	data_[0] = item;

    } else {

        if (head_ == 0)
	    head_ = MaxSize - 1;
	else
	    --head_;
	data_[head_] = item;
    }

    ++size_;
    return otlSUCCESS;
}


template <class T, int MaxSize>
inline OTLStatus
ODeque<T,MaxSize>::PushBack(const T& item)
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
ODeque<T,MaxSize>::PopFront()
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
inline OTLStatus
ODeque<T,MaxSize>::PopBack()
{
    if (size_ == 0) return otlUNDERFLOW;

    --size_;
    if (size_ != 0) {
        if (tail_ == 0)
	    tail_ = MaxSize - 1;
	else
	    --tail_;
    }

    return otlSUCCESS;
}


template <class T, int MaxSize>
inline T&
ODeque<T,MaxSize>::Front()
{
    if (size_ == 0) data_[head_] = T();
    return data_[head_];
}


template <class T, int MaxSize>
inline T&
ODeque<T,MaxSize>::Back()
{
    if (size_ == 0) data_[tail_] = T();
    return data_[tail_];
}


template <class T, int MaxSize>
inline void
ODeque<T,MaxSize>::Clear()
{
    size_ = 0;
    head_  = 0;
    tail_  = 0;
}


template <class T, int MaxSize>
inline int
ODeque<T,MaxSize>::GetSize()
{
    return size_;
}


template <class T, int MaxSize>
inline int
ODeque<T,MaxSize>::GetMaxSize()
{
    return MaxSize;
}

#endif /* _ODeque_h_DEFINED */
