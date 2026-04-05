//
// StackAllocator.h
//
// Copyright 2002 Sony Corporation
//

#ifndef StackAllocator_h_DEFINED
#define StackAllocator_h_DEFINED

#include <OPENR/ArrayStack.h>


template <class T, unsigned N>
class StackAllocator {
public:
    explicit StackAllocator();
    virtual ~StackAllocator();

    T* New();
    void  Delete(T* ptr);

private:
    void Push(T* ptr);

    StackAllocator(const StackAllocator&);		// disabled
    StackAllocator& operator = (const StackAllocator&);	// disabled
  
    ArrayStack<T*> stack_;
    T*             dataArea_;
};


template <class T, unsigned N>
inline
StackAllocator<T,N>::StackAllocator()
  : stack_(N), dataArea_(0)
{
  // Allocate actual data area 
  dataArea_ = new T[N];
  // store pointers into stack
  for (unsigned i=0; i<N; ++i){
    stack_.Push(static_cast<T*>(dataArea_ + i)); 
  }
}

template <class T, unsigned N>
inline
StackAllocator<T,N>::~StackAllocator()
{
  if (dataArea_ != 0){
    delete[] dataArea_;
  }
}

template <class T, unsigned N>
inline T*
StackAllocator<T,N>::New()
{
  if (stack_.Empty()){
    return (T *)::operator new(sizeof(T));
  } else {    
    return stack_.Pop();
  }
}

template <class T, unsigned N>
inline void
StackAllocator<T,N>::Delete(T* ptr)
{
  if (dataArea_ <= ptr &&  ptr < dataArea_ + N){
    stack_.Push(ptr);
  } else {
    ::operator delete(ptr);
  }
}

#endif // #ifdef StackAllocator_h_defined





