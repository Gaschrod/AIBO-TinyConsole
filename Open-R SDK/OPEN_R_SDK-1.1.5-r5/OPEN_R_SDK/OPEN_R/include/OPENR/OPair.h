//
// OPair.h
//
// Copyright 1997,1998,1999,2000,2001 Sony Corporation
//

#ifndef _OPair_h_DEFINED
#define _OPair_h_DEFINED

template <class T1, class T2>
struct OPair {
    T1 first;
    T2 second;

    OPair() {}
    OPair(const T1& a, const T2& b) : first(a), second(b) {}
    const OPair& operator=(const OPair<T1, T2>& pair) {
        if (this == &pair) return *this;
        first = pair.first;
        second = pair.second;
        return *this;
    }
};

#endif /* _OPair_h_DEFINED */
