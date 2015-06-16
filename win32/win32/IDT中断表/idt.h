#include <ntddk.h>

#pragma pack(push)	//Ĭ�϶���ֵѹջ
#pragma pack(1)		//�����µĶ���ֵ
//#pragma pack(push,1)	
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
	UCHAR  type:4;		//������ ��������
    UCHAR  always0:1;
	UCHAR  dpl:2;
	UCHAR  present:1;
	USHORT offset_high;//�жϴ�������ַ��16λ
}IDT_ENTRY,*PIDT_ENTRY;
#pragma pack(pop)		//����ֵ��ջ ���ָ�Ĭ�϶���ֵ


ULONG ReadIdt(ULONG CPUNUM)
{
  IDTR idtr;//��ȡ���ַ
  PIDT_ENTRY Aidt;
       KdPrint(("IDT_ENTRY size=%d \n",sizeof(IDT_ENTRY)));
  __asm sidt idtr;//��ȡ���ַ��Ϣ   
  /*
  idtr ���ж���������Ĵ��� ռ48λ ��16λΪ�䷶Χ ��32λΪ���ַ 
  SIDT ָ��
	���ж���������Ĵ���IDTR�����ݴ���ָ����ַ��Ԫ��
	SIDT ������Ȩָ�����˵���ǿ�����Ring3��ִ�и�ָ����IDT�Ļ���ַ���Ӷ��޸�IDT������һ���ж��Ű������ǵ��жϷ���
	һ��Ring3�����в������жϣ�VMM�ͻ���ô��жϷ�����򣬶����жϷ�������������Ring0���ˡ�
  LIDT
	���ڰ��ڴ��е��޳�ֵ�ͻ���ַ���������ص�IDTR�Ĵ����С�
	��ָ������ɵ�ǰ��Ȩ��CPL��0�Ĵ���ִ�У�ͨ�������ڴ���IDTʱ�Ĳ���ϵͳ��ʼ�������С�
  */
  KdPrint(("IDT BASE=%x \n",idtr.base));
  Aidt=PIDT_ENTRY(idtr.base);
  for (int i=0;i<0xff;i++)
  {
	  ULONG cur_idt= Aidt->offset_high;
      cur_idt=cur_idt<<16;//�õ���λ��ַ
	  cur_idt=cur_idt+ULONG(Aidt->offset_low);//�ϳ� �жϴ�������ַ
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

��ʵ��ַģʽ�У�CPU���ڴ��д�0��ʼ��1K�ֽ���Ϊһ���ж����������е�ÿ������ռ�ĸ��ֽڣ�
�������ֽڵĶε�ַ�������ֽڵ�ƫ������ɣ��������ɵĵ�ַ������Ӧ�жϴ���������ڵ�ַ��
���ǣ��ڱ���ģʽ�£������ֽڵı���ɵ��ж���������Ȼ���㲻��Ҫ��������Ϊ�����������ֽڵĶ���������
ƫ�����������ֽ�����ʾ��Ҫ�з�ӳģʽ�л�����Ϣ����ˣ��ڱ���ģʽ�£��ж��������еı�����8���ֽ���ɣ�
�ж�������Ҳ�Ľ����ж���������IDT��InterruptDescriptor Table�������е�ÿ���������һ������������gate descriptor����
���š��ĺ����ǵ��жϷ���ʱ������ͨ����Щ�ţ�Ȼ����ܽ�����Ӧ�Ĵ������

��Ҫ���������ǣ�
�� �ж��ţ�Interrupt gate��
��������Ϊ110,�ж��Ű�����һ���жϻ��쳣����������ڶε�ѡ����Ͷ���ƫ������������Ȩͨ���ж��Ž����жϴ������ʱ����������IF��־�������жϣ��Ա���Ƕ���жϵķ������ж����е�DPL��Descriptor Privilege Level��Ϊ0����ˣ��û�̬�Ľ��̲��ܷ���Intel���ж��š����е��жϴ���������ж��ż����ȫ���������ں�̬��
�� �����ţ�Trap gate��
��������Ϊ111,���ж������ƣ���Ψһ�������ǣ�����Ȩͨ�������Ž��봦�����ʱά��IF��־λ���䣬Ҳ����˵�������жϡ�
�� ϵͳ�ţ�System gate��
����Linux�ں��ر����õģ��������û�̬�Ľ��̷���Intel�������ţ���ˣ�����������DPLΪ3��ͨ��ϵͳ��������4��Linux�쳣����������ǵ�������3��4��5��128��Ҳ����˵�����û�̬�£�����ʹ��int3��into��bound ��int0x80�������ָ�

*/