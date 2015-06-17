void *kmalloc(size_t size, int flags)分配的内存物理地址上连续，虚拟地址上自然连续  用于申请较小的、连续的物理内存
硬件设备需要物理地址连续的内存，因为硬件设备往往存在于MMU之外，根本不了解虚拟地址

void *vmalloc(unsigned long size) 分配的内存虚拟地址上连续，物理地址不连续         用于申请较大的内存空间，虚拟内存是连续的
void vfree(void* addr) 释放从addr开始的内存块 这个函数可以睡眠，因此不能再中断上下文中使用