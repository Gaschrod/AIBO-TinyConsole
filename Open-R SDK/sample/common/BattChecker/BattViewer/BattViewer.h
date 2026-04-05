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
// BattViewer.h : main header file for the BATTVIEWER application
//

#if !defined(AFX_BATTVIEWER_H__82F80A45_CF3C_11D7_86A0_0090CC0FC3B5__INCLUDED_)
#define AFX_BATTVIEWER_H__82F80A45_CF3C_11D7_86A0_0090CC0FC3B5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CBattViewerApp:
// See BattViewer.cpp for the implementation of this class
//

class CBattViewerApp : public CWinApp
{
public:
	CBattViewerApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBattViewerApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CBattViewerApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BATTVIEWER_H__82F80A45_CF3C_11D7_86A0_0090CC0FC3B5__INCLUDED_)
