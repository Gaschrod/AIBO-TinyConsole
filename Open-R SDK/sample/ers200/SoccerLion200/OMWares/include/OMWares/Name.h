//
// Copyright 1999,2002 Sony Corporation 
//
// Permission to use, copy, modify, and redistribute this software for
// non-commercial use is hereby granted.
//
// This software is provided "as is" without warranty of any kind,
// either expressed or implied, including but not limited to the
// implied warranties of fitness for a particular purpose.
//

#ifndef _Name_h
#define _Name_h

#include <iostream>
using std::cout;
using std::endl;
#include <string.h>

const int sizeOfName = 32; 

struct Name {
    //
    // data
    //
    char s_[sizeOfName + 1];

    //
    // constructor
    //
    Name() {
        s_[0] = '\0';
        s_[sizeOfName] = '\0';
    };

    Name(const char* s) { 
        if (s != NULL) {
            strncpy(s_, s, sizeOfName);
            s_[sizeOfName] = '\0';
        } 
        else {
            s_[0] = '\0';
        }
    };

    Name(const Name& a) {
        *this = a;
    }

    //
    // destructor
    //
    ~Name() {}

    //
    // operator
    //
    operator char*() { 
        return s_; 
    }

    operator const char*() const { 
        return s_; 
    }

    Name& operator=(const Name& x) {
        if (this != &x) {
            strncpy(s_, x.s_, sizeOfName);
            s_[sizeOfName] = '\0';
        }
        return *this;
    }

    Name& operator=(const char* s) {
        if (s != NULL) {
            strncpy(s_, s, sizeOfName);
        }
        else {
            s_[0] = '\0';
        }
        return *this;
    }

    bool operator==(const Name& a) const {
        return (strcmp(s_, a.s_) == 0)? true : false;
    }

    bool operator!=(const Name& a) const {
        return (strcmp(s_, a.s_) != 0)? true : false;
    }

    bool operator<(const Name& a) const {
        return (strcmp(s_, a.s_) < 0)? true : false;
    }

    bool operator>(const Name& a) const {
        return (strcmp(s_, a.s_) > 0)? true : false;
    }

    //
    // print
    //
    void Print() const {
        cout << s_ << endl;
    }
};

#endif
