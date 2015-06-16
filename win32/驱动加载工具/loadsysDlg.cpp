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
	CFileDialog sysFile(true,NULL,NULL,0,"驱动文件sys|*.sys|所有文件|*.*|");
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
//   	A、OpenSCManager
       SC_HANDLE hServiceMgr=OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);
	   if (hServiceMgr==NULL)
	   {
		   TRACE("OpenSCManager 调用失败");
		   goto BExit;
	   }
//		B、CreateService
   hServiceDDK= CreateService( hServiceMgr,//SCM管理器句柄 
		   lpszDriverName, //驱动程序的在注册表中的名字  
		   lpszDriverName, // 注册表驱动程序的 DisplayName 值  
		   SERVICE_ALL_ACCESS, // 加载驱动程序的访问权限  
		   SERVICE_KERNEL_DRIVER,// 表示加载的服务是驱动程序  
		   SERVICE_DEMAND_START, // 注册表驱动程序的 Start 值  
		   SERVICE_ERROR_IGNORE, // 注册表驱动程序的 ErrorControl 值  
		   lpszDriverPathName, // 注册表驱动程序的 ImagePath 值  
		   NULL,              //要开启服务的 用户组
		   NULL,  //输出验证标签
		   NULL,   //所依赖的服务的名称
		   NULL,   //用户账户名称
		   NULL);  //用户口令
//		C、OpenService
   if (hServiceDDK==NULL)
   {   TRACE("CreateService 失败,继续调用OpenService");
	   hServiceDDK=OpenService(hServiceMgr,lpszDriverName,SERVICE_ALL_ACCESS);
      if (hServiceDDK==NULL)
	  {   TRACE("OpenService 失败");
		  goto BExit;
	  }
   }
//		D、StartService
   StartService(hServiceDDK,NULL,NULL);
   
//      E、CloseServiceHandle
BExit:
	//CloseServiceHandle
	//CloseServiceHandle
	   return bRet;
} */

BOOL LoadNTDriver(char* lpDriverName,char* lpDriverPathName)
{
	BOOL bRet = FALSE;
	
	SC_HANDLE hServiceMgr=NULL;//SCM管理器的句柄
	SC_HANDLE hServiceDDK=NULL;//NT驱动程序的服务句柄
	
	//打开服务控制管理器
	hServiceMgr = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS );
	
	if( hServiceMgr == NULL )  
	{
		//OpenSCManager失败
		TRACE( "OpenSCManager() Faild %d ! \n", GetLastError() );
		bRet = FALSE;
		goto BExit;
	}
	else
	{
		////OpenSCManager成功
		TRACE( "OpenSCManager() ok ! \n" );  
	}
	
	//创建驱动所对应的服务
	hServiceDDK = CreateService( hServiceMgr,
		lpDriverName, //驱动程序的在注册表中的名字  
		lpDriverName, // 注册表驱动程序的 DisplayName 值  
		SERVICE_ALL_ACCESS, // 加载驱动程序的访问权限  
		SERVICE_KERNEL_DRIVER,// 表示加载的服务是驱动程序  
		SERVICE_DEMAND_START, // 注册表驱动程序的 Start 值  
		SERVICE_ERROR_IGNORE, // 注册表驱动程序的 ErrorControl 值  
		lpDriverPathName, // 注册表驱动程序的 ImagePath 值  
		NULL,  
		NULL,  
		NULL,  
		NULL,  
		NULL);  
	
	DWORD dwRtn;
	//判断服务是否失败
	if( hServiceDDK == NULL )  
	{  
		dwRtn = GetLastError();
		if( dwRtn != ERROR_IO_PENDING && dwRtn != ERROR_SERVICE_EXISTS )  
		{  
			//由于其他原因创建服务失败
			TRACE( "CrateService() 失败 %d ! \n", dwRtn );  
			bRet = FALSE;
			goto BExit;
		}  
		else  
		{
			//服务创建失败，是由于服务已经创立过
			TRACE( "CrateService() 服务创建失败，是由于服务已经创立过 ERROR is ERROR_IO_PENDING or ERROR_SERVICE_EXISTS! \n" );  
		}
		
		// 驱动程序已经加载，只需要打开  
		hServiceDDK = OpenService( hServiceMgr, lpDriverName, SERVICE_ALL_ACCESS );  
		if( hServiceDDK == NULL )  
		{
			//如果打开服务也失败，则意味错误
			dwRtn = GetLastError();  
			TRACE( "OpenService() 失败 %d ! \n", dwRtn );  
			bRet = FALSE;
			goto BExit;
		}  
		else 
		{
			TRACE( "OpenService() 成功 ! \n" );
		}
	}  
	else  
	{
		TRACE( "CrateService() 成功 ! \n" );
	}
	
	//开启此项服务
	bRet= StartService( hServiceDDK, NULL, NULL );  
	if( !bRet )  //开启服务不成功
	{  
		TRACE( "StartService() 失败 服务可能已经开启%d ! \n", dwRtn );  
	}
	bRet = TRUE;
	//离开前关闭句柄
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
//卸载驱动程序  
BOOL UnLoadSys( char * szSvrName )  
{
	//一定义所用到的变量
	BOOL bRet = FALSE;
	SC_HANDLE hSCM=NULL;//SCM管理器的句柄,用来存放OpenSCManager的返回值
	SC_HANDLE hService=NULL;//NT驱动程序的服务句柄，用来存放OpenService的返回值
	SERVICE_STATUS SvrSta;
	//二打开SCM管理器
	hSCM = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS );  
	if( hSCM == NULL )  
	{
		//带开SCM管理器失败
		TRACE( "OpenSCManager() Faild %d ! \n", GetLastError() );  
		bRet = FALSE;
		goto BeforeLeave;
	}  
	else  
	{
		//打开SCM管理器成功
		TRACE( "OpenSCManager() ok ! \n" );  
	}
	//三打开驱动所对应的服务
	hService = OpenService( hSCM, szSvrName, SERVICE_ALL_ACCESS );  
	
	if( hService == NULL )  
	{
		//打开驱动所对应的服务失败 退出
		TRACE( "OpenService() Faild %d ! \n", GetLastError() );  
		bRet = FALSE;
		goto BeforeLeave;
	}  
	else  
	{  
		TRACE( "OpenService() ok ! \n" );  //打开驱动所对应的服务 成功
	}  
	//四停止驱动程序，如果停止失败，只有重新启动才能，再动态加载。  
	if( !ControlService( hService, SERVICE_CONTROL_STOP , &SvrSta ) )  
	{  
		TRACE( "用ControlService() 停止驱动程序失败 错误号:%d !\n", GetLastError() );  
	}  
	else  
	{
		//停止驱动程序成功
		TRACE( "用ControlService() 停止驱动程序成功 !\n" );  
	}  
	//五动态卸载驱动服务。  
	if( !DeleteService( hService ) )  //TRUE//FALSE
	{
		//卸载失败
		TRACE( "卸载失败:DeleteSrevice()错误号:%d !\n", GetLastError() );  
	}  
	else  
	{  
		//卸载成功
		TRACE ( "卸载成功 !\n" );  
		
	}  
	bRet = TRUE;
	//六 离开前关闭打开的句柄
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
	//卸载驱动程序  
  UnLoadSys(DriverName.GetBuffer(256))  ;
}
