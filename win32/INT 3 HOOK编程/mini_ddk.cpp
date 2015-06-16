//_stdcall
#include "mini_ddk.h"
#include "idt.h"

JMPCODE oldCode;//用来保存前5字节 以便恢复
PJMPCODE pcur;
bool ishook=false;

#pragma  INITCODE
extern "C" NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObject,PUNICODE_STRING B) //TYPEDEF LONG NTSTATUS
{  
	
	HookInt3Proc();		//IDtT HOOK

	//注册派遣函数
	pDriverObject->MajorFunction[IRP_MJ_CREATE]=ddk_DispatchRoutine_CONTROL; //IRP_MJ_CREATE相关IRP处理函数
	pDriverObject->MajorFunction[IRP_MJ_CLOSE]=ddk_DispatchRoutine_CONTROL; //IRP_MJ_CREATE相关IRP处理函数
	pDriverObject->MajorFunction[IRP_MJ_READ]=ddk_DispatchRoutine_CONTROL; //IRP_MJ_CREATE相关IRP处理函数
	pDriverObject->MajorFunction[IRP_MJ_CLOSE]=ddk_DispatchRoutine_CONTROL; //IRP_MJ_CREATE相关IRP处理函数
	pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL]=ddk_DispatchRoutine_CONTROL; //IRP_MJ_CREATE相关IRP处理函数
	CreateMyDevice(pDriverObject);//创建相应的设备
	pDriverObject->DriverUnload=DDK_Unload;
	return (1);
}
//#pragma code_seg("PAGE")
#pragma PAGECODE
VOID DDK_Unload (IN PDRIVER_OBJECT pDriverObject)
{ 
  PDEVICE_OBJECT pDev;//用来取得要删除设备对象
  UNICODE_STRING symLinkName; // 


	UnHookInt3Proc();		//IDT UNHOOK
  
 
  pDev=pDriverObject->DeviceObject;
  IoDeleteDevice(pDev); //删除设备
  
  //取符号链接名字
   RtlInitUnicodeString(&symLinkName,L"\\??\\My_DriverLinkName");
  //删除符号链接
   IoDeleteSymbolicLink(&symLinkName);
   KdPrint(("驱动成功被卸载...OK-----------")); //sprintf,printf
	//取得要删除设备对象
	//删掉所有设备
	DbgPrint("卸载成功");
}
#pragma PAGECODE
NTSTATUS ddk_DispatchRoutine_CONTROL(IN PDEVICE_OBJECT pDevobj,IN PIRP pIrp	)
{   //
	ULONG info;
	int *pi=(int*)ExAllocatePool(PagedPool,sizeof(int));
	//得到当前栈指针
	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(pIrp);
	ULONG mf=stack->MajorFunction;//区分IRP
	switch (mf)
	{
	case IRP_MJ_DEVICE_CONTROL:
		{ 	KdPrint(("Enter myDriver_DeviceIOControl\n"));
		NTSTATUS status = STATUS_SUCCESS;	

		//得到输入缓冲区大小
		ULONG cbin = stack->Parameters.DeviceIoControl.InputBufferLength;
		//得到输出缓冲区大小
		ULONG cbout = stack->Parameters.DeviceIoControl.OutputBufferLength;
		//得到IOCTL码
		ULONG code = stack->Parameters.DeviceIoControl.IoControlCode;
		switch (code)
		{ 
		case add_code:
			{  		
				int a,b;
				KdPrint(("add_code 1111111111111111111\n"));
				//缓冲区方式IOCTL
				//获取缓冲区数据	a,b		
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
				//C、驱动层返回数据至用户层
				//操作输出缓冲区
				int* OutputBuffer = (int*)pIrp->AssociatedIrp.SystemBuffer;
				_asm
				{
					    mov eax,a
						mov ebx,OutputBuffer
						mov [ebx],eax //bufferet=a+b

				}
				KdPrint(("a+b=%d \n",a));

				//设置实际操作输出缓冲区长度
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

	//对相应的IPR进行处理
	pIrp->IoStatus.Information=info;//设置操作的字节数为0，这里无实际意义
	pIrp->IoStatus.Status=STATUS_SUCCESS;//返回成功
	IoCompleteRequest(pIrp,IO_NO_INCREMENT);//指示完成此IRP
	KdPrint(("离开派遣函数\n"));//调试信息
	return STATUS_SUCCESS; //返回成功
}