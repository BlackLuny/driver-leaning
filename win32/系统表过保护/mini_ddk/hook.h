#pragma once
#ifdef __cplusplus
extern "C"
{
#endif
#include <NTDDK.h> //这里包含需要用C方式编译的头文件

#ifdef __cplusplus
}
#endif  
//#include <windef.h> 

bool ssdthook_flag=false;
ULONG     RealNtOpenAddress; 
HANDLE    MyPID; 


// A、构建自己的内核函数（用来替换对应的内核函数)

// 定义一下NtOpenProcess的原型
extern "C"  typedef NTSTATUS __stdcall NTOPENPROCESS 
( 
 OUT PHANDLE ProcessHandle, 
 IN ACCESS_MASK AccessMask, 
 IN POBJECT_ATTRIBUTES ObjectAttributes, 
 IN PCLIENT_ID ClientId 
 );
NTOPENPROCESS   *  RealNtOpenProcess; 

PEPROCESS  EP;

// 自定义的NtOpenProcess函数 ZwOpenProcess
#pragma PAGECODE
extern "C" NTSTATUS __stdcall MyNtOpenProcess( 
	OUT     PHANDLE ProcessHandle, 
	IN     ACCESS_MASK DesiredAccess, 
	IN     POBJECT_ATTRIBUTES ObjectAttributes, 
	IN     PCLIENT_ID ClientId ) 
{ 
	NTSTATUS     rc; 
	HANDLE       PID; 

	KdPrint(("++++++++++++Entry MyNtOpenProcess int   ++++++++++++++\n"));  

	rc = (NTSTATUS)RealNtOpenProcess( ProcessHandle, DesiredAccess, ObjectAttributes, ClientId ); 	

	if( (ClientId != NULL) ) 
	{ 
		PID = ClientId->UniqueProcess; 	 
		KdPrint(( "------------------------- PID=%d--------------\n",(int*)PID ));

		// 如果是被保护的PID，则拒绝访问，并将句柄设置为空 
		if(PID == MyPID) 
		{ 
			KdPrint(("被保护进程 MyPID=%d \n",(int)MyPID));
			//调试输出 类似C语言的 Printf
			ProcessHandle = NULL; //这个是关键
			rc = STATUS_ACCESS_DENIED; //这个返回值 
			//PsLookupProcessByProcessId((ULONG)PID,&EP);
			EP=PsGetCurrentProcess();			 
			KdPrint((" ACESS Process Name  --:%s--   \n",(PTSTR)((ULONG)EP+0x174)));

		} 
	} 

	return rc; 
} 

//HOOK 函数构建
#pragma PAGECODE
VOID Hook() 
{ 
	ssdthook_flag=true;//设置被HOOK标志
	KdPrint(("++++HOOK START ++++-\n"));
	LONG *SSDT_Adr,SSDT_NtOpenProcess_Cur_Addr,t_addr; 

	KdPrint(("驱动成功被加载中.............................\n"));
	//读取SSDT表中索引值为0x7A的函数
	//poi(poi(KeServiceDescriptorTable)+0x7a*4)
	t_addr=(LONG)KeServiceDescriptorTable->ServiceTableBase;
	SSDT_Adr=(PLONG)(t_addr+0x7A*4);

	SSDT_NtOpenProcess_Cur_Addr=*SSDT_Adr;	 
	RealNtOpenAddress = *SSDT_Adr; 
	RealNtOpenProcess = ( NTOPENPROCESS *)RealNtOpenAddress; 

	KdPrint(( "真实的NtOpenProcess地址: %x\n",(int) RealNtOpenAddress )); 
	KdPrint((" 伪造NTOpenProcess地址: %x\n", (int)MyNtOpenProcess ));  


	__asm //去掉页面保护
	{
		cli
			mov eax,cr0
			and eax,not 10000h //and eax,0FFFEFFFFh
			mov cr0,eax

	}


	*SSDT_Adr= (ULONG)MyNtOpenProcess; //SSDT HOOK


	__asm 
	{ 
		mov     eax, cr0 
		or     eax, 10000h 
		mov     cr0, eax 
		sti 
	}   
	return;
} 

//UnHook函数构建
////////////////////////////////////////////////////// 
#pragma PAGECODE
VOID UnHook() 
{ 
	ULONG Old_ssdt; 
	Old_ssdt = (ULONG)KeServiceDescriptorTable->ServiceTableBase + 0x7A * 4; 
	if    (ssdthook_flag)
	{    ssdthook_flag=false;
	__asm 
	{ 
		cli 
			mov     eax, cr0 
			and     eax, not 10000h 
			mov     cr0, eax 
	} 

	// 还原SSDT 
	*((ULONG*)Old_ssdt) = (ULONG)RealNtOpenAddress; 

	__asm 
	{ 
		mov     eax, cr0 
		or     eax, 10000h 
		mov     cr0, eax 
		sti 
	} 
	KdPrint(("UnHook还原SSDT OK \n")); 
	}

	return;
}