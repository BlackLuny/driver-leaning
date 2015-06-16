//----------------------www.yjxsoft.com------------------郁金香灬技术
//文件名 "38.h"
#define BUFFER_SIZE 1024

#pragma INITCODE
VOID CharTest()
{  
	KdPrint(("Char字串测试--Start \n"));

	PCHAR s1="abc11";        //CHAR ANSI
	KdPrint(("%x,%s\n",s1,s1)); 
	WCHAR* s2=L"abc11";     //WCHAR UNICODE //PWSTR
	KdPrint(("%x,%S\n",s2,s2));

	KdPrint(("Char字串测试--End \n"));
	_asm int 3

}

//ANSI_STRING
//UNICODE_STRING
//字符串初始化测试
#pragma INITCODE
VOID StringInitTest() 
{
	KdPrint(("初始化字串测试--Start \n"));
	ANSI_STRING  AnsiString1={0};
	UNICODE_STRING UnicodeString1={0};
    // AnsiString1.Buffer="AnsiString1字符串";
	AnsiString1.Buffer=(PCHAR)ExAllocatePool(PagedPool,BUFFER_SIZE);
	strcpy(AnsiString1.Buffer,"AnsiString1字符串");

	AnsiString1.Length=strlen( AnsiString1.Buffer);
	AnsiString1.MaximumLength=BUFFER_SIZE;

	//UnicodeString1.Buffer =  L"3333333333333333333322222222221111112";
	UnicodeString1.Buffer=(PWSTR)ExAllocatePool(PagedPool,BUFFER_SIZE);
	wcscpy(UnicodeString1.Buffer,L"3333333333333333333322222222221111112");
 
	UnicodeString1.Length=wcslen(UnicodeString1.Buffer)*2-1;
	UnicodeString1.MaximumLength = BUFFER_SIZE;

	//方法2 用RTL函数初始化
	//初始化ANSI_STRING字符串
	//RtlInitAnsiString(&AnsiString1,"AnsiString1字符串");	 
	//RtlInitUnicodeString(&UnicodeString1,L"3333333333333333333322222222221111112");


	//打印ASCII字符用 %Z
	KdPrint(("%x AnsiString1:   %Z\n",&AnsiString1,   &AnsiString1));
	//打印UNICODE字符用 %wZ 中文字符会被截断
	KdPrint(("%x UnicodeString1:%wZ\n",&UnicodeString1,&UnicodeString1));
	KdPrint(("初始化字串测试--END \n")); 
	//__asm int 3
	//释放字串
	RtlFreeAnsiString(&AnsiString1);//ExFreePool()
	RtlFreeUnicodeString(&UnicodeString1);

}//D、字符串复制，比较，（大小写，整数和字串）相互转换

#pragma INITCODE
VOID StringCopyTest() 
{ 
	KdPrint(("\n-------------------字串复制测试开始---------------D \n"));
	//初始化UnicodeString1
	UNICODE_STRING UnicodeString1;
	RtlInitUnicodeString(&UnicodeString1,L"UnicodeString1");

	//初始化UnicodeString2
	UNICODE_STRING UnicodeString2={0};
	UnicodeString2.Buffer = (PWSTR)ExAllocatePool(PagedPool,BUFFER_SIZE);
	UnicodeString2.MaximumLength = BUFFER_SIZE;
    __asm int 3
	//将初始化UnicodeString1拷贝到UnicodeString2
	RtlCopyUnicodeString(&UnicodeString2,&UnicodeString1);//UnicodeString2=UnicodeString1

	//分别显示UnicodeString1和UnicodeString2
	KdPrint(("字串1:%wZ\n",&UnicodeString1));
	KdPrint(("字串2:%wZ\n",&UnicodeString2));
   if ( RtlEqualUnicodeString(&UnicodeString1,&UnicodeString2,true))
   { KdPrint(("2个 字串相等 、\n"));}
	//销毁UnicodeString2
	//注意!调用过RtlInitUnicodeString 初始化的字串!UnicodeString1不用销毁, 

	//RtlFreeUnicodeString(&UnicodeString1); 加上此行会蓝屏
	RtlFreeUnicodeString(&UnicodeString2);
	KdPrint(("\n-------------------字串复制测试结束--------------- \n"));

}


