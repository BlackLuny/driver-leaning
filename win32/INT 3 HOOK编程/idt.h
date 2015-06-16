#include <ntddk.h>
#pragma pack(push)
#pragma pack(1) //
typedef struct _IDTR //IDT��ַ
{
	USHORT limit; //��Χ ռ8λ
	ULONG base; //����ַ ռ32λ _IDT_ENTRY����ָ��
}IDTR,*PIDTR;

typedef struct _IDT_ENTRY
{
	USHORT offset_low; //�жϴ�������ַ��16λ
	USHORT selector;
	UCHAR  reserved;
	UCHAR  type:4; 
    UCHAR  always0:1;
	UCHAR  dpl:2;
	UCHAR  present:1;
	USHORT offset_high;//�жϴ�������ַ��16λ
}IDT_ENTRY,*PIDT_ENTRY;
#pragma pack(pop) //#pragma pack(pop)

//-----------ȫ�ֱ���--------------------------------
ULONG int3proc_addr; //�������int 3��������ַ
ULONG jmpaddr_int3proc_9; //�������intproc+9��������ַ
//-----------ȫ�ֱ��� �������-----------------------

#pragma PAGECODE
ULONG ReadIdtBase(ULONG CPUNUM)			//��ȡIDT����׵�ַ
{
  IDTR idtr;//��ȡ���ַ
  PIDT_ENTRY Aidt;
       KdPrint(("IDT_ENTRY size=%d \n",sizeof(IDT_ENTRY)));
  __asm sidt idtr;//��ȡ���ַ��Ϣ
   KdPrint(("IDT BASE=%x \n",idtr.base));
  Aidt=PIDT_ENTRY(idtr.base);
  
  return idtr.base;
}

 
void __declspec(naked)  int3UnHookcode()  //IDT int 3 �׵�ַ��ǰ����ָ��
{
	__asm
	{
		push    0
		mov     word ptr [esp+2],0
	}

}


#pragma PAGECODE
void __declspec(naked)  myInt3Proc()
{	
	__asm
	{
		pushad			//����Ĵ���
		pushfd			//�����־�Ĵ���
	}

	KdPrint(("\n entry my Int3Proc \n"));
	//����������Լ����������˴���
	//��ȡ����������
	PEPROCESS EP;	 
	EP=PsGetCurrentProcess();
	// (PTSTR)((ULONG)EP+0x174)�Ƿ���� ��Ҫ���ϵ�Ľ���
	//dt _EPROCESS
	//+0x174 ImageFileName    : [16] UChar		//������������
	if (strcmp((PTSTR)((ULONG)EP+0x174),"notepad.exe")==0)			//ֵ��̽��
	{
		//��Ҫ�����Ľ��� ֱ������
		KdPrint(("\n ���� ���� ���� \n"));
		__asm retn 100;
	}
	__asm
	{
		popfd		//�ָ���־�Ĵ���
		popad		//�ָ��Ĵ���
	}

	__asm
	{

		    push 0
			mov word ptr [esp+2], 0
			//ǰ2����Ҫ�ָ���ָ�� ռ9�ֽ�
			jmp jmpaddr_int3proc_9
	}


}


#pragma  PAGECODE
ULONG HookInt3Proc()
{    
	
ULONG status=1;
PIDT_ENTRY Pidt_info=(PIDT_ENTRY)ReadIdtBase(0);
ULONG jmpaddr;

Pidt_info+=3;	//ת��IDT ����3 ��ߴ���� int 3 ��������ַ
//Pidt_info=Pidt_info+sizeof(Pidt_info)*3;
//begin�����int3��������ַ
int3proc_addr=Pidt_info->offset_high<<16;//makelong 0x804d0000
//MAKELONG(Pidt_info->offset_high,Pidt_info->offset_Slow) //0xfaa1 =804dfaa1  �����Ҳ����
int3proc_addr=int3proc_addr+Pidt_info->offset_low;
KdPrint (("\n int proc addr=%x \n",int3proc_addr));
//end;
//begin inline hook int3Proc write
// E9+jmp��ַ//jmp��ַ=myInt3Proc-int3proc_addr-5;
jmpaddr=ULONG(myInt3Proc)-int3proc_addr-5;		//�����ת��ƫ��
jmpaddr_int3proc_9=int3proc_addr+9;				//��������ת��ȥ�ĵ�ַ


//�޸�INT 3 �׵�ַ ������ת���Լ��ĺ���
__asm
{  
		push ebx
		push eax
		mov ebx,int3proc_addr			//IDT int 3���׵�ַ ������ebx
		mov byte ptr ds:[ebx],0xE9		//E9 ��jmpָ��
		mov eax,jmpaddr					//jmpaddr ��ƫ��
		mov dword ptr ds:[ebx+1],eax	//��д��ƫ��
		pop eax
		pop ebx
}
//end;inline hook int3proc write
return status;
}


#pragma  PAGECODE
ULONG UnHookInt3Proc()
{  
   ULONG status=1;
	
	KdPrint(("\n ж�� Idt Hook \n"));
	__asm
	{    
		push ebx
		push eax
		push ecx

		mov ebx,int3proc_addr		//INT 3 ���׵�ַ ����ebx
		lea ecx,int3UnHookcode		//int3UnHookcode �Ǻ�����ַ ���INT 3 ��ǰ����ָ�� �����ַ����ecx

		mov eax,[ecx+0]				//ǰ�ĸ��ֽڻָ�
		mov dword ptr ds:[ebx],eax

		mov eax,[ecx+4]
		mov dword ptr ds:[ebx+4],eax//�ٻָ��ĸ��ֽ�

		mov eax,[ecx+8]
		mov byte ptr ds:[ebx+8],al	//���ָ�һ���ֽ�
		
		pop ecx
		pop eax
		pop ebx
	}
	return status;
}