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

#ifndef _GInfo_h
#define _GInfo_h

#include <iostream>
using std::cout;
using std::endl;

const unsigned int numOfG = 3;
const int undefG = 0x80000000;

struct GInfo {
    //
    // data
    //
    int dir_[numOfG];

    //
    // constructor
    //
    GInfo() {
        for (int i = 0; i < numOfG; ++i) {
            dir_[i] = undefG;
        }
    }

    GInfo(int d1, int d2, int d3) {
        dir_[0] = d1;
        dir_[1] = d2;
        dir_[2] = d3;
    }

    GInfo(const GInfo& a) {
        *this = a;
    }    

    //
    // operator
    //
    GInfo& operator=(const GInfo& a) {
        if (this != &a) {
            for (int i = 0; i < numOfG; ++i) {
                dir_[i] = a.dir_[i];
            }
        }

        return *this;
    }

    bool operator==(const GInfo& a) const { 
        for (int i = 0; i < numOfG; ++i) {
            if (dir_[i] != a.dir_[i]) {
                return false;
            }
        }

        return false;
    }

    bool operator!=(const GInfo& a) const { 
        if (*this == a) {
            return false;
        }

        return true;
    }

    //
    // set
    //
    void Set(int d1, int d2, int d3) {
        dir_[0] = d1;
        dir_[1] = d2;
        dir_[2] = d3;
    }

    //
    // is
    //
    bool IsValid() const {
        bool bo(false);

        for (int i = 0; i < numOfG; i++) {
            if (dir_[i] != undefG) {
                bo = true;
            }
        }

        return bo;
    }

    //
    // print
    //
    void Print() const {
        for (int i = 0; i < numOfG; i++) {
            cout << dir_[i] << " ";
        }

        cout << endl;
    }
};

#endif // GInfo
