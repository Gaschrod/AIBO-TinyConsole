//
// Copyright 2004 Sony Corporation 
//
// Permission to use, copy, modify, and redistribute this software for
// non-commercial use is hereby granted.
//
// This software is provided "as is" without warranty of any kind,
// either expressed or implied, including but not limited to the
// implied warranties of fitness for a particular purpose.
//
//
//
// [+]How to build / Usage
// 
// There are three ways you can use this sample program:  with AIBO only, 
// as a simple Win32 program, or as a Win32 program with shared memory 
// and MFC.
// 
// 1 : Not defining OPENR_WIN32API and OPENR_SHAREDMEM_FOR_MFCAPP
//     This sample is for AIBO only (no PC required).
//     The remaining battery charge level is printed to the Wireless LAN
//     console.
// 
//     HOW TO USE:
//       $ cd sample/BattCheker
//       $ make install
//       $ cd MS
//       $ cp -rf OPEN-R <Memory Stick Drive letter>:
// 
// 2 : Define OPENR_WIN32API
//     This sample uses Remote Processing OPEN-R for Windows only (without
//     shared memory constructs or MFC).
//     This is executed on the host PC.
//     The remaining battery charge level is printed in a pop-up message
//     window.
// 
//     HOW TO USE:
//       $ cd sample/BattCheker/RP/robot
//       $ make install
//       $ cd MS
//       $ cp -rf OPEN-R <Memory Stick Drive letter>:
//       $ cd ../../host/win32
//       $ make install
//       $ /usr/local/OPEN_R_SDK/RP_OPENR_R/bin/start-rp-openr
//      (Read the Installation guide to setup Remote Processing OPEN-R)
//      (ipc-daemon must be executed before using start-rp-openr)
// 
// 3 : Define OPENR_WIN32API and OPENR_SHAREDMEM_FOR_MFCAPP
//     This sample is for Remote Processing OPEN-R for Windows only.
//     This is executed on the host PC, and uses shared memory to send data to
//     BattViewer.
//     BattViewer is an MFC application that is compiled by Visual C++.
//     It retrieves the data from BattChecker by using inter-process
//     communication.
//     BattViewer shows you the remaining battery charge level using a progress
//     bar.
// 
//     HOW TO USE:
//       Build BattViewer.exe by using Visual C++ (It is made by VC++6.0)
// 
//       $ cd sample/BattCheker/RP/robot
//       $ make install
//       $ cd MS
//       $ cp -rf OPEN-R <Memory Stick Drive letter>:
//       (AIBO's program is same as the one above)
//       $ cd ../../host/MFC
//       $ make install
//       $ /usr/local/OPEN_R_SDK/RP_OPENR_R/bin/start-rp-openr
//      (Read Installation guide to setup Remote Processing OPEN-R)
//      (ipc-daemon must be executed before using start-rp-openr)
// 
//       Execute BattViewer.exe by using VC++ or 
//       by the normal procedure of starting a program (double click, etc).

#include <math.h>
#ifdef OPENR_WIN32API
#include <windows.h>
#include <stdio.h>
#include <signal.h>
#endif
#include <OPENR/OPENRAPI.h>
#include <OPENR/OUnits.h>
#include <OPENR/OSyslog.h>
#include <OPENR/core_macro.h>
#include "BattChecker.h"

#ifdef OPENR_WIN32API
#ifdef OPENR_SHAREDMEM_FOR_MFCAPP
static const char MapName[]         = "MemMap1";
static const char MutexName[]       = "Mutex1";
static HANDLE MapFileHandle = NULL;
static HANDLE hMem1         = NULL;
static HANDLE hMutex        = NULL;
static LPVOID pShrMem1      = NULL;

struct BCHK_DATA {
    int cap;
    HWND hWnd;
};

union SHAREDMEM {
	void *p_v;
	BCHK_DATA *p_d;
};

static SHAREDMEM pUnionShrMem;

void sigint_handler(int);

#endif
#endif

//
// The initialization code to get sensor information is contained in this sample program
// The sensor information which could get it with NotifySensor() isn't being used.
// The sensor information can be gotten by easy remodeling.
//

BattChecker::BattChecker()
{
    is_init = false;
#ifdef OPENR_WIN32API
#ifdef OPENR_SHAREDMEM_FOR_MFCAPP
    signal(SIGINT, &sigint_handler );
#endif
#endif
}

OStatus
BattChecker::DoInit(const OSystemEvent& event)
{
    OSYSDEBUG(("BattChecker::DoInit()\n"));

    NEW_ALL_SUBJECT_AND_OBSERVER;
    REGISTER_ALL_ENTRY;
    SET_ALL_READY_AND_NOTIFY_ENTRY;

    OPENR::SetMotorPower(opowerON);
    
    return oSUCCESS;
}

