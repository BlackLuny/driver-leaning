//----------------------www.yjxsoft.com------------------�������Ἴ��
//�ļ��� "38.h"
#define BUFFER_SIZE 1024

#pragma INITCODE
VOID CharTest()
{  
	KdPrint(("Char�ִ�����--Start \n"));

	PCHAR s1="abc11";        //CHAR ANSI
	KdPrint(("%x,%s\n",s1,s1)); 
	WCHAR* s2=L"abc11";     //WCHAR UNICODE //PWSTR
	KdPrint(("%x,%S\n",s2,s2));

	KdPrint(("Char�ִ�����--End \n"));
	_asm int 3

}

//ANSI_STRING
//UNICODE_STRING
//�ַ�����ʼ������
#pragma INITCODE
VOID StringInitTest() 
{
	KdPrint(("��ʼ���ִ�����--Start \n"));
	ANSI_STRING  AnsiString1={0};
	UNICODE_STRING UnicodeString1={0};
    // AnsiString1.Buffer="AnsiString1�ַ���";
	AnsiString1.Buffer=(PCHAR)ExAllocatePool(PagedPool,BUFFER_SIZE);
	strcpy(AnsiString1.Buffer,"AnsiString1�ַ���");

	AnsiString1.Length=strlen( AnsiString1.Buffer);
	AnsiString1.MaximumLength=BUFFER_SIZE;

	//UnicodeString1.Buffer =  L"3333333333333333333322222222221111112";
	UnicodeString1.Buffer=(PWSTR)ExAllocatePool(PagedPool,BUFFER_SIZE);
	wcscpy(UnicodeString1.Buffer,L"3333333333333333333322222222221111112");
 
	UnicodeString1.Length=wcslen(UnicodeString1.Buffer)*2-1;
	UnicodeString1.MaximumLength = BUFFER_SIZE;

	//����2 ��RTL������ʼ��
	//��ʼ��ANSI_STRING�ַ���
	//RtlInitAnsiString(&AnsiString1,"AnsiString1�ַ���");	 
	//RtlInitUnicodeString(&UnicodeString1,L"3333333333333333333322222222221111112");


	//��ӡASCII�ַ��� %Z
	KdPrint(("%x AnsiString1:   %Z\n",&AnsiString1,   &AnsiString1));
	//��ӡUNICODE�ַ��� %wZ �����ַ��ᱻ�ض�
	KdPrint(("%x UnicodeString1:%wZ\n",&UnicodeString1,&UnicodeString1));
	KdPrint(("��ʼ���ִ�����--END \n")); 
	//__asm int 3
	//�ͷ��ִ�
	RtlFreeAnsiString(&AnsiString1);//ExFreePool()
	RtlFreeUnicodeString(&UnicodeString1);

}//D���ַ������ƣ��Ƚϣ�����Сд���������ִ����໥ת��

#pragma INITCODE
VOID StringCopyTest() 
{ 
	KdPrint(("\n-------------------�ִ����Ʋ��Կ�ʼ---------------D \n"));
	//��ʼ��UnicodeString1
	UNICODE_STRING UnicodeString1;
	RtlInitUnicodeString(&UnicodeString1,L"UnicodeString1");

	//��ʼ��UnicodeString2
	UNICODE_STRING UnicodeString2={0};
	UnicodeString2.Buffer = (PWSTR)ExAllocatePool(PagedPool,BUFFER_SIZE);
	UnicodeString2.MaximumLength = BUFFER_SIZE;
    __asm int 3
	//����ʼ��UnicodeString1������UnicodeString2
	RtlCopyUnicodeString(&UnicodeString2,&UnicodeString1);//UnicodeString2=UnicodeString1

	//�ֱ���ʾUnicodeString1��UnicodeString2
	KdPrint(("�ִ�1:%wZ\n",&UnicodeString1));
	KdPrint(("�ִ�2:%wZ\n",&UnicodeString2));
   if ( RtlEqualUnicodeString(&UnicodeString1,&UnicodeString2,true))
   { KdPrint(("2�� �ִ���� ��\n"));}
	//����UnicodeString2
	//ע��!���ù�RtlInitUnicodeString ��ʼ�����ִ�!UnicodeString1��������, 

	//RtlFreeUnicodeString(&UnicodeString1); ���ϴ��л�����
	RtlFreeUnicodeString(&UnicodeString2);
	KdPrint(("\n-------------------�ִ����Ʋ��Խ���--------------- \n"));

}


