//
// ODataArchive.h
//
// Copyright 1997,1998,1999,2000,2002 Sony Corporation
//

#ifndef _ODataArchive_h_DEFINED
#define _ODataArchive_h_DEFINED

#include <Types.h>
#include <OPENR/OStatus.h>

//
// Max number of ODataArchive file
//
const size_t odaMAX_SIZE = 32;

typedef longword ODataArchiveMagic;
const ODataArchiveMagic odamagicUNDEF = 0xffffffff; // UNDEFINED
const ODataArchiveMagic odamagicOSND  = 0x444e534f; // 'OSND' 
const ODataArchiveMagic odamagicODAR  = 0x5241444f; // 'ODAR' 
const ODataArchiveMagic odamagicOMTN  = 0x4e544d4f; // 'OMTN'
const ODataArchiveMagic odamagicWAVE  = 0x45564157; // 'WAVE'
const ODataArchiveMagic odamagicMIDI  = 0x4944494d; // 'MIDI'
const ODataArchiveMagic odamagicLED   = 0x0044454c; // 'LED '
const ODataArchiveMagic odamagicSYN   = 0x004e5953; // 'SYN ' 
const ODataArchiveMagic odamagicSTT   = 0x00545453; // 'STT ' 

typedef longword ODataArchiveID;
const ODataArchiveID odaID_UNDEF = 0xffffffff;

namespace ODataArchive {
    // Load() can't be used for OPEN-R SDK
    OStatus Load(char* path, ODataArchiveID* odaID);
    OStatus New(MemoryRegionID memID, void* oda, ODataArchiveID* odaID);
    OStatus Delete(ODataArchiveID odaID);
    int Find(ODataArchiveID odaID, char* name);
    int Find(ODataArchiveID odaID, ODataArchiveMagic magic, char* name);
    int GetNumFiles(ODataArchiveID odaID);
    MemoryRegionID GetMemoryRegionID(ODataArchiveID odaID);

    ODataArchiveMagic GetMagic(ODataArchiveID odaID, int index);
    char*             GetName(ODataArchiveID odaID, int index);
    int               GetSize(ODataArchiveID odaID, int index);
    byte*             GetData(ODataArchiveID odaID, int index);
    int               GetOffset(ODataArchiveID odaID, int index);

    OStatus SetData(ODataArchiveID odaID, int index, byte* data);
    OStatus Save(ODataArchiveID odaID, char* path);
}

#endif /* _ODataArchive_h_DEFINED */
