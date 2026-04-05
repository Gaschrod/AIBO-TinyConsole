; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CDisplayThread
LastTemplate=CWinThread
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "BattViewer.h"

ClassCount=4
Class1=CBattViewerApp
Class2=CBattViewerDlg
Class3=CAboutDlg

ResourceCount=5
Resource1=IDD_ABOUTBOX
Resource2=IDR_MAINFRAME
Resource3=IDD_BATTVIEWER_DIALOG
Resource4=IDD_ABOUTBOX (âpåÍ (±“ÿ∂))
Class4=CDisplayThread
Resource5=IDD_BATTVIEWER_DIALOG (âpåÍ (±“ÿ∂))

[CLS:CBattViewerApp]
Type=0
HeaderFile=BattViewer.h
ImplementationFile=BattViewer.cpp
Filter=N

[CLS:CBattViewerDlg]
Type=0
HeaderFile=BattViewerDlg.h
ImplementationFile=BattViewerDlg.cpp
Filter=M
BaseClass=CDialog
VirtualFilter=dWC
LastObject=IDC_PROGRESS1

[CLS:CAboutDlg]
Type=0
HeaderFile=BattViewerDlg.h
ImplementationFile=BattViewerDlg.cpp
Filter=D

[DLG:IDD_ABOUTBOX]
Type=1
Class=CAboutDlg
ControlCount=4
Control1=IDC_STATIC,static,1342177283
Control2=IDC_STATIC,static,1342308480
Control3=IDC_STATIC,static,1342308352
Control4=IDOK,button,1342373889

[DLG:IDD_BATTVIEWER_DIALOG]
Type=1
Class=CBattViewerDlg
ControlCount=3
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_PROGRESS1,msctls_progress32,1350565888

[DLG:IDD_ABOUTBOX (âpåÍ (±“ÿ∂))]
Type=1
Class=CAboutDlg
ControlCount=4
Control1=IDC_STATIC,static,1342177283
Control2=IDC_STATIC,static,1342308480
Control3=IDC_STATIC,static,1342308352
Control4=IDOK,button,1342373889

[DLG:IDD_BATTVIEWER_DIALOG (âpåÍ (±“ÿ∂))]
Type=1
Class=CBattViewerDlg
ControlCount=3
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_PROGRESS1,msctls_progress32,1350565888

[CLS:CDisplayThread]
Type=0
HeaderFile=DisplayThread.h
ImplementationFile=DisplayThread.cpp
BaseClass=CWinThread
Filter=N
VirtualFilter=TC

