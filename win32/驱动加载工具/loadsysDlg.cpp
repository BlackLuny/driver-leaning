// loadsysDlg.cpp : implementation file
//

#include "stdafx.h"
#include "loadsys.h"
#include "loadsysDlg.h"
#include <winsvc.h> 
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
BOOL LoadNTDriver(char* lpDriverName,char* lpDriverPathName);
BOOL UnLoadSys( char * szSvrName );
/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About
CString DriverName;

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
// CLoadsysDlg dialog

CLoadsysDlg::CLoadsysDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CLoadsysDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLoadsysDlg)
	m_syspathname = _T("");
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CLoadsysDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLoadsysDlg)
	DDX_Text(pDX, IDC_EDIT_SYSPATHNAME, m_syspathname);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CLoadsysDlg, CDialog)
	//{{AFX_MSG_MAP(CLoadsysDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_LOADSYS, OnButtonLoadsys)
	ON_BN_CLICKED(IDC_BUTTON_UNLOADSYS, OnButtonUnloadsys)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLoadsysDlg message handlers

BOOL CLoadsysDlg::OnInitDialog()
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
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CLoadsysDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CLoadsysDlg::OnPaint() 
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
HCURSOR CLoadsysDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CLoadsysDlg::OnButtonLoadsys() 
{
	// TODO: Add your control notification handler code here
	CFileDialog sysFile(true,NULL,NULL,0,"�����ļ�sys|*.sys|�����ļ�|*.*|");
    if (IDOK ==sysFile.DoModal())
	{
	 	m_syspathname=sysFile.GetPathName();
		m_syspathname=sysFile.GetFileName();
		DriverName=sysFile.GetFileName();
		UpdateData(false);
		//LoadNtDriver;
        LoadNTDriver(sysFile.GetFileName().GetBuffer(256),sysFile.GetPathName().GetBuffer(256));
	}

}
/*
BOOL LoadNTDriver(char* lpszDriverName, char* lpszDriverPathName)
{
	BOOL bRet=false;
	SC_HANDLE hServiceDDK=NULL;
//   	A��OpenSCManager
       SC_HANDLE hServiceMgr=OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);
	   if (hServiceMgr==NULL)
	   {
		   TRACE("OpenSCManager ����ʧ��");
		   goto BExit;
	   }
//		B��CreateService
   hServiceDDK= CreateService( hServiceMgr,//SCM��������� 
		   lpszDriverName, //�����������ע����е�����  
		   lpszDriverName, // ע������������ DisplayName ֵ  
		   SERVICE_ALL_ACCESS, // ������������ķ���Ȩ��  
		   SERVICE_KERNEL_DRIVER,// ��ʾ���صķ�������������  
		   SERVICE_DEMAND_START, // ע������������ Start ֵ  
		   SERVICE_ERROR_IGNORE, // ע������������ ErrorControl ֵ  
		   lpszDriverPathName, // ע������������ ImagePath ֵ  
		   NULL,              //Ҫ��������� �û���
		   NULL,  //�����֤��ǩ
		   NULL,   //�������ķ��������
		   NULL,   //�û��˻�����
		   NULL);  //�û�����
//		C��OpenService
   if (hServiceDDK==NULL)
   {   TRACE("CreateService ʧ��,��������OpenService");
	   hServiceDDK=OpenService(hServiceMgr,lpszDriverName,SERVICE_ALL_ACCESS);
      if (hServiceDDK==NULL)
	  {   TRACE("OpenService ʧ��");
		  goto BExit;
	  }
   }
//		D��StartService
   StartService(hServiceDDK,NULL,NULL);
   
//      E��CloseServiceHandle
BExit:
	//CloseServiceHandle
	//CloseServiceHandle
	   return bRet;
} */

