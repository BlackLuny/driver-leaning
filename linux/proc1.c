#include<linux/init.h>
#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/proc_fs.h>   /* read_proc需要的头文件。 */

MODULE_LICENSE("GPL");
/*
    原型函数：int (*read_proc)(char *page, char **start, off_t offset, int count, int *eof, void *data)
    这个函数与read的系统调用函数功能类似。就在/proc中为驱动程序设计了一个特有了文件（假设名scull）后，则用户使用cat /proc/scull时，会调用到此函数。
    在/proc中创建文件，并且完成此文件与一个read_proc的关联关系的函数是create_proc_read_entry.
    删除这种关系，并且删除这个文件的函数是remove_proc_entry.
    实体函数：scull_read_procmem(char *buf, char **start, off_t offset, int count, int *eof, void *data) 
parameter:
    buf:是从驱动层向应用层返回的数据区；当有用户读此/proc/xxx的文件时，由系统分配一页的缓存区，驱动使用read_proc此写入数据。
    start: 表示写在此页的哪里，此用法复杂，如果是返回小量数据（不超过一页）赋值为NULL。
    offset:与read用法一致，表示文件指针的偏移。
    count:与read用法一致，表示要读多少个字节。
    eof:  输出参数。
    data:由驱动内部使用。
return:
    返回值是可读到的字节数。
*/
int scull_read_procmem(char *buf, char **start, off_t offset, int count, int *eof, void *data) 
{
    int len = 0;
    
    len += sprintf(buf + len, "I am peter\n and thank you\n");
    *eof = 1;
    return len;
}

static int func1(void)
{
        printk("In Func: %s...\n",__func__);
        
        return 0;
}

EXPORT_SYMBOL(func1);

static int __init hello_init(void)
{
/* 当你定义了read_proc函数，你就需要将它连接到/proc目录中的某个条目。这个工作可以借助一个叫做create_proc_read_entry的函数完成
struct proc_dir_entry *create_proc_read_entry(const char *name, mode_t mode, struct proc_dir_entry *base, read_proc_t *read_proc, void *data)
参数：
name是要创建的/proc文件名，
mode是文件的保护掩码（0为系统缺省值），
base指明创建文件的目录(如果设置为NULL，文件就创建在/proc根目录下)，
read_proc是read_proc函数的入口地址，
data被内核忽略（但会被传递给read_proc函数）。
*/
    create_proc_read_entry("scullmem",
        0 /* default mode */,
        NULL /* parent dir */,
        scull_read_procmem,
        NULL /* client data */);
        
        printk("Module 1,Init!\n");
        return 0;
}

static void __exit hello_exit(void)
{
    remove_proc_entry("scullmem", NULL /* parent dir */); 
    printk("Module 1,Exit!\n");
}

module_init(hello_init);
module_exit(hello_exit);