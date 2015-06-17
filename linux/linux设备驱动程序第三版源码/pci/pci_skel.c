#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/init.h>


//指明该驱动程序适用于那些pci设备
static struct pci_device_id ids[] = {
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82801AA_3), },
	{ 0, }
};
MODULE_DEVICE_TABLE(pci, ids);
/*
MODULE_DEVICE_TABLE (usb, skel_table);
该宏生成一个名为__mod_pci_device_table的局部变量，该变量指向第二个参数内核构建时，depmod程序会在所有模块中搜索符号__mod_pci_device_table，
把数据（设备列表）从模块中抽出，添加到映射文件/lib/modules/KERNEL_VERSION/modules.pcimap中，当depmod结束之后，所有的PCI设备连同他们的模块名字都被该文件列出。
当内核告知热插拔系统一个新的PCI设备被发现时，热插拔系统使用modules.pcimap文件来找寻恰当的驱动程序  
MODULE_DEVICE_TABLE的第一个参数是设备的类型，如果是USB设备，那自然是usb（如果是PCI设备，那将是pci，这两个子系统用同一个宏来注册所支持的设备）。
后面一个参数是设备表，这个设备表的最后一个元素是空的，用于标识结束。例：假如代码定义了USB_SKEL_VENDOR_ID是 0xfff0，USB_SKEL_PRODUCT_ID是0xfff0，
也就是说，当有一个设备接到集线器时，usb子系统就会检查这个设备的 vendor ID和product ID，如果他们的值是0xfff0时，那么子系统就会调用这个模块作为设备的驱动。
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
	激活pci设备
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

//设备模块信息
static struct pci_driver pci_driver =
{
	.name = "pci_skel", //设备名称
	.id_table = ids,	//能够驱动的设备列表
	.probe = probe,		//查找并初始化设备
	.remove = remove,	//卸载设备
	//.suspend 指向一个挂起函数的指针 可选
	//resume 指向一个恢复函数的指针   可选
};

static int __init pci_skel_init(void)
{
	/* 注册硬件驱动程序 */
	return pci_register_driver(&pci_driver);
}

static void __exit pci_skel_exit(void)
{
	//移除当前绑定到该驱动程序的任何pci设备
	pci_unregister_driver(&pci_driver);
}

MODULE_LICENSE("GPL");

module_init(pci_skel_init);
module_exit(pci_skel_exit);
