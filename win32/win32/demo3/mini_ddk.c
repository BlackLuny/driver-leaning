#include <ntddk.h>

VOID DDK_Unload(IN PDRIVER_OBJECT pDriverObject)
{
	
}
NTSTATUS DriverEntry(IN PDRIVER_OBJECT pDriveObject,IN PUNICODE_STRING RegisterPath) 
{
	OBJECT_ATTRIBUTES obj_attrib;    //Ϊһ���ṹ
	UNICODE_STRING usStr; 
	RtlInitUnicodeString(&usStr,L"\\??\\c:\\demo.asm");
	//�� Initializeobjectattributes ���ʼ�� OBJECT_ATTRIBUTES ����ṹ;
	InitializeObjectAttributes(&obj_attrib,
												&usStr, // ��Ҫ�����Ķ��󡢱����ļ���ע���·����
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
Length���ṹ���С
RootDirectory����ѡ�ľ��ָ��һ��������Ŀ¼��·��������ָ����ObjectName��Ա��
ObjectName: �������������
Attributes   : ����ָ��һ��handle��һЩ���Եġ�OBJ_CASE_INSENSITIVE     OBJ_KERNEL_HANDLE 	......
SecurityDescriptor:ָ��һ�����������󱻴���ʱָ���İ�ȫ����(��ȫ_������)����������Ա��NULL��������յ�Ĭ�ϰ�ȫ���á�
SecurityQualityOfService����ѡ����

InitializeObjectAttributes( 
    OUT POBJECT_ATTRIBUTES InitializedAttributes , 
    IN PUNICODE_STRING ObjectName , 
    IN ULONG Attributes , 
    IN HANDLE RootDirectory , 
    IN PSECURITY_DESCRIPTOR SecurityDescriptor 
    );

*/