OStatus
BattChecker::DoStart(const OSystemEvent& event)
{
    OSYSDEBUG(("BattChecker::DoStart()\n"));

    ENABLE_ALL_SUBJECT;
    ASSERT_READY_TO_ALL_OBSERVER;

    return oSUCCESS;
}

OStatus
BattChecker::DoStop(const OSystemEvent& event)
{
    OSYSDEBUG(("BattChecker::DoStop()\n"));

    DISABLE_ALL_SUBJECT;
    DEASSERT_READY_TO_ALL_OBSERVER;

    return oSUCCESS;
}

OStatus
BattChecker::DoDestroy(const OSystemEvent& event)
{
    DELETE_ALL_SUBJECT_AND_OBSERVER;

#ifdef OPENR_WIN32API
#ifdef OPENR_SHAREDMEM_FOR_MFCAPP
    UnmapViewOfFile(pShrMem1);
    pUnionShrMem.p_v = NULL;
    CloseHandle(hMem1);
    CloseHandle(hMutex);
#endif
#endif
    return oSUCCESS;
}


void
BattChecker::NotifySensor(const ONotifyEvent& event)
{
    char buff[64];
    
    OSensorFrameVectorData* sensorVec = (OSensorFrameVectorData*)event.Data(0);

    if (is_init == false) {
        is_init = true;
#ifdef OPENR_WIN32API
#ifdef OPENR_SHAREDMEM_FOR_MFCAPP
        MapFileHandle = (HANDLE) 0xFFFFFFFF;
        hMutex = CreateMutex(NULL, FALSE, MutexName);
        if (hMutex == NULL)
            OSYSLOG1((osyslogERROR, "CreateMutex()\n"));
        WaitForSingleObject(hMutex, INFINITE);
        hMem1 = CreateFileMapping(MapFileHandle, NULL, PAGE_READWRITE, 0, 4096, MapName);
        DWORD dwExist = GetLastError();
        pUnionShrMem.p_v = pShrMem1 = MapViewOfFile(hMem1, FILE_MAP_WRITE | FILE_MAP_READ, 0, 0, 4096);
        if (pShrMem1 && (dwExist != ERROR_ALREADY_EXISTS)) {
            // Initialize shared memory
            pUnionShrMem.p_d->hWnd = 0;
            pUnionShrMem.p_d->cap = 0;
        }
        ReleaseMutex(hMutex);
        if (hMem1 == NULL)
            OSYSLOG1((osyslogERROR, "CreateFileMapping()\n"));
        if (pShrMem1 == NULL)
            OSYSLOG1((osyslogERROR, "MapViewOfFile()\n"));
#endif
#endif
    }
    OPowerStatus result;
    OStatus err = OPENR::GetPowerStatus(&result);
    if (err == oSUCCESS){
        int capacity = (int)result.remainingCapacity;
        OSYSPRINT(("Batt[%d]\n",capacity));
#ifdef OPENR_WIN32API
#ifdef OPENR_SHAREDMEM_FOR_MFCAPP
        if (pShrMem1) {
            WaitForSingleObject(hMutex, INFINITE);
            // Get Window Handler of BattViewer
            HWND hWndBattViewer = pUnionShrMem.p_d->hWnd;
            // Set Battery capacity to the shared memory
            pUnionShrMem.p_d->cap = capacity;
            ReleaseMutex(hMutex);
            if (hWndBattViewer)
                PostMessage(hWndBattViewer, WM_USER+1, 0, 0);
        }
#else
        sprintf(buff, "Batt[%d]", capacity);
        MessageBox (NULL, buff, "Battery Remain", MB_OK);
#endif // OPENR_SHAREDMEM_FOR_MFCAPP
#endif // OPENR_WIN32API
    } else{
        OSYSLOG1((osyslogERROR, "Failed to GetPowerStatus()\n"));
    }
    observer[event.ObsIndex()]->AssertReady();
}

void
BattChecker::ReadyEffector(const OReadyEvent& event)
{
// Dummy function to avoid a Shared Memory Error when the RP OPEN-R object is
// terminated by pressing Ctrl-C
    return;
}

#ifdef OPENR_WIN32API
#ifdef OPENR_SHAREDMEM_FOR_MFCAPP
void sigint_handler(int a)
{
    if (pShrMem1) {
        WaitForSingleObject(hMutex, INFINITE);
        // Get Window Handler of BattViewer
        HWND hWndBattViewer = pUnionShrMem.p_d->hWnd;
        // Set Battery capacity to the shared memory
        pUnionShrMem.p_d->cap = 0;
        ReleaseMutex(hMutex);
        if (hWndBattViewer)
            PostMessage(hWndBattViewer, WM_USER+1, 0, 0);
    }
    exit(1);
}
#endif
#endif

