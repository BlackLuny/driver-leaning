#include <ntddk.h>

VOID DDK_Unload(IN PDRIVER_OBJECT pDriverObject)
{
	
}
NTSTATUS DriverEntry(IN PDRIVER_OBJECT pDriveObject,IN PUNICODE_STRING RegisterPath) 
{
	OBJECT_ATTRIBUTES obj_attrib;    //为一个结构
	NTSTATUS status;
	IO_STATUS_BLOCK Io_Status_Block;
	HANDLE hFile = NULL;
	UNICODE_STRING usStr; 
	__asm int 3 ;
	RtlInitUnicodeString(&usStr,L"\\??\\c:\\asm\demo.asm");
	//用 Initializeobjectattributes宏	初始化 OBJECT_ATTRIBUTES 这个结构;
	// 初始化文件路径
	InitializeObjectAttributes(&obj_attrib,
												&usStr, // 需要操作的对象、比如文件或注册表路径等
												OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
												NULL,
												NULL);
		// 创建文件
			status = ZwCreateFile(&hFile,					//如果这个函数调用返回成成功(STATUS_SUCCESS),那就么打开的文件句柄就返回在这个地址内
												GENERIC_ALL,			//申请的权限
												&obj_attrib,				//对象描述
												&Io_Status_Block,		//操作的结果
												NULL,
												FILE_ATTRIBUTE_NORMAL,
												FILE_SHARE_READ,			//共享方式
												FILE_CREATE,					//打开方式
												FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
												NULL,
												0 );
		// 写入到目标文件
				status = ZwWriteFile(hFile,				 //打开文件的句柄
				NULL,
				NULL,
			 	NULL,
				&Io_Status_Block,							 //需要自己定义一个该类型变量传入做参数
				usStr.Buffer,									 //数据写入的缓冲区指针
				usStr.Length,									//写入数据的长度
				NULL,
				NULL);
	//关闭文件
	ZwClose(hFile);
	
	pDriveObject->DriverUnload=DDK_Unload;
	return STATUS_SUCCESS;
}
/*

typedef struct _LSA_UNICODE_STRING {
　　USHORT Length;									//指定字符串的长度
　　USHORT MaximumLength;					//Buffer字符串的总长度
　　PWSTR Buffer;									//一个宽字符的字符串的指针
} LSA_UNICODE_STRING, *PLSA_UNICODE_STRING, UNICODE_STRING, *PUNICODE_STRING;

typedef struct _OBJECT_ATTRIBUTES { 
ULONG Length; 												//结构体大小
HANDLE RootDirectory; 								//备选的句柄指向一个根对象目录的路径名称所指定的ObjectName成员
PUNICODE_STRING ObjectName; 					//驱动对象的名称
ULONG Attributes; 											//用来指定一个handle的一些属性的。OBJ_CASE_INSENSITIVE     OBJ_KERNEL_HANDLE 	......
PVOID SecurityDescriptor; 							//指定一个当驱动对象被创建时指定的安全描述(安全_描述符)。如果这个成员是NULL，物体会收到默认安全设置。
PVOID SecurityQualityOfService; 					//可选参数
} OBJECT_ATTRIBUTES, *POBJECT_ATTRIBUTES; 

InitializeObjectAttributes(
     OUT POBJECT_ATTRIBUTES  InitializedAttributes,						//返回文件对象
     IN PUNICODE_STRING  ObjectName,												//磁盘文件路径：\\??\\D:\\file.txt
     IN ULONG  Attributes,																	//设置为：OBJ_CASE_INSENSITIVE|OBJ_KERNEL_HANDLE
     IN HANDLE  RootDirectory,															//NULL
     IN PSECURITY_DESCRIPTOR  SecurityDescriptor						//NULL
     );
 ZwCreateFile(
    OUT PHANDLE  FileHandle,     			 					//返回文件句柄
    IN ACCESS_MASK DesiredAccess,							//文件访问权限设为所有ALL：GENERIC_ALL
    IN POBJECT_ATTRIBUTES ObjectAttributes,			//需要自定义一个文件对象POBJECT_ATTRIBUTES型变量
    OUT PIO_STATUS_BLOCK IoStatusBlock				,//可以使用已经定义的iosb变量
    IN PLARGE_INTEGER AllocationSize  OPTIONAL,//ddk说设置成NULL
    IN ULONG  FileAttributes,										//ddk说设置成FILE_ATTRIBUTES_NORMAL或0即可
    IN ULONG  ShareAccess,											//共享方式设置成只读：FILE_SHARE_READ
    IN ULONG  CreateDisposition,								//打开方式为创建并打开：FILE_OPEN_IF
    IN ULONG  CreateOptions,										//创建选项设置成：FILE_OPEN_IF,FILE_NON_DIRECTORY_FILE|FILE_RANDOM_ACCESS|FILE_SYNCHRONOUS_IO_NONALERT
   IN PVOID  EaBuffer  OPTIONAL,								//NULL
    IN ULONG  EaLength												 //设置成 0
    );//这里标有OPTIONAL的也都是NULL！？
 
ZwWriteFile(
    IN HANDLE  FileHandle,                   //打开文件的句柄
    IN HANDLE  Event  OPTIONAL,              //DDK说该参数应该设置为NULL
    IN PIO_APC_ROUTINE  ApcRoutine  OPTIONAL,//DDK说该参数应该设置为NULL
    IN PVOID  ApcContext  OPTIONAL,          //DDK说该参数应该设置为NULL
    OUT PIO_STATUS_BLOCK IoStatusBlock,      //需要自己定义一个该类型变量传入做参数
    IN PVOID  Buffer,                        //数据写入的缓冲区指针
    IN ULONG  Length,                        //写入数据的长度
    IN PLARGE_INTEGER  ByteOffset  OPTIONAL, //设置为NULL
    IN PULONG  Key  OPTIONAL                 //设置为NULL
    );//看到没有？ddk自己标有OPTIONAL的参数都设置成NULL，这是不是个通用规律？
*/