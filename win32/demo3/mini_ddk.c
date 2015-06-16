#include <ntddk.h>

VOID DDK_Unload(IN PDRIVER_OBJECT pDriverObject)
{
	
}
NTSTATUS DriverEntry(IN PDRIVER_OBJECT pDriveObject,IN PUNICODE_STRING RegisterPath) 
{
	OBJECT_ATTRIBUTES obj_attrib;    //为一个结构
	UNICODE_STRING usStr; 
	RtlInitUnicodeString(&usStr,L"\\??\\c:\\demo.asm");
	//用 Initializeobjectattributes 宏初始化 OBJECT_ATTRIBUTES 这个结构;
	InitializeObjectAttributes(&obj_attrib,
												&usStr, // 需要操作的对象、比如文件或注册表路径等
												OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
												NULL,
												NULL);
	pDriveObject->DriverUnload=DDK_Unload;
	return STATUS_SUCCESS;
}
/*
typedef struct _OBJECT_ATTRIBUTES { 
ULONG Length; 
HANDLE RootDirectory; 
PUNICODE_STRING ObjectName; 
ULONG Attributes; 
PVOID SecurityDescriptor; 
PVOID SecurityQualityOfService; 
} OBJECT_ATTRIBUTES, *POBJECT_ATTRIBUTES; 
Length：结构体大小
RootDirectory：备选的句柄指向一个根对象目录的路径名称所指定的ObjectName成员。
ObjectName: 驱动对象的名称
Attributes   : 用来指定一个handle的一些属性的。OBJ_CASE_INSENSITIVE     OBJ_KERNEL_HANDLE 	......
SecurityDescriptor:指定一个当驱动对象被创建时指定的安全描述(安全_描述符)。如果这个成员是NULL，物体会收到默认安全设置。
SecurityQualityOfService：可选参数

InitializeObjectAttributes( 
    OUT POBJECT_ATTRIBUTES InitializedAttributes , 
    IN PUNICODE_STRING ObjectName , 
    IN ULONG Attributes , 
    IN HANDLE RootDirectory , 
    IN PSECURITY_DESCRIPTOR SecurityDescriptor 
    );

*/