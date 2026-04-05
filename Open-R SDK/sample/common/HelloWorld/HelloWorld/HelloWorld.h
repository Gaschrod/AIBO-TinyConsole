//
// Copyright 2002 Sony Corporation 
//
// Permission to use, copy, modify, and redistribute this software for
// non-commercial use is hereby granted.
//
// This software is provided "as is" without warranty of any kind,
// either expressed or implied, including but not limited to the
// implied warranties of fitness for a particular purpose.
//

#ifndef HelloWorld_h_DEFINED
#define HelloWorld_h_DEFINED

#include <OPENR/OObject.h>

class HelloWorld : public OObject {
public:
    HelloWorld();
    virtual ~HelloWorld() {}

    virtual OStatus DoInit   (const OSystemEvent& event);
    virtual OStatus DoStart  (const OSystemEvent& event);
    virtual OStatus DoStop   (const OSystemEvent& event);
    virtual OStatus DoDestroy(const OSystemEvent& event);
};

#endif // HelloWorld_h_DEFINED
