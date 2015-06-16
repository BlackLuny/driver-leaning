// test_exe.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <windows.h>
#include "ctl_code.h"

int add(HANDLE hDevice, int a,int b)
{
	
	int port[2];
	int bufret;
	ULONG dwWrite;
	port[0]=a;
	port[1]=b;

	DeviceIoControl(hDevice, add_code , &port, 8, &bufret, 4, &dwWrite, NULL);
	return bufret;

}
 
int Hook(HANDLE hDevice, ULONG pid) //pid ��Ҫ����������ID
{
	
	int port[2];
	int bufret;
	ULONG dwWrite;
	port[0]=pid;
 
	
	DeviceIoControl(hDevice, hook_code , &port, 8, &bufret, 4, &dwWrite, NULL);
	return bufret;
	
}
int UnHook(HANDLE hDevice)
{
	
	int port[2];
	int bufret;
	ULONG dwWrite;	
	
	DeviceIoControl(hDevice, unhook_code , &port, 8, &bufret, 4, &dwWrite, NULL);
	return bufret;
	
}

int main(int argc, char* argv[])
{
	//add
	//CreateFile ���豸 ��ȡhDevice
	HANDLE hDevice = 
		CreateFile("\\\\.\\My_DriverLinkName", //\\??\\My_DriverLinkName
		GENERIC_READ | GENERIC_WRITE,
		0,		// share mode none
		NULL,	// no security
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL );		// no template
	printf("start\n");
	if (hDevice == INVALID_HANDLE_VALUE)
	{
		printf("��ȡ�������ʧ��: %s with Win32 error code: %d\n","MyDriver", GetLastError() );
		getchar();
		return -1;
	}

  ULONG pid;
  printf("�������Ҫ�������Ľ���PID=");
  scanf("%d",&pid);
  Hook(hDevice,pid);

 int a=55;
 int b=33;
 int r=add(hDevice,a,b);
  printf("\n %d+%d=%d \n",a,b,r);
  getchar();
	return 0;
}

