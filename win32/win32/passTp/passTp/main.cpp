extern "C"
{
#include "NTDDK.h"
}
#include "����.h"
#include "HookNtOpenProcess.h"


void Unload(PDRIVER_OBJECT pDriverObject)
{
	UnHookNtOpenProcess();

	DbgPrint("�����ɹ���ж��\n");
}

extern "C" NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObject,//I\O���������������ں˶���
								PUNICODE_STRING pRegistryPath)//����������ע����е�·��
{ 
// 	PLIST_ENTRY pLE = (PLIST_ENTRY)pDriverObject->DriverSection;      
// 	
// 	//ժ������������      
// 	pLE->Flink->Blink = pLE->Blink;      
// 	pLE->Blink->Flink = pLE->Flink;  
	HookNtOpenProcess();
	
	DbgPrint("�������سɹ�\n");
	
	pDriverObject->DriverUnload=Unload;
	
	return 1;
}

