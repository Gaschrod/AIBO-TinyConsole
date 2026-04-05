//
// Copyright 2003 Sony Corporation 
//
// Permission to use, copy, modify, and redistribute this software for
// non-commercial use is hereby granted.
//
// This software is provided "as is" without warranty of any kind,
// either expressed or implied, including but not limited to the
// implied warranties of fitness for a particular purpose.
//
// BattViewerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "BattViewer.h"
#include "BattViewerDlg.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static const char MapName[]           = "MemMap1";
static const char MutexName[]         = "Mutex1";

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBattViewerDlg dialog

CBattViewerDlg::CBattViewerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CBattViewerDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CBattViewerDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_hMem1 = NULL;
	m_pShrMem1 = NULL;
    m_hMutex = NULL;
}

void CBattViewerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBattViewerDlg)
	DDX_Control(pDX, IDC_PROGRESS1, m_battCapacity);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CBattViewerDlg, CDialog)
	//{{AFX_MSG_MAP(CBattViewerDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_USER + 1, LocalUpdate)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBattViewerDlg message handlers

BOOL CBattViewerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	m_battCapacity.SetRange(0, 100);
	m_battCapacity.SetPos(0);
	m_battCapacity.SetStep(1);

	UpdateBattCapacity();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CBattViewerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CBattViewerDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CBattViewerDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}


BOOL CBattViewerDlg::UpdateBattCapacity()
{
	int capacity;
	if (!(m_hMutex = CreateMutex(NULL, FALSE, MutexName))) {
		return FALSE;
	}
	if (!m_pShrMem1) {
        HANDLE MapFileHandle = (HANDLE) 0xFFFFFFFF;
 		WaitForSingleObject(m_hMutex, INFINITE);
		if (!(m_hMem1 = CreateFileMapping(MapFileHandle, NULL, PAGE_READWRITE, 0, 4096, MapName))) {
			ReleaseMutex(m_hMutex);
			return FALSE;
		}
		if (!(m_pShrMem1 = MapViewOfFile(m_hMem1, FILE_MAP_WRITE | FILE_MAP_READ, 0, 0, 4096))) {
			ReleaseMutex(m_hMutex);
			return FALSE;
		}
		m_pUnionShrMem.p_v = m_pShrMem1;
		m_pUnionShrMem.p_d->hWnd = GetSafeHwnd();
		m_pUnionShrMem.p_d->cap = 0;
		ReleaseMutex(m_hMutex);
	}
	WaitForSingleObject(m_hMutex, INFINITE);
	capacity = m_pUnionShrMem.p_d->cap;
	if (!m_pUnionShrMem.p_d->hWnd)
		m_pUnionShrMem.p_d->hWnd = GetSafeHwnd();
	ReleaseMutex(m_hMutex);
	m_battCapacity.SetPos(capacity);
	return TRUE;
}

LRESULT CBattViewerDlg::LocalUpdate(WPARAM a, LPARAM b)
{
	// Windows message (WM_USER + 1) comes from BattChecker program
	UpdateBattCapacity();
	return 0;
}
