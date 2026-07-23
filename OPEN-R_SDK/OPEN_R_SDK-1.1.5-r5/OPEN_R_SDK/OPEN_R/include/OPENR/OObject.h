//
// OObject.h
//
// Copyright 1997,1998,1999,2000 Sony Corporation
//

#ifndef _OObject_h_DEFINED
#define _OObject_h_DEFINED

#include <OPENR/OPENR.h>
#include <OPENR/OPENREvent.h>

class OObjectManagerProxy;
class OServiceManagerProxy;

class OObject {
public:
    OObject();
    virtual ~OObject();

    void Init   (const OSystemEvent& event);
    void Start  (const OSystemEvent& event);
    void Stop   (const OSystemEvent& event);
    void Destroy(const OSystemEvent& event);
    
    virtual OStatus DoInit   (const OSystemEvent& event);
    virtual OStatus DoStart  (const OSystemEvent& event);
    virtual OStatus DoStop   (const OSystemEvent& event);
    virtual OStatus DoDestroy(const OSystemEvent& event);

protected:
    OID  myOID_;
    OStatus RegisterServiceEntry(const OServiceEntry& entry, const char* name);

private:
    static OObjectManagerProxy   objectManager_;
    static OServiceManagerProxy  serviceManager_;

    // These method are forbidden.
    OObject(const OObject&);
    OObject& operator=(const OObject&);
};

#endif /* _OObject_h_DEFINED */
