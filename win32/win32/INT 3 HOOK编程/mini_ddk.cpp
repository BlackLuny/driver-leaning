//_stdcall
#include "mini_ddk.h"
#include "idt.h"

JMPCODE oldCode;//��������ǰ5�ֽ� �Ա�ָ�
PJMPCODE pcur;
bool ishook=false;

#pragma  INITCODE
extern "C" NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObject,PUNICODE_STRING B) //TYPEDEF LONG NTSTATUS
{  
	
	HookInt3Proc();		//IDtT HOOK

	//ע����ǲ����
	pDriverObject->MajorFunction[IRP_MJ_CREATE]=ddk_DispatchRoutine_CONTROL; //IRP_MJ_CREATE���IRP������
	pDriverObject->MajorFunction[IRP_MJ_CLOSE]=ddk_DispatchRoutine_CONTROL; //IRP_MJ_CREATE���IRP������
	pDriverObject->MajorFunction[IRP_MJ_READ]=ddk_DispatchRoutine_CONTROL; //IRP_MJ_CREATE���IRP������
	pDriverObject->MajorFunction[IRP_MJ_CLOSE]=ddk_DispatchRoutine_CONTROL; //IRP_MJ_CREATE���IRP������
	pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL]=ddk_DispatchRoutine_CONTROL; //IRP_MJ_CREATE���IRP������
	CreateMyDevice(pDriverObject);//������Ӧ���豸
	pDriverObject->DriverUnload=DDK_Unload;
	return (1);
}
//#pragma code_seg("PAGE")
#pragma PAGECODE
VOID DDK_Unload (IN PDRIVER_OBJECT pDriverObject)
{ 
  PDEVICE_OBJECT pDev;//����ȡ��Ҫɾ���豸����
  UNICODE_STRING symLinkName; // 


	UnHookInt3Proc();		//IDT UNHOOK
  
 
  pDev=pDriverObject->DeviceObject;
  IoDeleteDevice(pDev); //ɾ���豸
  
  //ȡ������������
   RtlInitUnicodeString(&symLinkName,L"\\??\\My_DriverLinkName");
  //ɾ����������
   IoDeleteSymbolicLink(&symLinkName);
   KdPrint(("�����ɹ���ж��...OK-----------")); //sprintf,printf
	//ȡ��Ҫɾ���豸����
	//ɾ�������豸
	DbgPrint("ж�سɹ�");
}
#pragma PAGECODE
NTSTATUS ddk_DispatchRoutine_CONTROL(IN PDEVICE_OBJECT pDevobj,IN PIRP pIrp	)
{   //
	ULONG info;
	int *pi=(int*)ExAllocatePool(PagedPool,sizeof(int));
	//�õ���ǰջָ��
	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(pIrp);
	ULONG mf=stack->MajorFunction;//����IRP
	switch (mf)
	{
	case IRP_MJ_DEVICE_CONTROL:
		{ 	KdPrint(("Enter myDriver_DeviceIOControl\n"));
		NTSTATUS status = STATUS_SUCCESS;	

		//�õ����뻺������С
		ULONG cbin = stack->Parameters.DeviceIoControl.InputBufferLength;
		//�õ������������С
		ULONG cbout = stack->Parameters.DeviceIoControl.OutputBufferLength;
		//�õ�IOCTL��
		ULONG code = stack->Parameters.DeviceIoControl.IoControlCode;
		switch (code)
		{ 
		case add_code:
			{  		
				int a,b;
				KdPrint(("add_code 1111111111111111111\n"));
				//��������ʽIOCTL
				//��ȡ����������	a,b		
				int * InputBuffer = (int*)pIrp->AssociatedIrp.SystemBuffer;
				_asm
				{
					   mov eax,InputBuffer
						mov ebx,[eax]
						mov a,ebx
						mov ebx,[eax+4]
						mov b,ebx
				}
				KdPrint(("a=%d,b=%d \n", a,b));

				a=a+b;
				//C�������㷵���������û���
				//�������������
				int* OutputBuffer = (int*)pIrp->AssociatedIrp.SystemBuffer;
				_asm
				{
					    mov eax,a
						mov ebx,OutputBuffer
						mov [ebx],eax //bufferet=a+b

				}
				KdPrint(("a+b=%d \n",a));

				//����ʵ�ʲ����������������
				info = 4;
				break;
			}
		case hook_code:
			{   
				break;
			}
		case unhook_code:
			{  // UnHook();
				break;
			}
		case sub_code:
			{
				break;
			}
		}//end code switch
		break;
		}
	case IRP_MJ_CREATE:
		{
			break;
		}
	case IRP_MJ_CLOSE:
		{
			break;
		}
	case IRP_MJ_READ:
		{
			break;
		}

	}

	//����Ӧ��IPR���д���
	pIrp->IoStatus.Information=info;//���ò������ֽ���Ϊ0��������ʵ������
	pIrp->IoStatus.Status=STATUS_SUCCESS;//���سɹ�
	IoCompleteRequest(pIrp,IO_NO_INCREMENT);//ָʾ��ɴ�IRP
	KdPrint(("�뿪��ǲ����\n"));//������Ϣ
	return STATUS_SUCCESS; //���سɹ�
}