//�ַ������д����
#pragma INITCODE
VOID StringToUpperTest() 
{  
	KdPrint(("\n-------------------�ִ�ת��д���� ��ʼ--------------- \n"));
	//��ʼ��UnicodeString1
	UNICODE_STRING UnicodeString1;
	RtlInitUnicodeString(&UnicodeString1,L"UnicodeString1��Aabc");

	//�仯ǰ
	KdPrint(("UnicodeString1 ��ֵ:%wZ\n",&UnicodeString1));

	//���д
	RtlUpcaseUnicodeString(&UnicodeString1,&UnicodeString1,FALSE);

	//�仯��
	KdPrint(("UnicodeString1 ת����д��:%wZ\n",&UnicodeString1));
	KdPrint(("\n-------------------�ִ�ת��д���� ����--------------- \n"));
}

//�ַ����������໥ת������
#pragma INITCODE
VOID StringToIntegerTest() 
{   
	KdPrint(("\n-------------------�ִ�ת�������� ��ʼ--------------- \n"));
	//(1)�ַ���ת��������
	//��ʼ��UnicodeString1
	UNICODE_STRING UnicodeString1;
	RtlInitUnicodeString(&UnicodeString1,L"101");

	ULONG lNumber;
	KdPrint(("��ת���ִ�%wZ \n",&UnicodeString1));
	NTSTATUS nStatus = RtlUnicodeStringToInteger(&UnicodeString1,2,&lNumber);
	if ( NT_SUCCESS(nStatus))
	{
		KdPrint(("�ִ�ת�������ɹ� ���=%d !\n",lNumber));
	}else
	{
		KdPrint(("ת������ ʧ��\n"));
	}

	//(2)����ת�����ַ���
	//��ʼ��UnicodeString2
	UNICODE_STRING UnicodeString2={0};
	UnicodeString2.Buffer = (PWSTR)ExAllocatePool(PagedPool,BUFFER_SIZE);
	UnicodeString2.MaximumLength = BUFFER_SIZE;
	nStatus = RtlIntegerToUnicodeString(200,2,&UnicodeString2);

	if ( NT_SUCCESS(nStatus))
	{
		KdPrint(("����ת���ִ��ɹ�! ���:%wZ\n",&UnicodeString2));
	}else
	{
		KdPrint(("ת���ִ� ʧ��!\n"));
	}

	//����UnicodeString2
	//ע��!���ù�RtlInitUnicodeString ��ʼ�����ִ�!UnicodeString1��������, 
	//RtlFreeUnicodeString(&UnicodeString1); ���ϴ��л�����
	RtlFreeUnicodeString(&UnicodeString2);
	KdPrint(("\n-------------------�ִ�ת�������� ����--------------- \n"));
}


//E��ANSI_STRING�ַ�����UNICODE_STRING�ַ����໥ת��

//ANSI_STRING�ַ�����UNICODE_STRING�ַ����໥ת������
#pragma INITCODE
VOID StringConverTest() 
{  
	KdPrint(("\n-------------------ANSI_STRING�ַ�����UNICODE_STRING�ַ����໥ת������ ��ʼ--------------- \n"));
	//(1)��UNICODE_STRING�ַ���ת����ANSI_STRING�ַ���
	//��ʼ��UnicodeString1
	UNICODE_STRING UnicodeString1;
	RtlInitUnicodeString(&UnicodeString1,L"UnicodeString1");

	ANSI_STRING AnsiString1;
	NTSTATUS nStatus = RtlUnicodeStringToAnsiString(&AnsiString1,&UnicodeString1,true);

	if ( NT_SUCCESS(nStatus))
	{
		KdPrint(("RtlUnicodeStringToAnsiString ת���ɹ� ���=%Z\n",&AnsiString1));
	}else
	{
		KdPrint(("RtlAnsiStringToUnicodeString ת��ʧ�� !\n"));
	}

	//����AnsiString1
	RtlFreeAnsiString(&AnsiString1);

	//(2)��ANSI_STRING�ַ���ת����UNICODE_STRING�ַ���
	//��ʼ��AnsiString2
	ANSI_STRING AnsiString2;
	RtlInitString(&AnsiString2,"AnsiString2");

	UNICODE_STRING UnicodeString2;
	nStatus = RtlAnsiStringToUnicodeString(&UnicodeString2,&AnsiString2,true);

	if ( NT_SUCCESS(nStatus))
	{
		KdPrint(("RtlAnsiStringToUnicodeStringת���ɹ� ���=%wZ\n",&UnicodeString2));
	}else
	{
		KdPrint(("RtlAnsiStringToUnicodeString�ִ�ת��ʧ��!\n"));
	}

	//����UnicodeString2
	RtlFreeUnicodeString(&UnicodeString2);
	KdPrint(("\n-------------------ANSI_STRING�ַ�����UNICODE_STRING�ַ����໥ת������ ����--------------- \n")); 

}