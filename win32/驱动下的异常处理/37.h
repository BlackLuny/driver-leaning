void Memaccess_Test()
{
	KdPrint(("\n�����ڴ���÷�\n"));
	int i=3, *pi=NULL;
	__try
	{
		*pi=i;
	}
	__except(1)
	{
		KdPrint(("�����ڴ� ������\n"));
		return;
	}

}

void ProbeForRead_Test()
{
	KdPrint(("nProbeForRead �����ڴ���÷�\n"));
	int  *pi=NULL;
	__try
	{  ProbeForRead(pi,14,4);
	//ProbeForWrite(pi,4,1);
	// KdPrint(("\n ProbeForRead end *pi=%d \n",*pi));

	//i=*pi;
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		KdPrint(("\n ProbeForRead �����ڴ� ������\n"));
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
		KdPrint(("\n __finally  д����� \n"));
		
	}
	__try
	{
		i=3;
	}
	__finally
	{
		KdPrint(("\n __finally  i=3; д����� \n"));
		
	}
	return;
}

void ASSERT_test()
{  int *p=NULL;
int i=1;
ASSERT(p!=NULL); //Ҳ�����ֶ�ǿ���׳� ����ASSERT(FALSE);

return;
}
#define add(a,b) { a=a+b; b=a+b;}
void test1()
{int a=1,b=1;
if  (false) {add(a,b);}
 
 
 KdPrint(("\n a=%d,b=%d \n",a,b));
	return;
}