//字符串变大写测试
#pragma INITCODE
VOID StringToUpperTest() 
{  
	KdPrint(("\n-------------------字串转大写测试 开始--------------- \n"));
	//初始化UnicodeString1
	UNICODE_STRING UnicodeString1;
	RtlInitUnicodeString(&UnicodeString1,L"UnicodeString1—Aabc");

	//变化前
	KdPrint(("UnicodeString1 初值:%wZ\n",&UnicodeString1));

	//变大写
	RtlUpcaseUnicodeString(&UnicodeString1,&UnicodeString1,FALSE);

	//变化后
	KdPrint(("UnicodeString1 转换大写后:%wZ\n",&UnicodeString1));
	KdPrint(("\n-------------------字串转大写测试 结束--------------- \n"));
}

//字符串与整型相互转化测试
#pragma INITCODE
VOID StringToIntegerTest() 
{   
	KdPrint(("\n-------------------字串转整数测试 开始--------------- \n"));
	//(1)字符串转换成数字
	//初始化UnicodeString1
	UNICODE_STRING UnicodeString1;
	RtlInitUnicodeString(&UnicodeString1,L"101");

	ULONG lNumber;
	KdPrint(("待转换字串%wZ \n",&UnicodeString1));
	NTSTATUS nStatus = RtlUnicodeStringToInteger(&UnicodeString1,2,&lNumber);
	if ( NT_SUCCESS(nStatus))
	{
		KdPrint(("字串转换整数成功 结果=%d !\n",lNumber));
	}else
	{
		KdPrint(("转换整数 失败\n"));
	}

	//(2)数字转换成字符串
	//初始化UnicodeString2
	UNICODE_STRING UnicodeString2={0};
	UnicodeString2.Buffer = (PWSTR)ExAllocatePool(PagedPool,BUFFER_SIZE);
	UnicodeString2.MaximumLength = BUFFER_SIZE;
	nStatus = RtlIntegerToUnicodeString(200,2,&UnicodeString2);

	if ( NT_SUCCESS(nStatus))
	{
		KdPrint(("数字转换字串成功! 结果:%wZ\n",&UnicodeString2));
	}else
	{
		KdPrint(("转换字串 失败!\n"));
	}

	//销毁UnicodeString2
	//注意!调用过RtlInitUnicodeString 初始化的字串!UnicodeString1不用销毁, 
	//RtlFreeUnicodeString(&UnicodeString1); 加上此行会蓝屏
	RtlFreeUnicodeString(&UnicodeString2);
	KdPrint(("\n-------------------字串转整数测试 结束--------------- \n"));
}


//E、ANSI_STRING字符串和UNICODE_STRING字符串相互转换

//ANSI_STRING字符串与UNICODE_STRING字符串相互转换测试
#pragma INITCODE
VOID StringConverTest() 
{  
	KdPrint(("\n-------------------ANSI_STRING字符串与UNICODE_STRING字符串相互转换测试 开始--------------- \n"));
	//(1)将UNICODE_STRING字符串转换成ANSI_STRING字符串
	//初始化UnicodeString1
	UNICODE_STRING UnicodeString1;
	RtlInitUnicodeString(&UnicodeString1,L"UnicodeString1");

	ANSI_STRING AnsiString1;
	NTSTATUS nStatus = RtlUnicodeStringToAnsiString(&AnsiString1,&UnicodeString1,true);

	if ( NT_SUCCESS(nStatus))
	{
		KdPrint(("RtlUnicodeStringToAnsiString 转换成功 结果=%Z\n",&AnsiString1));
	}else
	{
		KdPrint(("RtlAnsiStringToUnicodeString 转换失败 !\n"));
	}

	//销毁AnsiString1
	RtlFreeAnsiString(&AnsiString1);

	//(2)将ANSI_STRING字符串转换成UNICODE_STRING字符串
	//初始化AnsiString2
	ANSI_STRING AnsiString2;
	RtlInitString(&AnsiString2,"AnsiString2");

	UNICODE_STRING UnicodeString2;
	nStatus = RtlAnsiStringToUnicodeString(&UnicodeString2,&AnsiString2,true);

	if ( NT_SUCCESS(nStatus))
	{
		KdPrint(("RtlAnsiStringToUnicodeString转换成功 结果=%wZ\n",&UnicodeString2));
	}else
	{
		KdPrint(("RtlAnsiStringToUnicodeString字串转换失败!\n"));
	}

	//销毁UnicodeString2
	RtlFreeUnicodeString(&UnicodeString2);
	KdPrint(("\n-------------------ANSI_STRING字符串与UNICODE_STRING字符串相互转换测试 结束--------------- \n")); 

}