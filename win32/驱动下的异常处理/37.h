void Memaccess_Test()
{
	KdPrint(("\n测试内存可用否\n"));
	int i=3, *pi=NULL;
	__try
	{
		*pi=i;
	}
	__except(1)
	{
		KdPrint(("测试内存 不可用\n"));
		return;
	}

}

void ProbeForRead_Test()
{
	KdPrint(("nProbeForRead 测试内存可用否\n"));
	int  *pi=NULL;
	__try
	{  ProbeForRead(pi,14,4);
	//ProbeForWrite(pi,4,1);
	// KdPrint(("\n ProbeForRead end *pi=%d \n",*pi));

	//i=*pi;
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		KdPrint(("\n ProbeForRead 测试内存 不可用\n"));
		return;
	}
		
		return;

}

void finally_test()
{  int *p=NULL;
   int i=1;
	__try
	{
		*p=i;
	}
	__finally
	{
		KdPrint(("\n __finally  写入出错 \n"));
		
	}
	__try
	{
		i=3;
	}
	__finally
	{
		KdPrint(("\n __finally  i=3; 写入出错 \n"));
		
	}
	return;
}

void ASSERT_test()
{  int *p=NULL;
int i=1;
ASSERT(p!=NULL); //也可以手动强制抛出 断言ASSERT(FALSE);

return;
}
#define add(a,b) { a=a+b; b=a+b;}
void test1()
{int a=1,b=1;
if  (false) {add(a,b);}
 
 
 KdPrint(("\n a=%d,b=%d \n",a,b));
	return;
}
