#include <ntddk.h>
#include <ntstrsafe.h>
#define NTSTRSAFE_LIB
#define CCP_MAX_COM_ID 32

//串口名称 /Device/Serial0 以此类推 

// 过滤设备和真实设备
static PDEVICE_OBJECT s_fltobj[CCP_MAX_COM_ID] = { 0 };
static PDEVICE_OBJECT s_nextobj[CCP_MAX_COM_ID] = { 0 };

// 打开一个端口设备
PDEVICE_OBJECT ccpOpenCom(ULONG id,NTSTATUS *status)
{
	UNICODE_STRING name_str;
	static WCHAR name[32] = { 0 };
	PFILE_OBJECT fileobj = NULL;
	PDEVICE_OBJECT devobj = NULL;

	// 输入字符串。
	memset(name,0,sizeof(WCHAR)*32);
	//格式化打印字符串 
	RtlStringCchPrintfW(name,	//要格式化打印字符串的地址 
	32,							//name的长度 
	L"\\Device\\Serial%d",		//格式化 
	id							//%d的参数 
	);
	RtlInitUnicodeString(&name_str,name);
	/*
	
	NTSTATUS
  	RtlStringCchVPrintfW(
    OUT LPWSTR  pszDest,
    IN size_t  cchDest,
    IN LPCWSTR  pszFormat,
    IN va_list  argList
    );
    
    初始化 UNICODE_STRING 结构 
    VOID 
  	RtlInitUnicodeString(
    IN OUT PUNICODE_STRING  DestinationString,
    IN PCWSTR  SourceString
    );
	*/
	// 打开设备对象
	*status = IoGetDeviceObjectPointer(&name_str, FILE_ALL_ACCESS, &fileobj, &devobj);
	if (*status == STATUS_SUCCESS)
		ObDereferenceObject(fileobj);
	/*
	NTSTATUS 
  	IoGetDeviceObjectPointer(
    IN PUNICODE_STRING  ObjectName,
    IN ACCESS_MASK  DesiredAccess,
    OUT PFILE_OBJECT  *FileObject,
    OUT PDEVICE_OBJECT  *DeviceObject
    );
	*/
	return devobj;
}

