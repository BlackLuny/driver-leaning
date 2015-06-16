// test_API.h : main header file for the TEST_API application
//

#if !defined(AFX_TEST_API_H__2B31E195_E694_46BB_B9EB_B50D93266880__INCLUDED_)
#define AFX_TEST_API_H__2B31E195_E694_46BB_B9EB_B50D93266880__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CTest_APIApp:
// See test_API.cpp for the implementation of this class
//

class CTest_APIApp : public CWinApp
{
public:
	CTest_APIApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTest_APIApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CTest_APIApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TEST_API_H__2B31E195_E694_46BB_B9EB_B50D93266880__INCLUDED_)
