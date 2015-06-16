#include <ntddk.h>
#define   INITCODE code_seg("INIT")
#pragma INITCODE
VOID DDK_Unload(IN PDRIVER_OBJECT pDriverObject);//前置说明 卸载例程
NTSTATUS DriverEntry(IN PDRIVER_OBJECT pDriveObject,IN PUNICODE_STRING RegisterPath) //typedef long NTSTAUS
{
	CHAR *cstr = "Hello CHAR";
	WCHAR *wstr = "Hello WCHAR";
	/*
	typedef struct _UNICODE_STRING {
			USHORT Length;									 // 字符串的长度（字节数）
			USHORT MaximumLength; 				// 字符串缓冲区的长度（字节数）
			PWSTR Buffer; 									// 字符串缓冲区
	} UNICODE_STRING, *PUNICODE_STRING;
	*/
	UNICODE_STRING usStr;  			//其实一个结构体  应为unicode字符串不是以\0结束的 所以只能使用结构体 里面一个表示其长度的变量
	//所以只能用RtlInitUnicodeString函数初始化
	RtlInitUnicodeString(&usStr,L"Hello Unicode");
	// 	宏KdPrint ，它在发行版不被编译，只在调试版才会运行。
	//		#define KdPrint(_x_) DbgPrint _x_，导致其有两个括号
	//Windows 内核是使用Unicode 编码的
	KdPrint(("驱动成功被加载_____________OK"));   //DgbPrint("Hello DDK");
	KdPrint(("%s",cstr));
	KdPrint(("%S",wstr));
	KdPrint(("%wZ",&usStr));
	pDriveObject->DriverUnload=DDK_Unload;
	return (1);
}
VOID DDK_Unload(IN PDRIVER_OBJECT pDriverObject)
{
	KdPrint(("驱动成功被卸载____________OK")); //sprintf,printf
}
/*
字符串的复制可以使用RtlCopyUnicodeString函数，
字符串的比较可以使用RtlCompareUnicodeString 函数，
字符串转换成大写可以使用RtlUpcaseUnicodeString 函数（没有转换成小写的），
字符串与整数数字互相转换分别可以使用RtlUnicodeStringToInteger 和RtlIntegerToUnicodeString 函数

NTSTATUS 是被定义为32位的无符号长整型。
在驱动程序开发中，人们习惯用 NTSTATUS 返回状态。
其中0~0X7FFFFFFF，被认为是正确的状态，而0X80000000~0XFFFFFFFF被认为是错误的状态。
有一个非常有用的宏-----NT_SUCCESS，用来检测状态是否正确。
*/