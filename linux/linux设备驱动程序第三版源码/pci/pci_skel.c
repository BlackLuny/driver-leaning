#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/init.h>


//ָ��������������������Щpci�豸
static struct pci_device_id ids[] = {
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82801AA_3), },
	{ 0, }
};
MODULE_DEVICE_TABLE(pci, ids);
/*
MODULE_DEVICE_TABLE (usb, skel_table);
�ú�����һ����Ϊ__mod_pci_device_table�ľֲ��������ñ���ָ��ڶ��������ں˹���ʱ��depmod�����������ģ������������__mod_pci_device_table��
�����ݣ��豸�б���ģ���г������ӵ�ӳ���ļ�/lib/modules/KERNEL_VERSION/modules.pcimap�У���depmod����֮�����е�PCI�豸��ͬ���ǵ�ģ�����ֶ������ļ��г���
���ں˸�֪�Ȳ��ϵͳһ���µ�PCI�豸������ʱ���Ȳ��ϵͳʹ��modules.pcimap�ļ�����Ѱǡ������������  
MODULE_DEVICE_TABLE�ĵ�һ���������豸�����ͣ������USB�豸������Ȼ��usb�������PCI�豸���ǽ���pci����������ϵͳ��ͬһ������ע����֧�ֵ��豸����
����һ���������豸������豸������һ��Ԫ���ǿյģ����ڱ�ʶ����������������붨����USB_SKEL_VENDOR_ID�� 0xfff0��USB_SKEL_PRODUCT_ID��0xfff0��
Ҳ����˵������һ���豸�ӵ�������ʱ��usb��ϵͳ�ͻ�������豸�� vendor ID��product ID��������ǵ�ֵ��0xfff0ʱ����ô��ϵͳ�ͻ�������ģ����Ϊ�豸��������
*/

static unsigned char skel_get_revision(struct pci_dev *dev)
{
	u8 revision;

	pci_read_config_byte(dev, PCI_REVISION_ID, &revision);
	return revision;
}

static int probe(struct pci_dev *dev, const struct pci_device_id *id)
{
	/* 
	����pci�豸
	*/
	pci_enable_device(dev);
	
	if (skel_get_revision(dev) == 0x42)
		return -ENODEV;


	return 0;
}

static void remove(struct pci_dev *dev)
{
	/* clean up any allocated resources and stuff here.
	 * like call release_region();
	 */
}

//�豸ģ����Ϣ
static struct pci_driver pci_driver =
{
	.name = "pci_skel", //�豸����
	.id_table = ids,	//�ܹ��������豸�б�
	.probe = probe,		//���Ҳ���ʼ���豸
	.remove = remove,	//ж���豸
	//.suspend ָ��һ����������ָ�� ��ѡ
	//resume ָ��һ���ָ�������ָ��   ��ѡ
};

static int __init pci_skel_init(void)
{
	/* ע��Ӳ���������� */
	return pci_register_driver(&pci_driver);
}

static void __exit pci_skel_exit(void)
{
	//�Ƴ���ǰ�󶨵�������������κ�pci�豸
	pci_unregister_driver(&pci_driver);
}

MODULE_LICENSE("GPL");

module_init(pci_skel_init);
module_exit(pci_skel_exit);
