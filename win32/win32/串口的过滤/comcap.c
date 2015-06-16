#include <ntddk.h>
#include <ntstrsafe.h>
#define NTSTRSAFE_LIB
#define CCP_MAX_COM_ID 32

//�������� /Device/Serial0 �Դ����� 

// �����豸����ʵ�豸
static PDEVICE_OBJECT s_fltobj[CCP_MAX_COM_ID] = { 0 };
static PDEVICE_OBJECT s_nextobj[CCP_MAX_COM_ID] = { 0 };

// ��һ���˿��豸
PDEVICE_OBJECT ccpOpenCom(ULONG id,NTSTATUS *status)
{
	UNICODE_STRING name_str;
	static WCHAR name[32] = { 0 };
	PFILE_OBJECT fileobj = NULL;
	PDEVICE_OBJECT devobj = NULL;

	// �����ַ�����
	memset(name,0,sizeof(WCHAR)*32);
	//��ʽ����ӡ�ַ��� 
	RtlStringCchPrintfW(name,	//Ҫ��ʽ����ӡ�ַ����ĵ�ַ 
	32,							//name�ĳ��� 
	L"\\Device\\Serial%d",		//��ʽ�� 
	id							//%d�Ĳ��� 
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
    
    ��ʼ�� UNICODE_STRING �ṹ 
    VOID 
  	RtlInitUnicodeString(
    IN OUT PUNICODE_STRING  DestinationString,
    IN PCWSTR  SourceString
    );
	*/
	// ���豸����
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

//�豸�� 
NTSTATUS
ccpAttachDevice(
				PDRIVER_OBJECT driver, 
				PDEVICE_OBJECT oldobj,
				PDEVICE_OBJECT *fltobj, 
				PDEVICE_OBJECT *next)
{
	NTSTATUS status;
	PDEVICE_OBJECT topdev = NULL;

	// �����豸��Ȼ���֮��
	status = IoCreateDevice(driver,
		0,
		NULL,
		oldobj->DeviceType,
		0,
		FALSE,
		fltobj);
	/*
	IoCreateDevice�������ڴ���������豸����.
����NTSTATUS IoCreateDevice
����(
����IN PDRIVER_OBJECT DriverObject,
����IN ULONG DeviceExtensionSize,
����IN PUNICODE_STRING DeviceNameOPTIONAL,
����IN DEVICE_TYPE DeviceType,
����IN ULONG DeviceCharacteristics,
����IN BOOLEAN Exclusive,
����OUT PDEVICE_OBJECT *DeviceObject
����);
	DriverObject
����һ��ָ����øú����������������.ÿһ����������������DriverEntry���������һ��ָ�����������������.
����WDM���ܺ͹�����������Ҳ�����ǵ�AddDevice���̽���һ��������������ָ��.
����DeviceExtensionSize
����ָ����������Ϊ�豸��չ���������Ľṹ��Ĵ�С.
����DeviceName
����(��ѡ�Ĳ���)ָ��һ�������β�İ���Unicode�ַ����Ļ�����,��������豸������,���ַ���������һ���������豸·����.
����WDM������������͹��������������ǵ��豸����û������.
������ע��:����豸��δ�ṩ(�����������NULL),IoCreateDevice�������豸���󽫲�����һ��DACL��֮�����.
����DeviceType
����ָ��һ����һ��ϵͳ�����FILE_DEVICE_XXX����,����������豸������
����(��FILE_DEVICE_DISK,FILE_DEVICE_KEYBOARD��),��Ӧ�̶����һ�������豸������.
����DeviceCharacteristics
����ָ��һ������ϵͳ����ĳ���,������һ��,�ṩ�й�����������豸������Ϣ.���ڿ��ܵ��豸������Ϣ,
������DEVICE_OBJECT�ṹ��.
����Exclusive
�������ָ���豸�Ƕ�ռ��,�󲿷����������������ֵΪFALSE,����Ƕ�ռ�Ļ�����ΪTRUE���Ƕ�ռ����ΪFALSE.
����DeviceObject
����һ��ָ��DEVICE_OBJECT�ṹ��ָ���ָ��,����һ��ָ���ָ��,ָ���ָ����������DEVICE_OBJECT�ṹ���ָ��.
	*/
	if (status != STATUS_SUCCESS)
		return status;

	// ������Ҫ��־λ��
	if(oldobj->Flags & DO_BUFFERED_IO)
		(*fltobj)->Flags |= DO_BUFFERED_IO;
	if(oldobj->Flags & DO_DIRECT_IO)
		(*fltobj)->Flags |= DO_DIRECT_IO;
	if(oldobj->Flags & DO_BUFFERED_IO)
		(*fltobj)->Flags |= DO_BUFFERED_IO;
	if(oldobj->Characteristics & FILE_DEVICE_SECURE_OPEN)
		(*fltobj)->Characteristics |= FILE_DEVICE_SECURE_OPEN;
	(*fltobj)->Flags |=  DO_POWER_PAGABLE;
	// ��һ���豸����һ���豸��
	topdev = IoAttachDeviceToDeviceStack(*fltobj,oldobj);
	/*	
	PDEVICE_OBJECT 
	IoAttachDeviceToDeviceStack(
	IN PDEVICE_OBJECT  SourceDevice,	// �����豸 
	IN PDEVICE_OBJECT  TargetDevice		// Ҫ���󶨵��豸ջ�е��豸 
	);
	// �������ձ��󶨵��豸 
	*/
	if (topdev == NULL)
	{
		// �����ʧ���ˣ������´����������豸������������
		IoDeleteDevice(*fltobj);
		*fltobj = NULL;
		status = STATUS_UNSUCCESSFUL;
		return status;
	}
	//���豸ջ�������豸ָ�븳�� *next
	*next = topdev;

	// ��������豸�Ѿ�������
	(*fltobj)->Flags = (*fltobj)->Flags & ~DO_DEVICE_INITIALIZING;
	return STATUS_SUCCESS;
}

// ������������еĴ��ڡ�
void ccpAttachAllComs(PDRIVER_OBJECT driver)
{
	ULONG i;
	PDEVICE_OBJECT com_ob;
	NTSTATUS status;
	for(i = 0;i<CCP_MAX_COM_ID;i++)
	{
		// ���object���á�
		com_ob = ccpOpenCom(i,&status);
		if(com_ob == NULL)
			continue;
		// ������󶨡������ܰ��Ƿ�ɹ���
		ccpAttachDevice(driver,com_ob,&s_fltobj[i],&s_nextobj[i]);
		// ȡ��object���á�
	}
}

#define  DELAY_ONE_MICROSECOND  (-10)
#define  DELAY_ONE_MILLISECOND (DELAY_ONE_MICROSECOND*1000)
#define  DELAY_ONE_SECOND (DELAY_ONE_MILLISECOND*1000)

//ж������ 
void ccpUnload(PDRIVER_OBJECT drv)
{
	ULONG i;
	LARGE_INTEGER interval;

	// ���Ƚ����
	for(i=0;i<CCP_MAX_COM_ID;i++)
	{
		if(s_nextobj[i] != NULL)
			IoDetachDevice(s_nextobj[i]);
	}

	// ˯��5�롣�ȴ�����irp�������
	interval.QuadPart = (5*1000 * DELAY_ONE_MILLISECOND);		
	KeDelayExecutionThread(KernelMode,FALSE,&interval);

	// ɾ����Щ�豸
	for(i=0;i<CCP_MAX_COM_ID;i++)
	{
		if(s_fltobj[i] != NULL)
			IoDeleteDevice(s_fltobj[i]);
	}
}

//�ַ����� 
NTSTATUS ccpDispatch(PDEVICE_OBJECT device,PIRP irp)
{
	PIO_STACK_LOCATION irpsp = IoGetCurrentIrpStackLocation(irp);
	irpsp->Parameters.DeviceIoControl.IoControlCode
	irpsp->Parameters.DeviceIoControl.InputBufferLength
	irpsp->Parameters.AssociatedIrp.SystemBuffer
	NTSTATUS status;
	ULONG i,j;
	// ���ȵ�֪�����͸����ĸ��豸���豸һ�����CCP_MAX_COM_ID
	// ������ǰ��Ĵ��뱣��õģ�����s_fltobj�С�
	for(i=0;i<CCP_MAX_COM_ID;i++)
	{
		if(s_fltobj[i] == device)
		{			
			// ���е�Դ������ȫ��ֱ�ӷŹ���
			if(irpsp->MajorFunction == IRP_MJ_POWER)
			{
				// ֱ�ӷ��ͣ�Ȼ�󷵻�˵�Ѿ��������ˡ�
				PoStartNextPowerIrp(irp);
				//PoStartNextPowerIrp֪ͨ��Դ���������Է�����һ����ԴIRP
				IoSkipCurrentIrpStackLocation(irp);
				//IoSkipCurrentIrpStackLocation���þ���ʹIO_STACK_LOCATIONָ����ǰ��һ��
				return PoCallDriver(s_nextobj[i],irp);
				/*PoCallDriver������ʹIO_STACK_LOCATIONָ����ǰһ�����к͵Ľ������IO_STACK_LOCATIONָ�벻�� 
				����һ�������������ǲ���̵���IoGetCurrentIrpStackLocationʱ�������յ���������ʹ 
				�õ���ȫ��ͬ��IO_STACK_LOCATIONָ��*/
			}
			// ��������ֻ����д����д����Ļ�����û������Լ��䳤�ȡ�
			// Ȼ���ӡһ�¡�
			if(irpsp->MajorFunction == IRP_MJ_WRITE)
			{
				// �����д���Ȼ�ó���
				ULONG len = irpsp->Parameters.Write.Length;
				// Ȼ���û�����
				PUCHAR buf = NULL;
				if(irp->MdlAddress != NULL)
					//ֱ�ӷ�ʽ 
					buf = (PUCHAR)MmGetSystemAddressForMdlSafe(irp->MdlAddress,NormalPagePriority);
				else
					//������ʽ 
					buf = (PUCHAR)irp->UserBuffer;
					//���巽ʽ 
				if(buf == NULL)
					buf = (PUCHAR)irp->AssociatedIrp.SystemBuffer;
				// ��ӡ����
				for(j=0;j<len;++j)
				{
					KdPrint(("comcap: Send Data: %2x\r\n",
						buf[j]));
				}
			}

			// ��Щ����ֱ���·�ִ�м��ɡ����ǲ�����ֹ���߸ı�����
			IoSkipCurrentIrpStackLocation(irp);
			return IoCallDriver(s_nextobj[i],irp);
		}
	}

	// ��������Ͳ��ڱ��󶨵��豸�У�����������ģ�ֱ�ӷ��ز�������
	irp->IoStatus.Information = 0;
	irp->IoStatus.Status = STATUS_INVALID_PARAMETER;
	IoCompleteRequest(irp,IO_NO_INCREMENT);
	return STATUS_SUCCESS;	
}

NTSTATUS DriverEntry(PDRIVER_OBJECT driver, PUNICODE_STRING reg_path)
{
	ULONG i;
	// ���еķַ����������ó�һ���ġ�
	for(i=0;i<IRP_MJ_MAXIMUM_FUNCTION;i++)
	{
		driver->MajorFunction[i] = ccpDispatch;
	}

	// ֧�ֶ�̬ж�ء�
	driver->DriverUnload = ccpUnload;

	// �����еĴ��ڡ�
	ccpAttachAllComs(driver);

	// ֱ�ӷ��سɹ����ɡ�
	return STATUS_SUCCESS;
}