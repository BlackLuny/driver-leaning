#include <ntddk.h>

#pragma pack(push)	//默认对其值压栈
#pragma pack(1)		//设置新的对其值
//#pragma pack(push,1)	
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
	UCHAR  type:4;		//类型码 门描述符
    UCHAR  always0:1;
	UCHAR  dpl:2;
	UCHAR  present:1;
	USHORT offset_high;//中断处理函数地址低16位
}IDT_ENTRY,*PIDT_ENTRY;
#pragma pack(pop)		//对其值出栈 即恢复默认对其值


ULONG ReadIdt(ULONG CPUNUM)
{
  IDTR idtr;//获取表基址
  PIDT_ENTRY Aidt;
       KdPrint(("IDT_ENTRY size=%d \n",sizeof(IDT_ENTRY)));
  __asm sidt idtr;//获取表基址信息   
  /*
  idtr 是中断描述符表寄存器 占48位 低16位为其范围 高32位为其地址 
  SIDT 指令
	将中断描述符表寄存器IDTR的内容存入指定地址单元。
	SIDT 不是特权指令，就是说我们可以在Ring3下执行该指令，获得IDT的基地址，从而修改IDT，增加一个中断门安置我们的中断服务，
	一旦Ring3程序中产生此中断，VMM就会调用此中断服务程序，而此中断服务程序就运行在Ring0下了。
  LIDT
	用于把内存中的限长值和基地址操作数加载到IDTR寄存器中。
	该指令仅能由当前特权级CPL是0的代码执行，通常被用于创建IDT时的操作系统初始化代码中。
  */
  KdPrint(("IDT BASE=%x \n",idtr.base));
  Aidt=PIDT_ENTRY(idtr.base);
  for (int i=0;i<0xff;i++)
  {
	  ULONG cur_idt= Aidt->offset_high;
      cur_idt=cur_idt<<16;//得到高位地址
	  cur_idt=cur_idt+ULONG(Aidt->offset_low);//合成 中断处理函数地址
	 // if ( Aidt->offset_high==0) {__asm int 3};
	 // if ( Aidt->offset_low==0) {__asm int 3};
	 KdPrint(("high=%x,low=%x,IDT %d=%0000x \n",Aidt->offset_high,Aidt->offset_low,i,cur_idt));
	//  if ( Aidt->offset_high==0)
	//  {  KdPrint(("high=%x,low=%x,IDT %d=%x \n",Aidt->offset_high,Aidt->offset_low,i,cur_idt));}
	//  else{  KdPrint(("high=%x,low=%x,IDT %d=0000%x \n",Aidt->offset_high,Aidt->offset_low,i,cur_idt));}
	    
	  Aidt++;
    
  }
  

  return idtr.base;

}

/*

在实地址模式中，CPU把内存中从0开始的1K字节作为一个中断向量表。表中的每个表项占四个字节，
由两个字节的段地址和两个字节的偏移量组成，这样构成的地址便是相应中断处理程序的入口地址。
但是，在保护模式下，由四字节的表项构成的中断向量表显然满足不了要求。这是因为，除了两个字节的段描述符，
偏移量必用四字节来表示；要有反映模式切换的信息。因此，在保护模式下，中断向量表中的表项由8个字节组成，
中断向量表也改叫做中断描述符表IDT（InterruptDescriptor Table）。其中的每个表项叫做一个门描述符（gate descriptor），
“门”的含义是当中断发生时必须先通过这些门，然后才能进入相应的处理程序。

主要门描述符是：
· 中断门（Interrupt gate）
其类型码为110,中断门包含了一个中断或异常处理程序所在段的选择符和段内偏移量。当控制权通过中断门进入中断处理程序时，处理器清IF标志，即关中断，以避免嵌套中断的发生。中断门中的DPL（Descriptor Privilege Level）为0，因此，用户态的进程不能访问Intel的中断门。所有的中断处理程序都由中断门激活，并全部限制在内核态。
· 陷阱门（Trap gate）
其类型码为111,与中断门类似，其唯一的区别是，控制权通过陷阱门进入处理程序时维持IF标志位不变，也就是说，不关中断。
· 系统门（System gate）
这是Linux内核特别设置的，用来让用户态的进程访问Intel的陷阱门，因此，门描述符的DPL为3。通过系统门来激活4个Linux异常处理程序，它们的向量是3、4、5及128，也就是说，在用户态下，可以使用int3、into、bound 及int0x80四条汇编指令。

*/