//设备绑定 
NTSTATUS
ccpAttachDevice(
				PDRIVER_OBJECT driver, 
				PDEVICE_OBJECT oldobj,
				PDEVICE_OBJECT *fltobj, 
				PDEVICE_OBJECT *next)
{
	NTSTATUS status;
	PDEVICE_OBJECT topdev = NULL;

	// 生成设备，然后绑定之。
	status = IoCreateDevice(driver,
		0,
		NULL,
		oldobj->DeviceType,
		0,
		FALSE,
		fltobj);
	/*
	IoCreateDevice函数用于创建常规的设备对象.
　　NTSTATUS IoCreateDevice
　　(
　　IN PDRIVER_OBJECT DriverObject,
　　IN ULONG DeviceExtensionSize,
　　IN PUNICODE_STRING DeviceNameOPTIONAL,
　　IN DEVICE_TYPE DeviceType,
　　IN ULONG DeviceCharacteristics,
　　IN BOOLEAN Exclusive,
　　OUT PDEVICE_OBJECT *DeviceObject
　　);
	DriverObject
　　一个指向调用该函数的驱动程序对象.每一个驱动程序在它的DriverEntry过程里接收一个指向它的驱动程序对象.
　　WDM功能和过滤驱动程序也在他们的AddDevice过程接受一个驱动程序对象的指针.
　　DeviceExtensionSize
　　指定驱动程序为设备扩展对象而定义的结构体的大小.
　　DeviceName
　　(可选的参数)指向一个以零结尾的包含Unicode字符串的缓冲区,那是这个设备的名称,该字符串必须是一个完整的设备路径名.
　　WDM功能驱动程序和过滤驱动程序它们的设备对象没有名字.
　　　注意:如果设备名未提供(即这个参数是NULL),IoCreateDevice创建的设备对象将不会有一个DACL与之相关联.
　　DeviceType
　　指定一个由一个系统定义的FILE_DEVICE_XXX常量,表明了这个设备的类型
　　(如FILE_DEVICE_DISK,FILE_DEVICE_KEYBOARD等),或供应商定义的一种新型设备的类型.
　　DeviceCharacteristics
　　指定一个或多个系统定义的常量,连接在一起,提供有关驱动程序的设备其他信息.对于可能的设备特征信息,
　　见DEVICE_OBJECT结构体.
　　Exclusive
　　如果指定设备是独占的,大部分驱动程序设置这个值为FALSE,如果是独占的话设置为TRUE，非独占设置为FALSE.
　　DeviceObject
　　一个指向DEVICE_OBJECT结构体指针的指针,这是一个指针的指针,指向的指针用来接收DEVICE_OBJECT结构体的指针.
	*/
	if (status != STATUS_SUCCESS)
		return status;

	// 拷贝重要标志位。
	if(oldobj->Flags & DO_BUFFERED_IO)
		(*fltobj)->Flags |= DO_BUFFERED_IO;
	if(oldobj->Flags & DO_DIRECT_IO)
		(*fltobj)->Flags |= DO_DIRECT_IO;
	if(oldobj->Flags & DO_BUFFERED_IO)
		(*fltobj)->Flags |= DO_BUFFERED_IO;
	if(oldobj->Characteristics & FILE_DEVICE_SECURE_OPEN)
		(*fltobj)->Characteristics |= FILE_DEVICE_SECURE_OPEN;
	(*fltobj)->Flags |=  DO_POWER_PAGABLE;
	// 绑定一个设备到另一个设备上
	topdev = IoAttachDeviceToDeviceStack(*fltobj,oldobj);
	/*	
	PDEVICE_OBJECT 
	IoAttachDeviceToDeviceStack(
	IN PDEVICE_OBJECT  SourceDevice,	// 过滤设备 
	IN PDEVICE_OBJECT  TargetDevice		// 要被绑定的设备栈中的设备 
	);
	// 返回最终被绑定的设备 
	*/
	if (topdev == NULL)
	{
		// 如果绑定失败了，销毁新创建的虚拟设备，重新来过。
		IoDeleteDevice(*fltobj);
		*fltobj = NULL;
		status = STATUS_UNSUCCESSFUL;
		return status;
	}
	//把设备栈顶部的设备指针赋给 *next
	*next = topdev;

	// 设置这个设备已经启动。
	(*fltobj)->Flags = (*fltobj)->Flags & ~DO_DEVICE_INITIALIZING;
	return STATUS_SUCCESS;
}

// 这个函数绑定所有的串口。
void ccpAttachAllComs(PDRIVER_OBJECT driver)
{
	ULONG i;
	PDEVICE_OBJECT com_ob;
	NTSTATUS status;
	for(i = 0;i<CCP_MAX_COM_ID;i++)
	{
		// 获得object引用。
		com_ob = ccpOpenCom(i,&status);
		if(com_ob == NULL)
			continue;
		// 在这里绑定。并不管绑定是否成功。
		ccpAttachDevice(driver,com_ob,&s_fltobj[i],&s_nextobj[i]);
		// 取消object引用。
	}
}

#define  DELAY_ONE_MICROSECOND  (-10)
#define  DELAY_ONE_MILLISECOND (DELAY_ONE_MICROSECOND*1000)
#define  DELAY_ONE_SECOND (DELAY_ONE_MILLISECOND*1000)

//卸载例程 
void ccpUnload(PDRIVER_OBJECT drv)
{
	ULONG i;
	LARGE_INTEGER interval;

	// 首先解除绑定
	for(i=0;i<CCP_MAX_COM_ID;i++)
	{
		if(s_nextobj[i] != NULL)
			IoDetachDevice(s_nextobj[i]);
	}

	// 睡眠5秒。等待所有irp处理结束
	interval.QuadPart = (5*1000 * DELAY_ONE_MILLISECOND);		
	KeDelayExecutionThread(KernelMode,FALSE,&interval);

	// 删除这些设备
	for(i=0;i<CCP_MAX_COM_ID;i++)
	{
		if(s_fltobj[i] != NULL)
			IoDeleteDevice(s_fltobj[i]);
	}
}

