#include <ntddk.h>

VOID DDK_Unload(IN PDRIVER_OBJECT pDriverObject)
{
	
}
NTSTATUS DriverEntry(IN PDRIVER_OBJECT pDriveObject,IN PUNICODE_STRING RegisterPath) 
{
	OBJECT_ATTRIBUTES obj_attrib;    //Ϊһ���ṹ
	NTSTATUS status;
	IO_STATUS_BLOCK Io_Status_Block;
	HANDLE hFile = NULL;
	UNICODE_STRING usStr; 
	__asm int 3 ;
	RtlInitUnicodeString(&usStr,L"\\??\\c:\\asm\demo.asm");
	//�� Initializeobjectattributes��	��ʼ�� OBJECT_ATTRIBUTES ����ṹ;
	// ��ʼ���ļ�·��
	InitializeObjectAttributes(&obj_attrib,
												&usStr, // ��Ҫ�����Ķ��󡢱����ļ���ע���·����
												OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
												NULL,
												NULL);
		// �����ļ�
			status = ZwCreateFile(&hFile,					//�������������÷��سɳɹ�(STATUS_SUCCESS),�Ǿ�ô�򿪵��ļ�����ͷ����������ַ��
												GENERIC_ALL,			//�����Ȩ��
												&obj_attrib,				//��������
												&Io_Status_Block,		//�����Ľ��
												NULL,
												FILE_ATTRIBUTE_NORMAL,
												FILE_SHARE_READ,			//����ʽ
												FILE_CREATE,					//�򿪷�ʽ
												FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
												NULL,
												0 );
		// д�뵽Ŀ���ļ�
				status = ZwWriteFile(hFile,				 //���ļ��ľ��
				NULL,
				NULL,
			 	NULL,
				&Io_Status_Block,							 //��Ҫ�Լ�����һ�������ͱ�������������
				usStr.Buffer,									 //����д��Ļ�����ָ��
				usStr.Length,									//д�����ݵĳ���
				NULL,
				NULL);
	//�ر��ļ�
	ZwClose(hFile);
	
	pDriveObject->DriverUnload=DDK_Unload;
	return STATUS_SUCCESS;
}
/*

typedef struct _LSA_UNICODE_STRING {
����USHORT Length;									//ָ���ַ����ĳ���
����USHORT MaximumLength;					//Buffer�ַ������ܳ���
����PWSTR Buffer;									//һ�����ַ����ַ�����ָ��
} LSA_UNICODE_STRING, *PLSA_UNICODE_STRING, UNICODE_STRING, *PUNICODE_STRING;

typedef struct _OBJECT_ATTRIBUTES { 
ULONG Length; 												//�ṹ���С
HANDLE RootDirectory; 								//��ѡ�ľ��ָ��һ��������Ŀ¼��·��������ָ����ObjectName��Ա
PUNICODE_STRING ObjectName; 					//�������������
ULONG Attributes; 											//����ָ��һ��handle��һЩ���Եġ�OBJ_CASE_INSENSITIVE     OBJ_KERNEL_HANDLE 	......
PVOID SecurityDescriptor; 							//ָ��һ�����������󱻴���ʱָ���İ�ȫ����(��ȫ_������)����������Ա��NULL��������յ�Ĭ�ϰ�ȫ���á�
PVOID SecurityQualityOfService; 					//��ѡ����
} OBJECT_ATTRIBUTES, *POBJECT_ATTRIBUTES; 

InitializeObjectAttributes(
     OUT POBJECT_ATTRIBUTES  InitializedAttributes,						//�����ļ�����
     IN PUNICODE_STRING  ObjectName,												//�����ļ�·����\\??\\D:\\file.txt
     IN ULONG  Attributes,																	//����Ϊ��OBJ_CASE_INSENSITIVE|OBJ_KERNEL_HANDLE
     IN HANDLE  RootDirectory,															//NULL
     IN PSECURITY_DESCRIPTOR  SecurityDescriptor						//NULL
     );
 ZwCreateFile(
    OUT PHANDLE  FileHandle,     			 					//�����ļ����
    IN ACCESS_MASK DesiredAccess,							//�ļ�����Ȩ����Ϊ����ALL��GENERIC_ALL
    IN POBJECT_ATTRIBUTES ObjectAttributes,			//��Ҫ�Զ���һ���ļ�����POBJECT_ATTRIBUTES�ͱ���
    OUT PIO_STATUS_BLOCK IoStatusBlock				,//����ʹ���Ѿ������iosb����
    IN PLARGE_INTEGER AllocationSize  OPTIONAL,//ddk˵���ó�NULL
    IN ULONG  FileAttributes,										//ddk˵���ó�FILE_ATTRIBUTES_NORMAL��0����
    IN ULONG  ShareAccess,											//����ʽ���ó�ֻ����FILE_SHARE_READ
    IN ULONG  CreateDisposition,								//�򿪷�ʽΪ�������򿪣�FILE_OPEN_IF
    IN ULONG  CreateOptions,										//����ѡ�����óɣ�FILE_OPEN_IF,FILE_NON_DIRECTORY_FILE|FILE_RANDOM_ACCESS|FILE_SYNCHRONOUS_IO_NONALERT
   IN PVOID  EaBuffer  OPTIONAL,								//NULL
    IN ULONG  EaLength												 //���ó� 0
    );//�������OPTIONAL��Ҳ����NULL����
 
ZwWriteFile(
    IN HANDLE  FileHandle,                   //���ļ��ľ��
    IN HANDLE  Event  OPTIONAL,              //DDK˵�ò���Ӧ������ΪNULL
    IN PIO_APC_ROUTINE  ApcRoutine  OPTIONAL,//DDK˵�ò���Ӧ������ΪNULL
    IN PVOID  ApcContext  OPTIONAL,          //DDK˵�ò���Ӧ������ΪNULL
    OUT PIO_STATUS_BLOCK IoStatusBlock,      //��Ҫ�Լ�����һ�������ͱ�������������
    IN PVOID  Buffer,                        //����д��Ļ�����ָ��
    IN ULONG  Length,                        //д�����ݵĳ���
    IN PLARGE_INTEGER  ByteOffset  OPTIONAL, //����ΪNULL
    IN PULONG  Key  OPTIONAL                 //����ΪNULL
    );//����û�У�ddk�Լ�����OPTIONAL�Ĳ��������ó�NULL�����ǲ��Ǹ�ͨ�ù��ɣ�
*/