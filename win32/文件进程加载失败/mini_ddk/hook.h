#pragma once
#ifdef __cplusplus
extern "C"
{
#endif
#include <NTDDK.h> //���������Ҫ��C��ʽ�����ͷ�ļ�

#ifdef __cplusplus
}
#endif  
//#include <windef.h> 

bool ssdthook_flag=false;
ULONG     RealNtOpenAddress; 
HANDLE    MyPID; 


// A�������Լ����ں˺����������滻��Ӧ���ں˺���)

// ����һ��NtOpenProcess��ԭ��
extern "C"    typedef  NTSTATUS      __stdcall NTOPENPROCESS 
( 
 OUT PHANDLE ProcessHandle, 


 IN ACCESS_MASK AccessMask, 


 IN POBJECT_ATTRIBUTES ObjectAttributes, 


 IN PCLIENT_ID ClientId 


 );
NTOPENPROCESS   *  RealNtOpenProcess; 

PEPROCESS  EP;

// �Զ����NtOpenProcess���� ZwOpenProcess
#pragma PAGECODE
extern "C" NTSTATUS __declspec(naked) __stdcall MyNtOpenProcess( 
	OUT     PHANDLE ProcessHandle, 
	IN     ACCESS_MASK DesiredAccess, 
	IN     POBJECT_ATTRIBUTES ObjectAttributes, 
	IN     PCLIENT_ID ClientId ) 
{ 
	NTSTATUS     rc; 
	HANDLE       PID; 

	//KdPrint(("++++++++++++Entry MyNtOpenProcess int   ++++++++++++++\n"));  

	//rc = (NTSTATUS)RealNtOpenProcess( ProcessHandle, DesiredAccess, ObjectAttributes, ClientId ); 	

	if( (ClientId != NULL) ) 
	{ 
		PID = ClientId->UniqueProcess; 	 
		KdPrint(( "------------------------- PID=%d--------------\n",(int*)PID ));

		// ����Ǳ�������PID����ܾ����ʣ������������Ϊ�� 
		if(PID == MyPID) 
		{ 
			KdPrint(("���������� MyPID=%d \n",(int)MyPID));
			//������� ����C���Ե� Printf
			ProcessHandle = NULL; //����ǹؼ�
			rc = STATUS_ACCESS_DENIED; //�������ֵ 
			//PsLookupProcessByProcessId((ULONG)PID,&EP);
			EP=PsGetCurrentProcess();			 
			KdPrint((" ACESS Process Name  --:%s--   \n",(PTSTR)((ULONG)EP+0x174)));
			__asm
			{
				retn 0x10  //��ҪHOOK�ľ�ֱ�ӷ���
			}


		} 
	} 
	__asm
	{   int 3
		push    0C4h
		mov eax,RealNtOpenProcess //
		add eax,5
		jmp eax

	}
	//return rc; 
} 

//HOOK ��������
#pragma PAGECODE
VOID Hook() 
{ 
	ssdthook_flag=true;//���ñ�HOOK��־
	KdPrint(("++++HOOK START ++++-\n"));
	LONG *SSDT_Adr,SSDT_NtOpenProcess_Cur_Addr,t_addr; 

	KdPrint(("�����ɹ���������.............................\n"));
	//��ȡSSDT��������ֵΪ0x7A�ĺ���
	//poi(poi(KeServiceDescriptorTable)+0x7a*4)
	t_addr=(LONG)KeServiceDescriptorTable->ServiceTableBase;
	SSDT_Adr=(PLONG)(t_addr+0x7A*4);

	SSDT_NtOpenProcess_Cur_Addr=*SSDT_Adr;	 
	RealNtOpenAddress = *SSDT_Adr; 
	RealNtOpenProcess = ( NTOPENPROCESS *)RealNtOpenAddress; 

	KdPrint(( "��ʵ��NtOpenProcess��ַ: %x\n",(int) RealNtOpenAddress )); 
	KdPrint((" α��NTOpenProcess��ַ: %x\n", (int)MyNtOpenProcess ));  


	__asm //ȥ��ҳ�汣��
	{
		cli
			mov eax,cr0
			and eax,not 10000h //and eax,0FFFEFFFFh
			mov cr0,eax

	}

    //[804e5a88]=MyNtOpenProcess
	//[8058270a]=jmp MyNtOpenProcess=E9 jmpaddr
	ULONG jmpaddr=(ULONG)MyNtOpenProcess-RealNtOpenAddress-5;
	SSDT_Adr= (PLONG) *SSDT_Adr; //SSDT HOOK jmp jz jnz
	// in line hook
  __asm
  { 
	  mov ebx,SSDT_Adr //RealNtOpenAddress
	  mov byte ptr ds:[ebx],0xe9
	  mov eax,jmpaddr
	  mov DWORD ptr ds:[ebx+1],eax
  }

	__asm 
	{  int 3
		mov     eax, cr0 
		or     eax, 10000h 
		mov     cr0, eax 
		sti 
	}   
	return;
} 

//UnHook��������
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

	// ��ԭSSDT 
	*((ULONG*)Old_ssdt) = (ULONG)RealNtOpenAddress; 

	__asm 
	{ 
		mov     eax, cr0 
		or     eax, 10000h 
		mov     cr0, eax 
		sti 
	} 
	KdPrint(("UnHook��ԭSSDT OK \n")); 
	}

	return;
}