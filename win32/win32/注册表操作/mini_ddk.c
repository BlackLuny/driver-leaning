
#include <ntddk.h>

VOID Unload(IN PDRIVER_OBJECT DriverObject)
{
}

NTSTATUS  DriverEntry(IN PDRIVER_OBJECT DriverObject,IN PUNICODE_STRING RegistryPath)
{
	HANDLE hHandle=NULL;
	NTSTATUS status;
	UNICODE_STRING usPath;
	UNICODE_STRING usValueName;
	UNICODE_STRING usData;
	OBJECT_ATTRIBUTES oa;

	RtlInitUnicodeString(&usPath,L"\\Registry\\Machine\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run");
	RtlInitUnicodeString(&usValueName,L"MyRunTest");
	RtlInitUnicodeString(&usData,L"C:\\MyExe.EXE");

	InitializeObjectAttributes(&oa,//处示化对像
		&usPath,
		OBJ_CASE_INSENSITIVE,
		NULL,NULL
		);

	status=ZwOpenKey(&hHandle,KEY_WRITE,&oa);
	if (NT_SUCCESS(status))
	{
		status=ZwSetValueKey(hHandle,&usValueName,0,REG_SZ,usData.Buffer,usData.Length);
		if (NT_SUCCESS(status))
		{
			ZwClose(hHandle);
			KdPrint(("ZwSetValueKey Success!"));
		} 
		else
		{
			ZwClose(hHandle);
			KdPrint(("ZwSetValueKey Error!"));
		}
	}else
	{
		KdPrint(("ZwOpenKey Error!"));
	}
	DriverObject->DriverUnload = Unload;
	return STATUS_SUCCESS;
}