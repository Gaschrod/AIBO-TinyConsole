//
// ArrayStack.h
//
// Copyright 1998,1999,2000,2001,2002 Sony Corporation
//

#ifndef _ArrayStack_h_DEFINED
#define _ArrayStack_h_DEFINED

#include <Types.h>

template <class T>
class  ArrayStack {
private:
    unsigned int  sp_;      // stack pointer
    size_t        max_;     // max size
    T*            stack_;   // stack base address
    
public:
    ArrayStack();
    ~ArrayStack();
    ArrayStack( size_t n );
    
    void   Allocate( size_t n );
    void   Push( const T& );
    T      Pop();
    bool   Empty() { return ( sp_ == 0 ); }
    size_t Size()  { return ( sp_ ); }
};

template <class T>
ArrayStack<T>::ArrayStack()
    : sp_(0),
      max_(0),
      stack_(0)
{
}

template <class T>
ArrayStack<T>::~ArrayStack()
{
    if ( stack_ != 0 ){
        delete stack_;
    }
}

template <class T>
ArrayStack<T>::ArrayStack( size_t n )
    : sp_(0),
      max_(n)
{
    stack_ = new T[n];
    if ( stack_ == 0 ){
        cout << "Error ! ArrayStack::ArrayStack( int n )" << endl;
        cout << "\tCannot allocate memory" << endl;
    }
}

template <class T>
void ArrayStack<T>::Allocate( size_t n )
{
    if ( stack_ != 0 ){ 
        delete stack_;
    }
    
    stack_ = new T[n];
    if ( stack_ == 0 ){
        cout << "Error ! ArrayStack::ArrayStack( int n )" << endl;
        cout << "\tCannot allocate memory" << endl;
    }

    max_ = n;
}

template <class T>
void ArrayStack<T>::Push( const T& data )
{
    if ( sp_ < max_ ){
        stack_[ sp_++ ] = data;
    } else {
        cout << "Error ! ArrayStack<T>::Push()" << endl;
        cout << "\tCannot push any more data" << endl;
    }
}

template <class T>
T ArrayStack<T>::Pop()
{
    if ( sp_ > 0 ){
        return  stack_[ -- sp_ ];
    } else {
        cout << "Error ! ArrayStack<T>::Pop()" << endl;
        cout << "\tNo data to pop out" << endl;
        return  T();
    }
}

#endif /* _ArrayStack_h_DEFINED */
