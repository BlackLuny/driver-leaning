#include <ntddk.h>
#pragma pack(push)
#pragma pack(1) //
typedef struct _IDTR //IDT基址
{
	USHORT limit; //范围 占8位
	ULONG base; //基地址 占32位 _IDT_ENTRY类型指针
}IDTR,*PIDTR;

typedef struct _IDT_ENTRY
{
	USHORT offset_low; //中断处理函数地址低16位
	USHORT selector;
	UCHAR  reserved;
	UCHAR  type:4; 
    UCHAR  always0:1;
	UCHAR  dpl:2;
	UCHAR  present:1;
	USHORT offset_high;//中断处理函数地址低16位
}IDT_ENTRY,*PIDT_ENTRY;
#pragma pack(pop) //#pragma pack(pop)

//-----------全局变量--------------------------------
ULONG int3proc_addr; //用来存放int 3处理函数地址
ULONG jmpaddr_int3proc_9; //用来存放intproc+9处理函数地址
//-----------全局变量 定义结束-----------------------

#pragma PAGECODE
ULONG ReadIdtBase(ULONG CPUNUM)			//获取IDT表的首地址
{
  IDTR idtr;//获取表基址
  PIDT_ENTRY Aidt;
       KdPrint(("IDT_ENTRY size=%d \n",sizeof(IDT_ENTRY)));
  __asm sidt idtr;//获取表基址信息
   KdPrint(("IDT BASE=%x \n",idtr.base));
  Aidt=PIDT_ENTRY(idtr.base);
  
  return idtr.base;
}

 
void __declspec(naked)  int3UnHookcode()  //IDT int 3 首地址的前两条指令
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
		pushad			//保存寄存器
		pushfd			//保存标志寄存器
	}

	KdPrint(("\n entry my Int3Proc \n"));
	//在这里添加自己的条件过滤代码
	//获取进程上下文
	PEPROCESS EP;	 
	EP=PsGetCurrentProcess();
	// (PTSTR)((ULONG)EP+0x174)是否等于 需要反断点的进程
	//dt _EPROCESS
	//+0x174 ImageFileName    : [16] UChar		//估计是其名称
	if (strcmp((PTSTR)((ULONG)EP+0x174),"notepad.exe")==0)			//值得探讨
	{
		//需要保护的进程 直接蓝屏
		KdPrint(("\n 蓝屏 蓝屏 蓝屏 \n"));
		__asm retn 100;
	}
	__asm
	{
		popfd		//恢复标志寄存器
		popad		//恢复寄存器
	}

	__asm
	{

		    push 0
			mov word ptr [esp+2], 0
			//前2条需要恢复的指令 占9字节
			jmp jmpaddr_int3proc_9
	}


}


#pragma  PAGECODE
ULONG HookInt3Proc()
{    
	
ULONG status=1;
PIDT_ENTRY Pidt_info=(PIDT_ENTRY)ReadIdtBase(0);
ULONG jmpaddr;

Pidt_info+=3;	//转到IDT 数组3 里边存放着 int 3 处理函数地址
//Pidt_info=Pidt_info+sizeof(Pidt_info)*3;
//begin计算出int3处理函数地址
int3proc_addr=Pidt_info->offset_high<<16;//makelong 0x804d0000
//MAKELONG(Pidt_info->offset_high,Pidt_info->offset_Slow) //0xfaa1 =804dfaa1  这个宏也可以
int3proc_addr=int3proc_addr+Pidt_info->offset_low;
KdPrint (("\n int proc addr=%x \n",int3proc_addr));
//end;
//begin inline hook int3Proc write
// E9+jmp地址//jmp地址=myInt3Proc-int3proc_addr-5;
jmpaddr=ULONG(myInt3Proc)-int3proc_addr-5;		//求出跳转的偏移
jmpaddr_int3proc_9=int3proc_addr+9;				//求出最后跳转回去的地址


//修改INT 3 首地址 让其跳转到自己的函数
__asm
{  
		push ebx
		push eax
		mov ebx,int3proc_addr			//IDT int 3的首地址 保存在ebx
		mov byte ptr ds:[ebx],0xE9		//E9 是jmp指令
		mov eax,jmpaddr					//jmpaddr 是偏移
		mov dword ptr ds:[ebx+1],eax	//再写入偏移
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
	
	KdPrint(("\n 卸载 Idt Hook \n"));
	__asm
	{    
		push ebx
		push eax
		push ecx

		mov ebx,int3proc_addr		//INT 3 的首地址 放入ebx
		lea ecx,int3UnHookcode		//int3UnHookcode 是函数地址 存放INT 3 的前两条指令 把其地址存入ecx

		mov eax,[ecx+0]				//前四个字节恢复
		mov dword ptr ds:[ebx],eax

		mov eax,[ecx+4]
		mov dword ptr ds:[ebx+4],eax//再恢复四个字节

		mov eax,[ecx+8]
		mov byte ptr ds:[ebx+8],al	//最后恢复一个字节
		
		pop ecx
		pop eax
		pop ebx
	}
	return status;
}