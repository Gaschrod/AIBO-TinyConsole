//
// _mcoop.h
//
// Copyright 2002 Sony Corporation
//
#ifndef _mcoop_h_DEFINED
#define _mcoop_h_DEFINED

#include <string.h>
#include <sys/types.h>
#include <Types.h>
#include <OID.h>
#include <MCOOPError.h>

//
//  IMPORTANT !!!!!!
//  myname and currentMsg should be defined in stub main file of
//  each object.
//
extern char   myname[];
extern struct AperiosMessage currentMsg;

class RelativeTime {
public:
    longword        sec;
    longword        msec;
    RelativeTime () : sec (0), msec (0) {}
    RelativeTime (longword sc) : sec (sc), msec (0) {}
    RelativeTime (longword sc, longword msc) : sec (sc), msec (msc) {}

    // These operators are OPEN-R Emulator original. Be careful!
    bool operator < (const RelativeTime& t) {
        if (sec != t.sec)
            return sec < t.sec;
        else
            return msec < t.msec;
    }
    RelativeTime& operator += (const RelativeTime& t) {
        msec += t.msec;
        sec += (t.sec + msec/1000);
        msec = msec % 1000;
        return *this;
    }
};

//
// signal handler
//
extern "C" void Terminate(int cause);

//
// mCOOP API
//
extern "C" {
    //
    // OID
    //
    sError WhoAmI(OID* object);
    sError Find(const char*, OID* object);
    sError Register(OID object, const char* name);
    sError Unregister(const char* name);
    
    //
    //  Message Passing
    //
    sError Send(OID dest, Selector meth, void* msg, size_t sizeOfMsg);
    sError SendWithRBox(OID dest, Selector meth,
                        void* msg, size_t sizeOfMsg, RID* rBoxID);
    sError Receive(RID rBoxID, void* msg = 0, size_t sizeOfMsg = 0,
                   size_t* sizeOfReceivedMsg = 0, RelativeTime* timeout = 0);
    sError CleanRBox(RID rBoxID);
    sError Call(OID dest, Selector meth, void* msg, size_t sizeOfMsg,
                void* replyMsg = 0, size_t sizeOfReplyMsg = 0);
    sError Reply(void* msg = 0, size_t sizeOfMsg = 0);

    //
    // Shared Memory
    //
    sError NewSharedMemoryRegion(size_t size, MemoryProtectionInfo info,
                                 MemoryRegionID* memRegionID, void** baseAddr);
    sError DeleteSharedMemoryRegion(MemoryRegionID memRegionID);
    sError AllowToAttachMemoryRegion(MemoryRegionID memRegionID,
                                     MemoryProtectionInfo info,
                                     OID destObjID, bool givingOwnership);
    sError AttachMemoryRegion(MemoryRegionID memRegionID,
                              MemoryProtectionInfo info,  void** baseAddr);
    sError DetachMemoryRegion(MemoryRegionID memRegionID);

    //
    // Memory
    //
    sError NewRegion(size_t size, void** pBaseAddress);
    sError DeleteRegion(void* pBaseAddress);
}

//
// version number of stub-generator user uses
//
extern float stubgenVersion;
#ifndef STUBGEN_VERSION_defined
#define STUBGEN_VERSION_defined
#endif // STUBGEN_VERSION_defined

const long msgType = 128; // Normal Aperios Message

struct AperiosMessage {
    static const size_t MaxDataSize = 256;

    longword type;
    Selector sel;
    RID      resultBox;
    size_t   datasize;
    char     data[MaxDataSize];

    //  For Receive() and etc
    AperiosMessage(void) : type(0), sel(0), resultBox(), datasize(MaxDataSize) {}

    //  For Send()
    AperiosMessage(Selector s, void* src, size_t size)
        : type(msgType), sel(s), resultBox() { Set(src, size); }

    //  For Call() or SendWithRBox()
    AperiosMessage(Selector s, void* src, size_t size, RID rid)
        : type(msgType), sel(s), resultBox(rid) { Set(src, size); }

    //  For Reply()
    AperiosMessage(void* src, size_t size, longword id)
        : type(id), sel(0x8fffffff), resultBox() { Set(src, size); }

    size_t RealSize(void) const {
        return MessageSizeFor(datasize);
    }

    static size_t Size(void) {
        return MessageSizeFor(MaxDataSize);
    }
    static size_t MessageSizeFor(size_t size) {
        return (sizeof(AperiosMessage) - sizeof(longword) - (MaxDataSize - size));
    }
    static size_t PayloadSizeOf(size_t size) {
        return (size - MessageSizeFor(0));
    }

    void Set(void* src, size_t size);
};

//
// RegistryManager
//
const Selector REGISTRY_MANAGER_REGISTER   = 0;
const Selector REGISTRY_MANAGER_UNREGISTER = 1;
const Selector REGISTRY_MANAGER_FIND       = 2;

struct RegistryManagerMsg {
    char name[128];
    OID  oid;

    RegistryManagerMsg() : oid() {
        name[0] = 0;
    }

    RegistryManagerMsg(const char* n, OID o) : oid(o) {
        strcpy(name, n);
    }

    RegistryManagerMsg(const RegistryManagerMsg& msg) {
        strcpy(name, msg.name);
        oid = msg.oid;
    }
};

extern "C" void  SetRegistryManagerKey(char* key);
extern "C" key_t GetRegistryManagerKey();

// to avoid error in some systems
class msgbuf;

#endif
