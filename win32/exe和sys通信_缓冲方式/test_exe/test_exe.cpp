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
	
	/*
BOOL WINAPI DeviceIoControl(
__in        HANDLE hDevice, 设备句柄
__in        DWORD dwIoControlCode,应用程序调用驱动程序的控制命令，就是IOCTL_XXX IOCTLs
__in_opt    LPVOID lpInBuffer,应用程序传递给驱动程序的数据缓冲区地址
__in        DWORD nInBufferSize,应用程序传递给驱动程序的数据缓冲区大小，字节数
__out_opt   LPVOID lpOutBuffer,驱动程序返回给应用程序的数据缓冲区地址
__in        DWORD nOutBufferSize,驱动程序返回给应用程序的数据缓冲区大小，字节数
__out_opt   LPDWORD lpBytesReturned,驱动程序实际返回给应用程序的数据字节数地址
__inout_opt LPOVERLAPPED lpOverlapped这个结构用于重叠操作。针对同步操作，请用ByVal As Long传递零值
)
	*/
	DeviceIoControl(hDevice, add_code , &port, 8, &bufret, 4, &dwWrite, NULL);
	
	return bufret;

}

int main(int argc, char* argv[])
{
	//add
	//CreateFile 打开设备 获取hDevice
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
		printf("获取驱动句柄失败: %s with Win32 error code: %d\n","MyDriver", GetLastError() );
		getchar();
		return -1;
	}
	int a=55;
	int b=33;
  	int r=add(hDevice,a,b);
  	printf("%d+%d=%d \n",a,b,r);
  	getchar();
	return 0;
}

