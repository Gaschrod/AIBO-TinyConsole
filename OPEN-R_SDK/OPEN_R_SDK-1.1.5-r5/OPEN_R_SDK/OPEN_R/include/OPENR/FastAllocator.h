//
// FastAllocator.h
//
// Copyright 2002 Sony Corporation
//

#ifndef FastAllocator_h_DEFINED
#define FastAllocator_h_DEFINED

#include <OPENR/OSyslog.h>
#include <list>
using std::list;

template <class T>
class FastAllocatorBase {
public:
    explicit FastAllocatorBase(unsigned n=0);
    virtual ~FastAllocatorBase();

    T* New();
    void  Delete(T* ptr);

private:
    void Push(T* ptr);

    FastAllocatorBase(const FastAllocatorBase&);		// disabled
    FastAllocatorBase& operator = (const FastAllocatorBase&);	// disabled

    list<T*> pool;
};


template <class T>
inline
FastAllocatorBase<T>::FastAllocatorBase(unsigned n)
: pool()
{
    for (unsigned i=0; i<n; ++i){
	Push(static_cast<T*>(::operator new(sizeof(T))));
    }
}

template <class T>
inline
FastAllocatorBase<T>::~FastAllocatorBase()
{
    list<T*>::iterator itr = pool.begin();
    list<T*>::iterator last = pool.end();

    while (itr != last) {
	::operator delete(*itr);
	++ itr;
    }

    pool.erase(pool.begin(), pool.end());
}

template <class T>
inline T*
FastAllocatorBase<T>::New()
{
    if (pool.empty()) {
	static size_t n = 0;
	const size_t limit = 200;
	if (++n > limit) {
	    OSYSLOG1((osyslogWARNING,
		      "FastAllocatorBase::New() Too many data allocation (%d)",
		      n));
	}
	return static_cast<T*>(::operator new(sizeof(T)));
    } else {
	T* ret = pool.front();
	pool.pop_front();
	return ret;
    }
}

template <class T>
inline void
FastAllocatorBase<T>::Delete(T* ptr)
{
    Push(ptr);
}

template <class T>
inline void
FastAllocatorBase<T>::Push(T* ptr)
{
    pool.push_back(ptr);
}


template <class T, unsigned N>
class FastAllocator : public FastAllocatorBase<T> {
public:
    FastAllocator() : FastAllocatorBase<T>(N) {}
private:
    FastAllocator(const FastAllocator&);			// disabled
    FastAllocator& operator = (const FastAllocator&);		// disabled
};


#endif
