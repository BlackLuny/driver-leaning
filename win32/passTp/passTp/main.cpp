extern "C"
{
#include "NTDDK.h"
}
#include "函数.h"
#include "HookNtOpenProcess.h"


void Unload(PDRIVER_OBJECT pDriverObject)
{
	UnHookNtOpenProcess();

	DbgPrint("驱动成功被卸载\n");
}

extern "C" NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObject,//I\O管理器传进来的内核对象
								PUNICODE_STRING pRegistryPath)//驱动程序在注册表中的路径
{ 
// 	PLIST_ENTRY pLE = (PLIST_ENTRY)pDriverObject->DriverSection;      
// 	
// 	//摘链表隐藏驱动      
// 	pLE->Flink->Blink = pLE->Blink;      
// 	pLE->Blink->Flink = pLE->Flink;  
	HookNtOpenProcess();
	
	DbgPrint("驱动加载成功\n");
	
	pDriverObject->DriverUnload=Unload;
	
	return 1;
}

