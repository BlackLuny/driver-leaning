#include<linux/init.h>
#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/proc_fs.h>   /* read_proc��Ҫ��ͷ�ļ��� */

MODULE_LICENSE("GPL");
/*
    ԭ�ͺ�����int (*read_proc)(char *page, char **start, off_t offset, int count, int *eof, void *data)
    ���������read��ϵͳ���ú����������ơ�����/proc��Ϊ�������������һ���������ļ���������scull�������û�ʹ��cat /proc/scullʱ������õ��˺�����
    ��/proc�д����ļ���������ɴ��ļ���һ��read_proc�Ĺ�����ϵ�ĺ�����create_proc_read_entry.
    ɾ�����ֹ�ϵ������ɾ������ļ��ĺ�����remove_proc_entry.
    ʵ�庯����scull_read_procmem(char *buf, char **start, off_t offset, int count, int *eof, void *data) 
parameter:
    buf:�Ǵ���������Ӧ�ò㷵�ص��������������û�����/proc/xxx���ļ�ʱ����ϵͳ����һҳ�Ļ�����������ʹ��read_proc��д�����ݡ�
    start: ��ʾд�ڴ�ҳ��������÷����ӣ�����Ƿ���С�����ݣ�������һҳ����ֵΪNULL��
    offset:��read�÷�һ�£���ʾ�ļ�ָ���ƫ�ơ�
    count:��read�÷�һ�£���ʾҪ�����ٸ��ֽڡ�
    eof:  ���������
    data:�������ڲ�ʹ�á�
return:
    ����ֵ�ǿɶ������ֽ�����
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
/* ���㶨����read_proc�����������Ҫ�������ӵ�/procĿ¼�е�ĳ����Ŀ������������Խ���һ������create_proc_read_entry�ĺ������
struct proc_dir_entry *create_proc_read_entry(const char *name, mode_t mode, struct proc_dir_entry *base, read_proc_t *read_proc, void *data)
������
name��Ҫ������/proc�ļ�����
mode���ļ��ı������루0Ϊϵͳȱʡֵ����
baseָ�������ļ���Ŀ¼(�������ΪNULL���ļ��ʹ�����/proc��Ŀ¼��)��
read_proc��read_proc��������ڵ�ַ��
data���ں˺��ԣ����ᱻ���ݸ�read_proc��������
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