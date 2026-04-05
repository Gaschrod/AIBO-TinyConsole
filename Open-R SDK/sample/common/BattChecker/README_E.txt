** BattChecker Sample program **

There are two samples in this folder. Both samples are executed only on
Windows.

1. This (BattChecker) is a sample program using win32api and Remote
   Processing OPEN-R. It opens a simple window on the host PC and 
   shows the remaining charge level of AIBO's battery.

2. There are two programs. One (BattChecker) is a sample program using
   Remote Processing OPEN-R that is executed on the host PC.
   Another one (BattViewer) is a MFC windows application. 
   These programs exchange data by using inter-process communication
   (shared memory) in Windows.

   The program (BattChecker) using Remote Processing OPEN-R retrieves the
   remaining charge level of AIBO's battery and writes it to shared memory.  
   The Windows application (BattViewer) using MFC reads this value from 
   shared memory and displays it in a progress bar window.
   The MFC windows application (BattViewer) is made by using Visual C++ 6.0.

[+] Directory tree
BattChecker--+--BattChecker--+--host--+--MFC Makefile(for host)
             |               |        |
             |               |        +--win32 Makefile(for host)
             |               |
             |               +--robot Makefile(for robot)
             |
             +--MS (if not using RP OPEN-R; for robot)
             |
             +--RP--+--host--+--MFC--MS  (if using RP OPEN-R; for host)
             |      |        |
             |      |        +--win32-MS (if using RP OPEN-R; for host)
             |      |
             |      +--robot--MS (if using RP OPEN-R; for robot)
             |
             +--BattViewer (MFC application; for showing battery level)

[+]How to build / Usage

There are three ways you can use this sample program:  with AIBO only, 
as a simple Win32 program, or as a Win32 program with shared memory 
and MFC.

1 : Not defining OPENR_WIN32API and OPENR_SHAREDMEM_FOR_MFCAPP
    This sample is for AIBO only (no PC required).
    The remaining battery charge level is printed to the Wireless LAN
    console.

    HOW TO USE:
      $ cd sample/BattCheker
      $ make install
      $ cd MS
      $ cp -rf OPEN-R <Memory Stick Drive letter>:

2 : Define OPENR_WIN32API
    This sample uses Remote Processing OPEN-R for Windows only (without
    shared memory constructs or MFC).
    This is executed on the host PC.
    The remaining battery charge level is printed in a pop-up message
    window.

    HOW TO USE:
      $ cd sample/BattCheker/RP/robot
      $ make install
      $ cd MS
      $ cp -rf OPEN-R <Memory Stick Drive letter>:
      $ cd ../../host/win32
      $ make install
      $ /usr/local/OPEN_R_SDK/RP_OPENR_R/bin/start-rp-openr
     (Read the Installation guide to setup Remote Processing OPEN-R)
     (ipc-daemon must be executed before using start-rp-openr)

3 : Define OPENR_WIN32API and OPENR_SHAREDMEM_FOR_MFCAPP
    This sample is for Remote Processing OPEN-R for Windows only.
    This is executed on the host PC, and uses shared memory to send data to
    BattViewer.
    BattViewer is an MFC application that is compiled by Visual C++.
    It retrieves the data from BattChecker by using inter-process
    communication.
    BattViewer shows you the remaining battery charge level using a progress
    bar.

    HOW TO USE:
      Build BattViewer.exe by using Visual C++ (It is made by VC++6.0)

      $ cd sample/BattCheker/RP/robot
      $ make install
      $ cd MS
      $ cp -rf OPEN-R <Memory Stick Drive letter>:
      (AIBO's program is same as the one above)
      $ cd ../../host/MFC
      $ make install
      $ /usr/local/OPEN_R_SDK/RP_OPENR_R/bin/start-rp-openr
     (Read Installation guide to setup Remote Processing OPEN-R)
     (ipc-daemon must be executed before using start-rp-openr)

      Execute BattViewer.exe by using VC++ or 
      by the normal procedure of starting a program (double click, etc).

----
"Windows" is a registered trademark of Microsoft Corporation in the US and 
other countries.
