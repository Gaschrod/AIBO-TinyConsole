//
// OMemoryLocator.h
//
// Copyright 1998,2002 Sony Corporation
//

#ifndef OMemoryLocator_h_DEFINED
#define OMemoryLocator_h_DEFINED

#if defined(__GNUC__)
#include <iostream>
using std::cout;
using std::endl;
using std::ostream;
#else
#include <iostream.h>
#endif
#include <MCOOP.h>
#include <OID.h>

struct OMemoryLocator {

    MemoryRegionID   mid;
    u_int            offset;

    OMemoryLocator() : mid(UNDEF), offset(UNDEF) {}
    OMemoryLocator(MemoryRegionID m, u_int o) : mid(m), offset(o) {}
    OMemoryLocator(const OMemoryLocator& e)
	: mid(e.mid), offset(e.offset) {}

    void Set(MemoryRegionID m, u_int o) { mid = m; offset = o; }
    bool IsInvalid() const { return (mid == UNDEF && offset == UNDEF); }
    void MakeInvalid() { mid = UNDEF; offset = UNDEF; }

    friend bool operator ==(const OMemoryLocator& a,
			    const OMemoryLocator& b)
    {
	return (a.mid == b.mid && a.offset == b.offset);
    }

    friend bool operator < (const OMemoryLocator& a,
			    const OMemoryLocator& b)
    {
	if (a.mid < b.mid) {
	    return true;
	} else if (a.mid == b.mid) {
	    return (a.offset < b.offset);
	} else {
	    return false;
	}
    }
};

inline ostream& operator << ( ostream& output, const OMemoryLocator& loc )
{
    return output << "mid = " << (void *)loc.mid << ", offset = "
	<< (void *)loc.offset;
}

#endif // OMemoryLocator_h_DEFINED