BOOL LoadNTDriver(char* lpDriverName,char* lpDriverPathName)
{
	BOOL bRet = FALSE;
	
	SC_HANDLE hServiceMgr=NULL;//SCM�������ľ��
	SC_HANDLE hServiceDDK=NULL;//NT��������ķ�����
	
	//�򿪷�����ƹ�����
	hServiceMgr = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS );
	
	if( hServiceMgr == NULL )  
	{
		//OpenSCManagerʧ��
		TRACE( "OpenSCManager() Faild %d ! \n", GetLastError() );
		bRet = FALSE;
		goto BExit;
	}
	else
	{
		////OpenSCManager�ɹ�
		TRACE( "OpenSCManager() ok ! \n" );  
	}
	
	//������������Ӧ�ķ���
	hServiceDDK = CreateService( hServiceMgr,
		lpDriverName, //�����������ע����е�����  
		lpDriverName, // ע������������ DisplayName ֵ  
		SERVICE_ALL_ACCESS, // ������������ķ���Ȩ��  
		SERVICE_KERNEL_DRIVER,// ��ʾ���صķ�������������  
		SERVICE_DEMAND_START, // ע������������ Start ֵ  
		SERVICE_ERROR_IGNORE, // ע������������ ErrorControl ֵ  
		lpDriverPathName, // ע������������ ImagePath ֵ  
		NULL,  
		NULL,  
		NULL,  
		NULL,  
		NULL);  
	
	DWORD dwRtn;
	//�жϷ����Ƿ�ʧ��
	if( hServiceDDK == NULL )  
	{  
		dwRtn = GetLastError();
		if( dwRtn != ERROR_IO_PENDING && dwRtn != ERROR_SERVICE_EXISTS )  
		{  
			//��������ԭ�򴴽�����ʧ��
			TRACE( "CrateService() ʧ�� %d ! \n", dwRtn );  
			bRet = FALSE;
			goto BExit;
		}  
		else  
		{
			//���񴴽�ʧ�ܣ������ڷ����Ѿ�������
			TRACE( "CrateService() ���񴴽�ʧ�ܣ������ڷ����Ѿ������� ERROR is ERROR_IO_PENDING or ERROR_SERVICE_EXISTS! \n" );  
		}
		
		// ���������Ѿ����أ�ֻ��Ҫ��  
		hServiceDDK = OpenService( hServiceMgr, lpDriverName, SERVICE_ALL_ACCESS );  
		if( hServiceDDK == NULL )  
		{
			//����򿪷���Ҳʧ�ܣ�����ζ����
			dwRtn = GetLastError();  
			TRACE( "OpenService() ʧ�� %d ! \n", dwRtn );  
			bRet = FALSE;
			goto BExit;
		}  
		else 
		{
			TRACE( "OpenService() �ɹ� ! \n" );
		}
	}  
	else  
	{
		TRACE( "CrateService() �ɹ� ! \n" );
	}
	
	//�����������
	bRet= StartService( hServiceDDK, NULL, NULL );  
	if( !bRet )  //�������񲻳ɹ�
	{  
		TRACE( "StartService() ʧ�� ��������Ѿ�����%d ! \n", dwRtn );  
	}
	bRet = TRUE;
	//�뿪ǰ�رվ��
BExit:
	if(hServiceDDK)
	{
		CloseServiceHandle(hServiceDDK);
	}
	if(hServiceMgr)
	{
		CloseServiceHandle(hServiceMgr);
	}
	return bRet;
}
//ж����������  
BOOL UnLoadSys( char * szSvrName )  
{
	//һ�������õ��ı���
	BOOL bRet = FALSE;
	SC_HANDLE hSCM=NULL;//SCM�������ľ��,�������OpenSCManager�ķ���ֵ
	SC_HANDLE hService=NULL;//NT��������ķ��������������OpenService�ķ���ֵ
	SERVICE_STATUS SvrSta;
	//����SCM������
	hSCM = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS );  
	if( hSCM == NULL )  
	{
		//����SCM������ʧ��
		TRACE( "OpenSCManager() Faild %d ! \n", GetLastError() );  
		bRet = FALSE;
		goto BeforeLeave;
	}  
	else  
	{
		//��SCM�������ɹ�
		TRACE( "OpenSCManager() ok ! \n" );  
	}
	//������������Ӧ�ķ���
	hService = OpenService( hSCM, szSvrName, SERVICE_ALL_ACCESS );  
	
	if( hService == NULL )  
	{
		//����������Ӧ�ķ���ʧ�� �˳�
		TRACE( "OpenService() Faild %d ! \n", GetLastError() );  
		bRet = FALSE;
		goto BeforeLeave;
	}  
	else  
	{  
		TRACE( "OpenService() ok ! \n" );  //����������Ӧ�ķ��� �ɹ�
	}  
	//��ֹͣ�����������ֹͣʧ�ܣ�ֻ�������������ܣ��ٶ�̬���ء�  
	if( !ControlService( hService, SERVICE_CONTROL_STOP , &SvrSta ) )  
	{  
		TRACE( "��ControlService() ֹͣ��������ʧ�� �����:%d !\n", GetLastError() );  
	}  
	else  
	{
		//ֹͣ��������ɹ�
		TRACE( "��ControlService() ֹͣ��������ɹ� !\n" );  
	}  
	//�嶯̬ж����������  
	if( !DeleteService( hService ) )  //TRUE//FALSE
	{
		//ж��ʧ��
		TRACE( "ж��ʧ��:DeleteSrevice()�����:%d !\n", GetLastError() );  
	}  
	else  
	{  
		//ж�سɹ�
		TRACE ( "ж�سɹ� !\n" );  
		
	}  
	bRet = TRUE;
	//�� �뿪ǰ�رմ򿪵ľ��
BeforeLeave:
	if(hService>0)
	{
		CloseServiceHandle(hService);
	}
	if(hSCM>0)
	{
		CloseServiceHandle(hSCM);
	}
	return bRet;	
} 

void CLoadsysDlg::OnButtonUnloadsys() 
{
	// TODO: Add your control notification handler code here
	//ж����������  
  UnLoadSys(DriverName.GetBuffer(256))  ;
}
