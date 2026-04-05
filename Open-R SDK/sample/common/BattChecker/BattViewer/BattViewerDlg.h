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
// BattViewerDlg.h : header file
//

#if !defined(AFX_BATTVIEWERDLG_H__82F80A47_CF3C_11D7_86A0_0090CC0FC3B5__INCLUDED_)
#define AFX_BATTVIEWERDLG_H__82F80A47_CF3C_11D7_86A0_0090CC0FC3B5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CBattViewerDlg dialog

class CBattViewerDlg : public CDialog
{
// Construction
public:
	CBattViewerDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CBattViewerDlg)
	enum { IDD = IDD_BATTVIEWER_DIALOG };
	CProgressCtrl	m_battCapacity;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBattViewerDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
public:
protected:
	BOOL UpdateBattCapacity(void);
	HICON m_hIcon;
	LPVOID m_pShrMem1;
	HANDLE m_hMem1;
    HANDLE m_hMutex;
    afx_msg LRESULT LocalUpdate(WPARAM, LPARAM);

	struct BCHK_DATA {
		int cap;
		HWND hWnd;
	};
	union SHARDMEM {
		void *p_v;
		BCHK_DATA *p_d;
	};
	SHARDMEM m_pUnionShrMem;
	// Generated message map functions
	//{{AFX_MSG(CBattViewerDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BATTVIEWERDLG_H__82F80A47_CF3C_11D7_86A0_0090CC0FC3B5__INCLUDED_)
