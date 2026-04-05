//
// SharedMemoryHeader.h
//
// Copyright 2002 Sony Corporation
//

#ifndef SharedMemoryHeader_h_DEFINED
#define SharedMemoryHeader_h_DEFINED

#include <MCOOP.h>
#include <OPENR/ODataFormats.h>

#if 0
/*
 *  SharedMemoryHeader for DRX900VirtualRobot ODataVectorInfo
 */
//
//----------------------------------------------------------------------
// ODataVectorInfo
//----------------------------------------------------------------------
//
/*
struct ODataVectorInfo {          // 32 bytes (total)
    ODataType        type;        //  4 bytes
    size_t           maxNumData;  //  4 bytes
    size_t           numData;     //  4 bytes
    size_t           infoOffset;  //  4 bytes
    size_t           infoSize;    //  4 bytes
    size_t           totalSize;   //  4 bytes
    MemoryRegionID   memRegionID; //  4 bytes
    void*            physAddr;    //  4 bytes

    void Init(ODataType t, size_t maxData, size_t iOffset,
              size_t iSize, size_t total, MemoryRegionID memID, void* phys);
    void SetNumData(size_t ndata);
};
*/


struct SharedMemoryHeader {
    ODataType        type;        //  4 bytes
    size_t           maxNumInfo;  //  4 bytes
    size_t           numInfo;     //  4 bytes
    size_t           numData;     //  4 bytes
    size_t           offset;      //  4 bytes
    size_t           totalSize;   //  4 bytes
    MemoryRegionID   memID;       //  4 bytes
    void*            physAddr;    //  4 bytes

/*
    ShmRegion() : baseadr(0), size(0), memID(0), offset(0) {}
    ShmRegion(void* b, size_t s) : baseadr(b), size(s) {}
    ShmRegion(void* b, size_t s, MemoryRegionID id, unsigned int o)
	: baseadr(b), size(s), memID(id), offset(o) {}
*/
#else

    /*
     *  SharedMemoryHeader for DRX900VirtualRobot ODataVectorInfo
     */
    //
    //----------------------------------------------------------------------
    // ODataVectorInfo
    //----------------------------------------------------------------------
    //
/*
const size_t odataOPTIONAL_MAX = 128;

struct ODataVectorInfo {	                  // 192 bytes (total)
    MemoryRegionID   memRegionID;                 //   4 bytes
    void*            physAddr;	                  //   4 bytes
    size_t           offset;                      //   4 bytes
    size_t           totalSize;                   //   4 bytes
    ODataType        type;	                  //   4 bytes
    size_t           infoOffset;                  //   4 bytes
    size_t           infoSize;	                  //   4 bytes
    size_t           maxNumData;                  //   4 bytes
    size_t           numData;	                  //   4 bytes
    OVRSyncKey       syncKey;                     //   4 bytes
    longword         wait;                        //   4 bytes
    size_t           optOffset;			  //   4 bytes
    size_t	     optSize;			  //   4 bytes
    longword         padding[3];                  //  12 bytes
    byte             optional[odataOPTIONAL_MAX]; // 128 bytes
};
*/

struct SharedMemoryHeader {     // total 16 bytes
    MemoryRegionID   memID;       //  4 bytes
    void*            physAddr;    //  4 bytes
    size_t           offset;      //  4 bytes
    size_t           totalSize;   //  4 bytes
    //ODataType        type;        //  4 bytes
    //size_t           maxNumInfo;  //  4 bytes
    //size_t           numInfo;     //  4 bytes
    //size_t           numData;     //  4 bytes

#endif

    void Print() {
	cout << "memID     :" << (void *)memID << endl;
	cout << "physAddr  :" << (void *)physAddr << endl;
	cout << "offset    :" << offset << endl;
	cout << "totalSize :" << totalSize << endl;
	//cout << "type      :" << type << endl;
	//cout << "maxNumInfo:" << maxNumInfo << endl;
	//cout << "numInfo   :" << numInfo << endl;
	//cout << "numData   :" << numData << endl;
    }
};

typedef SharedMemoryHeader ShmHdr;


#if 0
/*
 *  Data exchange VRComm <---> Objcomm2
 */
SharedMemoryHeader*
ODataVectorInfo2SharedMemoryHeader(ODataVectorInfo* info);

SharedMemoryHeader*
OImageData2SharedMemoryHeader(OImageData* imageData, MemoryRegionID id);
#endif

#endif



