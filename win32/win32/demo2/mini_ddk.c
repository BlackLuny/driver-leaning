#include <ntddk.h>
#define   INITCODE code_seg("INIT")
#pragma INITCODE
VOID DDK_Unload(IN PDRIVER_OBJECT pDriverObject);//ǰ��˵�� ж������
NTSTATUS DriverEntry(IN PDRIVER_OBJECT pDriveObject,IN PUNICODE_STRING RegisterPath) //typedef long NTSTAUS
{
	CHAR *cstr = "Hello CHAR";
	WCHAR *wstr = "Hello WCHAR";
	/*
	typedef struct _UNICODE_STRING {
			USHORT Length;									 // �ַ����ĳ��ȣ��ֽ�����
			USHORT MaximumLength; 				// �ַ����������ĳ��ȣ��ֽ�����
			PWSTR Buffer; 									// �ַ���������
	} UNICODE_STRING, *PUNICODE_STRING;
	*/
	UNICODE_STRING usStr;  			//��ʵһ���ṹ��  ӦΪunicode�ַ���������\0������ ����ֻ��ʹ�ýṹ�� ����һ����ʾ�䳤�ȵı���
	//����ֻ����RtlInitUnicodeString������ʼ��
	RtlInitUnicodeString(&usStr,L"Hello Unicode");
	// 	��KdPrint �����ڷ��а治�����룬ֻ�ڵ��԰�Ż����С�
	//		#define KdPrint(_x_) DbgPrint _x_������������������
	//Windows �ں���ʹ��Unicode �����
	KdPrint(("�����ɹ�������_____________OK"));   //DgbPrint("Hello DDK");
	KdPrint(("%s",cstr));
	KdPrint(("%S",wstr));
	KdPrint(("%wZ",&usStr));
	pDriveObject->DriverUnload=DDK_Unload;
	return (1);
}
VOID DDK_Unload(IN PDRIVER_OBJECT pDriverObject)
{
	KdPrint(("�����ɹ���ж��____________OK")); //sprintf,printf
}
/*
�ַ����ĸ��ƿ���ʹ��RtlCopyUnicodeString������
�ַ����ıȽϿ���ʹ��RtlCompareUnicodeString ������
�ַ���ת���ɴ�д����ʹ��RtlUpcaseUnicodeString ������û��ת����Сд�ģ���
�ַ������������ֻ���ת���ֱ����ʹ��RtlUnicodeStringToInteger ��RtlIntegerToUnicodeString ����

NTSTATUS �Ǳ�����Ϊ32λ���޷��ų����͡�
���������򿪷��У�����ϰ���� NTSTATUS ����״̬��
����0~0X7FFFFFFF������Ϊ����ȷ��״̬����0X80000000~0XFFFFFFFF����Ϊ�Ǵ����״̬��
��һ���ǳ����õĺ�-----NT_SUCCESS���������״̬�Ƿ���ȷ��
*/