//分发函数 
NTSTATUS ccpDispatch(PDEVICE_OBJECT device,PIRP irp)
{
	PIO_STACK_LOCATION irpsp = IoGetCurrentIrpStackLocation(irp);
	irpsp->Parameters.DeviceIoControl.IoControlCode
	irpsp->Parameters.DeviceIoControl.InputBufferLength
	irpsp->Parameters.AssociatedIrp.SystemBuffer
	NTSTATUS status;
	ULONG i,j;
	// 首先得知道发送给了哪个设备。设备一共最多CCP_MAX_COM_ID
	// 个，是前面的代码保存好的，都在s_fltobj中。
	for(i=0;i<CCP_MAX_COM_ID;i++)
	{
		if(s_fltobj[i] == device)
		{			
			// 所有电源操作，全部直接放过。
			if(irpsp->MajorFunction == IRP_MJ_POWER)
			{
				// 直接发送，然后返回说已经被处理了。
				PoStartNextPowerIrp(irp);
				//PoStartNextPowerIrp通知电源管理器可以发送下一个电源IRP
				IoSkipCurrentIrpStackLocation(irp);
				//IoSkipCurrentIrpStackLocation作用就是使IO_STACK_LOCATION指针少前进一步
				return PoCallDriver(s_nextobj[i],irp);
				/*PoCallDriver函数会使IO_STACK_LOCATION指针向前一步，中和的结果就是IO_STACK_LOCATION指针不变 
				当下一个驱动程序的派遣例程调用IoGetCurrentIrpStackLocation时，它将收到与我们正使 
				用的完全相同的IO_STACK_LOCATION指针*/
			}
			// 此外我们只过滤写请求。写请求的话，获得缓冲区以及其长度。
			// 然后打印一下。
			if(irpsp->MajorFunction == IRP_MJ_WRITE)
			{
				// 如果是写，先获得长度
				ULONG len = irpsp->Parameters.Write.Length;
				// 然后获得缓冲区
				PUCHAR buf = NULL;
				if(irp->MdlAddress != NULL)
					//直接方式 
					buf = (PUCHAR)MmGetSystemAddressForMdlSafe(irp->MdlAddress,NormalPagePriority);
				else
					//其他方式 
					buf = (PUCHAR)irp->UserBuffer;
					//缓冲方式 
				if(buf == NULL)
					buf = (PUCHAR)irp->AssociatedIrp.SystemBuffer;
				// 打印内容
				for(j=0;j<len;++j)
				{
					KdPrint(("comcap: Send Data: %2x\r\n",
						buf[j]));
				}
			}

			// 这些请求直接下发执行即可。我们并不禁止或者改变它。
			IoSkipCurrentIrpStackLocation(irp);
			return IoCallDriver(s_nextobj[i],irp);
		}
	}

	// 如果根本就不在被绑定的设备中，那是有问题的，直接返回参数错误。
	irp->IoStatus.Information = 0;
	irp->IoStatus.Status = STATUS_INVALID_PARAMETER;
	IoCompleteRequest(irp,IO_NO_INCREMENT);
	return STATUS_SUCCESS;	
}

NTSTATUS DriverEntry(PDRIVER_OBJECT driver, PUNICODE_STRING reg_path)
{
	ULONG i;
	// 所有的分发函数都设置成一样的。
	for(i=0;i<IRP_MJ_MAXIMUM_FUNCTION;i++)
	{
		driver->MajorFunction[i] = ccpDispatch;
	}

	// 支持动态卸载。
	driver->DriverUnload = ccpUnload;

	// 绑定所有的串口。
	ccpAttachAllComs(driver);

	// 直接返回成功即可。
	return STATUS_SUCCESS;
}