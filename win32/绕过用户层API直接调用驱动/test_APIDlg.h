// test_APIDlg.h : header file
//

#if !defined(AFX_TEST_APIDLG_H__224B2A28_82B2_41AD_AD5E_1B1173D28093__INCLUDED_)
#define AFX_TEST_APIDLG_H__224B2A28_82B2_41AD_AD5E_1B1173D28093__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CTest_APIDlg dialog

class CTest_APIDlg : public CDialog
{
// Construction
public:
	CTest_APIDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CTest_APIDlg)
	enum { IDD = IDD_TEST_API_DIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTest_APIDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CTest_APIDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnBUTTONsysfind();
	afx_msg void OnButtonMyFindwindow();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TEST_APIDLG_H__224B2A28_82B2_41AD_AD5E_1B1173D28093__INCLUDED_)
