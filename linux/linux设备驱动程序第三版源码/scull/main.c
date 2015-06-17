/*
 * main.c -- the bare scull char module
 *
 * Copyright (C) 2001 Alessandro Rubini and Jonathan Corbet
 * Copyright (C) 2001 O'Reilly & Associates
 *
 * The source code in this file can be freely used, adapted,
 * and redistributed in source or binary form, so long as an
 * acknowledgment appears in derived source files.  The citation
 * should list that the code comes from the book "Linux Device
 * Drivers" by Alessandro Rubini and Jonathan Corbet, published
 * by O'Reilly & Associates.   No warranty is attached;
 * we cannot take responsibility for errors or fitness for use.
 *
 */

#include <linux/config.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>

#include <linux/kernel.h>   /* printk() */
#include <linux/slab.h>     /* kmalloc() */
#include <linux/fs.h>       /* everything... */
#include <linux/errno.h>    /* error codes */
#include <linux/types.h>    /* size_t */
#include <linux/proc_fs.h>
#include <linux/fcntl.h>    /* O_ACCMODE */
#include <linux/seq_file.h>
#include <linux/cdev.h>

#include <asm/system.h>     /* cli(), *_flags */
#include <asm/uaccess.h>    /* copy_*_user */

#include "scull.h"      /* local definitions */

/*
 * Our parameters which can be set at load time.
 */

int scull_major =   SCULL_MAJOR;
int scull_minor =   0;
int scull_nr_devs = SCULL_NR_DEVS;  /* number of bare scull devices */
int scull_quantum = SCULL_QUANTUM;
int scull_qset =    SCULL_QSET;

module_param(scull_major, int, S_IRUGO);
module_param(scull_minor, int, S_IRUGO);
module_param(scull_nr_devs, int, S_IRUGO);
module_param(scull_quantum, int, S_IRUGO);
module_param(scull_qset, int, S_IRUGO);

MODULE_AUTHOR("Alessandro Rubini, Jonathan Corbet");
MODULE_LICENSE("Dual BSD/GPL");

struct scull_dev *scull_devices;    /* allocated in scull_init_module */


/*
 * Empty out the scull device; must be called with the device
 * semaphore held.
 */
 /*  
 * 释放整个数据区，简单遍历列表并且释放它发现的任何量子和量子集。  
 * 在scull_open在文件为写而打开时调用。  
 * 调用这个函数时必须持有信号量。  
 */  
int scull_trim(struct scull_dev *dev)
{
    struct scull_qset *next, *dptr;
    //量子集大小
    int qset = dev->qset;   /* "dev" is not-null */
    int i;

    for (dptr = dev->data; dptr; dptr = next) /* all the list items */
    {
        if (dptr->data)
        {
            //量子集中有数据   
            //遍历释放当前量子集中的每个量子，量子集大小为qset   
            for (i = 0; i < qset; i++)
            {
                kfree(dptr->data[i]);
            }
            //释放量子数组指针
            kfree(dptr->data);
            dptr->data = NULL;
        }
        //next获取下一个量子集，释放当前量子集
        next = dptr->next;
        kfree(dptr);
    }
    //清理struct scull_dev dev中的变量的值
    dev->size = 0;
    dev->quantum = scull_quantum;
    dev->qset = scull_qset;
    dev->data = NULL;
    return 0;
}
#ifdef SCULL_DEBUG /* use proc only if debugging */
/*
 * The proc filesystem: function to read and entry
 这个函数与read的系统调用函数功能类似。就在/proc中为驱动程序设计了一个特有了文件（假设名scull）后，则用户使用cat /proc/scull时，会调用到此函数。
 */

int scull_read_procmem( char *buf,      //是从驱动层向应用层返回的数据区；当有用户读此/proc/xxx的文件时，由系统分配一页的缓存区，驱动使用read_proc此写入数据。
                        char **start,   //表示写在此页的哪里，此用法复杂，如果是返回小量数据（不超过一页）赋值为NULL。
                        off_t offset,   //与read用法一致，表示文件指针的偏移。
                        int count,      //与read用法一致，表示要读多少个字节。
                        int *eof,       //输出参数，指向一个整型数，当没有数据可返回时，驱动程序必须设置这个参数。
                        void *data      //由驱动内部使用,可用于内部记录。
                        )
{
    int i, j, len = 0;
    int limit = count - 80; /* Don't print more than this */

    for (i = 0; i < scull_nr_devs && len <= limit; i++)    //循环打印4个设备的信息
    {
        struct scull_dev *d = &scull_devices[i];
        struct scull_qset *qs = d->data;
        if (down_interruptible(&d->sem))
        {
            return -ERESTARTSYS;
        }
        len += sprintf(buf+len,"\nDevice %i: qset %i, q %i, sz %li\n",
                i, d->qset, d->quantum, d->size);
        for (; qs && len <= limit; qs = qs->next) /* scan the list */
        { 
            len += sprintf(buf + len, "  item at %p, qset at %p\n", qs, qs->data);
            if (qs->data && !qs->next) /* dump only the last item */
            {
                for (j = 0; j < d->qset; j++)
                {
                    if (qs->data[j])
                    {
                        len += sprintf(buf + len, "    % 4i: %8p\n", j, qs->data[j]);
                    }
                }
            }
        }
        up(&scull_devices[i].sem);
    }
    *eof = 1;
    return len;
}


/*
 * For now, the seq_file implementation will exist in parallel.  The
 * older read_procmem function should maybe go away, though.
 */

/*
 * Here are our sequence iteration methods.  Our "position" is
 * simply the device number.
 */
static void *scull_seq_start(struct seq_file *s, loff_t *pos)
{
    if (*pos >= scull_nr_devs)
    {
        return NULL;   /* No more to read */
    }
    return scull_devices + *pos;
}

static void *scull_seq_next(struct seq_file *s, void *v, loff_t *pos)
{
    (*pos)++;
    if (*pos >= scull_nr_devs)
    {
        return NULL;
    }
    return scull_devices + *pos;
}

static void scull_seq_stop(struct seq_file *s, void *v)
{
    /* Actually, there's nothing to do here */
}

static int scull_seq_show(struct seq_file *s, void *v)
{
    struct scull_dev *dev = (struct scull_dev *) v;
    struct scull_qset *d;
    int i;

    if (down_interruptible(&dev->sem))
    {
        return -ERESTARTSYS;
    }
    seq_printf(s, "\nDevice %i: qset %i, q %i, sz %li\n",
            (int) (dev - scull_devices), dev->qset,
            dev->quantum, dev->size);
    for (d = dev->data; d; d = d->next)/* scan the list */
    { 
        seq_printf(s, "  item at %p, qset at %p\n", d, d->data);
        if (d->data && !d->next) /* dump only the last item */
        {
            for (i = 0; i < dev->qset; i++)
            {
                if (d->data[i])
                {
                    seq_printf(s, "    % 4i: %8p\n", i, d->data[i]);
                }
            }
        }
    }
    up(&dev->sem);
    return 0;
}
    
/*
 * Tie the sequence operators up.
 start()：
	主要实现初始化工作，在遍历一个链接对象开始时，调用。返回一个链接对象的偏移或者SEQ_START_TOKEN（表征这是所有循环的开始）。出错返回ERR_PTR。
stop():
	当所有链接对象遍历结束时调用。主要完成一些清理工作。
next():
	用来在遍历中寻找下一个链接对象。返回下一个链接对象或者NULL（遍历结束）。
show():
	对遍历对象进行操作的函数。主要是调用seq_printf(), seq_puts()之类的函数，打印出这个对象节点的信息。
 */
static struct seq_operations scull_seq_ops = {
    .start = scull_seq_start,
    .next  = scull_seq_next,
    .stop  = scull_seq_stop,
    .show  = scull_seq_show
};

/*
 * Now to implement the /proc file we need only make an open
 * method which sets up the sequence operators.
 步骤二：3、具体实现proc文件的open操作，目的与seq_file相关联。
 */
static int scull_proc_open(struct inode *inode, struct file *file)
{
    return seq_open(file, &scull_seq_ops);
}

/*
 * Create a set of file operations for our proc file.
 */
static struct file_operations scull_proc_ops = {
    .owner   = THIS_MODULE,
    .open    = scull_proc_open,
    .read    = seq_read,
    .llseek  = seq_lseek,
    .release = seq_release
};
    

/*
 * Actually create (and remove) the /proc file(s).
 */

static void scull_create_proc(void)
{
    struct proc_dir_entry *entry;
    create_proc_read_entry("scullmem", 0 /* default mode */,
            NULL /* parent dir */,
			scull_read_procmem,
            NULL /* client data */);
	//第一步 创建proc文件
    entry = create_proc_entry("scullseq", 0, NULL);
	//第二步 将proc文件和对应的文件操作关联起来
    if (entry)
    {
        entry->proc_fops = &scull_proc_ops;
    }
}

static void scull_remove_proc(void)
{
    /* no problem if it was not registered */
	/* 我们在模块中创建的proc文件，都应该模块cleanup模块的时候删除，以防影响系统，另外，我们应该在删除模块函数的开始执行这个操作，
	防止相关联的数据已经删除或者注销后再来处理，避免异常出现。 */ 
    remove_proc_entry("scullmem", NULL /* parent dir */);
    remove_proc_entry("scullseq", NULL);
}


#endif /* SCULL_DEBUG */





/*
 * Open and close
open 完成的工作
检查设备特定的错误（诸如设备未就绪或诸如的问题）
如果设备是首次打开，则对其进行初始化
如果有必要，更新f_op指针
分配并填写置于filp->private_data里的数据结构
 */

int scull_open(struct inode *inode, struct file *filp)
{
    struct scull_dev *dev; /* device information */

    /*
    container_of 通过结构体变量中某个成员的首地址进而获得整个结构体变量的首地址
    container_of(ptr, type, member) 
    ptr:表示结构体中member的地址
    type:表示结构体类型
    member:表示结构体中的成员
    通过ptr的地址可以返回结构体的首地址
    */
    //通过struct scull_dev结构体中 struct cdev成员变量的地址得到struct scull_dev的地址
    dev = container_of(inode->i_cdev, struct scull_dev, cdev);
    filp->private_data = dev; /* for other methods */

    /* 当设备以写方式打开时，它的长度被截为0 */
    if ( (filp->f_flags & O_ACCMODE) == O_WRONLY)
    {
        if (down_interruptible(&dev->sem))
        {
            return -ERESTARTSYS;
        }
        scull_trim(dev); /* ignore errors */
        up(&dev->sem);
    }
    return 0;          /* success */
}

/*
释放由open分配的，保存在filp->private_data中的所有内容
在最后一次关闭操作时关闭设备
*/
int scull_release(struct inode *inode, struct file *filp)
{
    return 0;
}
/*
 * Follow the list
 */
 //返回设备dev的第n个量子集的指针，量子集不够n个就申请新的
struct scull_qset *scull_follow(struct scull_dev *dev, int n)
{
    //第一个量子集指针
    struct scull_qset *qs = dev->data;

    /* Allocate first qset explicitly if need be */
    
    // 如果当前设备还没有量子集，就显示分配第一个量子集   
    if (! qs)
    {
        qs = dev->data = kmalloc(sizeof(struct scull_qset), GFP_KERNEL);
        if (qs == NULL)
        {
            return NULL;  /* Never mind */
        }
        memset(qs, 0, sizeof(struct scull_qset));
    }

    /* Then follow the list */
    // 遍历当前设备的量子集链表n步，量子集不够就申请新的
    while (n--)
    {
        if (!qs->next)
        {
            qs->next = kmalloc(sizeof(struct scull_qset), GFP_KERNEL);
            if (qs->next == NULL)
            {
                return NULL;  /* Never mind */
            }
            memset(qs->next, 0, sizeof(struct scull_qset));
        }
        qs = qs->next;
        continue;
    }
    return qs;
}

/*
 * Data management: read and write
 */

ssize_t scull_read( struct file *filp, //设备对应的文件结构    
                    char __user *buf,  //从内核空间把数据读到用户空间
                    size_t count,      //字节数
                    loff_t *f_pos)     //要读的位置，在filp私有数据中的偏移   
{
    struct scull_dev *dev = filp->private_data; 
    struct scull_qset *dptr;    /* the first listitem */
    //量子、量子集大小   
    int quantum = dev->quantum；
    int qset = dev->qset;
    //一个量子集的字节数   
    int itemsize = quantum * qset; /* how many bytes in the listitem */
    int item, s_pos, q_pos, rest;
    ssize_t retval = 0;

    /*
    down_interruptible 获得信号量，如果得不到信号量就睡眠，此时没有信号打断，那么进入睡眠。
    但是在睡眠过程中可能被信号打断，打断之后返回-EINTR，主要用来进程间的互斥同步。
    */
    if (down_interruptible(&dev->sem))
    {
        return -ERESTARTSYS;
    }
    //要读的位置超过了数据总量
    if (*f_pos >= dev->size)
    {
        goto out;
    }
    //要读的count超出了size，截断count
    if (*f_pos + count > dev->size)
    {
        count = dev->size - *f_pos;
    }

    /* find listitem, qset index, and offset in the quantum */
    //在量子/量子集中定位读写位置：第几个量子集，中的第几个量子，在量子中偏移   
    //第几个量子集   
    item = (long)*f_pos / itemsize;
    //在量子集中的偏移量  
    rest = (long)*f_pos % itemsize;
    //第几个量子，在量子中的偏移
    s_pos = rest / quantum; q_pos = rest % quantum;

    /* follow the list up to the right position (defined elsewhere) */
    //读取要读的量子集的指针  
    dptr = scull_follow(dev, item);
    //读取出错处理  
    if (dptr == NULL || !dptr->data || ! dptr->data[s_pos])
    {
        goto out; /* don't fill holes */
    }

    /* read only up to the end of this quantum */
     // 只在一个量子中读：如果count超出当前量子，截断count 
    if (count > quantum - q_pos)
    {
        count = quantum - q_pos;
    }
    // 将要读位置的内容复制count字节到用户空间buf中   
    if (copy_to_user(buf, dptr->data[s_pos] + q_pos, count))
    {
        retval = -EFAULT;
        goto out;
    }
    *f_pos += count;
    retval = count;

  out:
    //释放信号量
    up(&dev->sem);
    return retval;
}

ssize_t scull_write(struct file *filp, //设备对应的文件结构    
                    const char __user *buf, //从用户空间把数据读到内核空间
                    size_t count,      //字节数
                    loff_t *f_pos)     //要读的位置，在filp私有数据中的偏移   
{
    struct scull_dev *dev = filp->private_data;
    struct scull_qset *dptr;
     //量子、量子集大小 
    int quantum = dev->quantum, qset = dev->qset;
    // 一个量子集总字节数
    int itemsize = quantum * qset;
    int item, s_pos, q_pos, rest;
    ssize_t retval = -ENOMEM; /* value used in "goto out" statements */

    if (down_interruptible(&dev->sem))
    {
        return -ERESTARTSYS;
    }

    /* find listitem, qset index and offset in the quantum */
     //第几个量子集
    item = (long)*f_pos / itemsize;
    //在该量子集中的偏移
    rest = (long)*f_pos % itemsize;
    //在该量子集中的第几个量子，在量子中的偏移
    s_pos = rest / quantum; q_pos = rest % quantum;

    /* follow the list up to the right position */
     //返回该量子集的指针
    dptr = scull_follow(dev, item);
    if (dptr == NULL)
    {
        goto out;
    }
    //如果该量子集数据为NULL，就申请一块新内存
    if (!dptr->data)
    {
        dptr->data = kmalloc(qset * sizeof(char *), GFP_KERNEL);
        if (!dptr->data)
        {
            goto out;
        }
        memset(dptr->data, 0, qset * sizeof(char *));
    }
    if (!dptr->data[s_pos])
    {
        dptr->data[s_pos] = kmalloc(quantum, GFP_KERNEL);
        if (!dptr->data[s_pos])
        {
            goto out;
        }
    }
    /* write only up to the end of this quantum */
    // 只在一个量子中写，如果count超出当前量子就截断
    if (count > quantum - q_pos)
    {
        count = quantum - q_pos;
    }
    //从用户空间拷贝数据到内核空间，失败返回没有拷贝的字节数，成功返回0
    if (copy_from_user(dptr->data[s_pos]+q_pos, buf, count))
    {
        retval = -EFAULT;
        goto out;
    }
    *f_pos += count;
    retval = count;

        /* update the size */
        // 更新字节总数大小 
    if (dev->size < *f_pos)
    {
        dev->size = *f_pos;
    }

  out:
    up(&dev->sem);
    return retval;
}

/*
 * The ioctl() implementation
 */

int scull_ioctl(struct inode *inode, struct file *filp,
                 unsigned int cmd, unsigned long arg)
{

	int err = 0, tmp;
	int retval = 0;

	/*
	抽取类型和编号位字段，并拒绝错误的命令号
	在调用access_ok()之前返回ENOTTY(不恰当的ioctl)
	*/
	//从命令参数中解析出幻数type #define _IOC_TYPE(nr) (((nr) >> _IOC_TYPESHIFT) & _IOC_TYPEMASK)
	if (_IOC_TYPE(cmd) != SCULL_IOC_MAGIC)
	{
	return -ENOTTY;
	}
	//从命令参数中解析出序数number #define _IOC_NR(nr) (((nr) >> _IOC_NRSHIFT) & _IOC_NRMASK)
	if (_IOC_NR(cmd) > SCULL_IOC_MAXNR)
	{
	return -ENOTTY;
	}

	/*
	方向是一个位掩码，而VERIFY_WRITE 用于 R/W 传输，类型是针对用户控件而言，而access_ok是面向内核空间的，因此读取和写入的概念恰好相反
	#define _IOC_NONE 0U //没有数据传输
	#define _IOC_WRITE 1U //向设备写入数据，驱动程序必须从用户空间读入数据
	#define _IOC_READ 2U //从设备中读取数据，驱动程序必须向用户空间写入数据
	*/
	//从命令参数中解析出数据方向，即写进还是读出 #define _IOC_DIR(nr) (((nr) >> _IOC_DIRSHIFT) & _IOC_DIRMASK)
	if (_IOC_DIR(cmd) & _IOC_READ)
	{
		err = !access_ok(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd));
	}
	else if (_IOC_DIR(cmd) & _IOC_WRITE)
	{
		err =  !access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd));
	}
	if (err)
	{
		return -EFAULT;
	}

	switch(cmd)
	{
	case SCULL_IOCRESET:
		scull_quantum = SCULL_QUANTUM;
		scull_qset = SCULL_QSET;
		break;

	case SCULL_IOCSQUANTUM: /* Set: arg points to the value */
		/*
		capable函数来对权限做出检查，检查是否有权对指定的资源进行操作，该函数返回0则代表无权操作
		CAP_DAC_OVERRIDE 越过在文件和目录上的访问限制(数据访问控制或 DAC)的能力
		CAP_NET_ADMIN 进行网络管理任务的能力, 包括那些能够影响网络接口的任务
		CAP_SYS_MODULE 加载或去除内核模块的能力
		CAP_SYS_RAWIO 进行 "raw"（裸）I/O 操作的能力. 例子包括存取设备端口或者直接和 USB 设备通讯
		CAP_SYS_ADMIN 截获的能力, 提供对许多系统管理操作的途径
		CAP_SYS_TTY_CONFIG 执行 tty 配置任务的能力
		*/
		if (! capable (CAP_SYS_ADMIN))
		{
			return -EPERM;
		}
		//把一个用户变量值传给内核变量，get_user ( x, ptr) ptr为用户空间指针 x为内核变量 即用户空间指针ptr所指变量值传送给内核变量x
		retval = __get_user(scull_quantum, (int __user *)arg);
		break;

	case SCULL_IOCTQUANTUM: /* Tell: arg is the value */
		if (! capable (CAP_SYS_ADMIN))
		{
			return -EPERM;
		}
		scull_quantum = arg;
		break;

	case SCULL_IOCGQUANTUM: /* Get: arg is pointer to result */
		//把一个内核变量值传给用户空间变量 put_user ( x, ptr) x为要传送的内核变量，ptr为用户空间的指针 即内核变量x传送给用户空间指针ptr所指变量值
		retval = __put_user(scull_quantum, (int __user *)arg);
		break;

	case SCULL_IOCQQUANTUM: /* Query: return it (it's positive) */
		return scull_quantum;

	case SCULL_IOCXQUANTUM: /* eXchange: use arg as pointer */
		if (! capable (CAP_SYS_ADMIN))
		{
			return -EPERM;
		}
		tmp = scull_quantum;
		retval = __get_user(scull_quantum, (int __user *)arg);
		if (retval == 0)
		{
			retval = __put_user(tmp, (int __user *)arg);
		}
		break;

	case SCULL_IOCHQUANTUM: /* sHift: like Tell + Query */
		if (! capable (CAP_SYS_ADMIN))
		{
			return -EPERM;
		}
		tmp = scull_quantum;
		scull_quantum = arg;
		return tmp;

	case SCULL_IOCSQSET:
		if (! capable (CAP_SYS_ADMIN))
		{
			return -EPERM;
		}
		retval = __get_user(scull_qset, (int __user *)arg);
		break;

	case SCULL_IOCTQSET:
		if (! capable (CAP_SYS_ADMIN))
		{
			return -EPERM;
		}
		scull_qset = arg;
		break;

	case SCULL_IOCGQSET:
		retval = __put_user(scull_qset, (int __user *)arg);
		break;

	case SCULL_IOCQQSET:
		return scull_qset;

	case SCULL_IOCXQSET:
		if (! capable (CAP_SYS_ADMIN))
		{
			return -EPERM;
		}
		tmp = scull_qset;
		retval = __get_user(scull_qset, (int __user *)arg);
		if (retval == 0)
		{
			retval = put_user(tmp, (int __user *)arg);
		}
		break;

	case SCULL_IOCHQSET:
		if (! capable (CAP_SYS_ADMIN))
		{
			return -EPERM;
		}
		tmp = scull_qset;
		scull_qset = arg;
		return tmp;

	/*
	 * The following two change the buffer size for scullpipe.
	 * The scullpipe device uses this same ioctl method, just to
	 * write less code. Actually, it's the same driver, isn't it?
	 */

	case SCULL_P_IOCTSIZE:
		scull_p_buffer = arg;
		break;

	case SCULL_P_IOCQSIZE:
		return scull_p_buffer;


	default:  /* redundant, as cmd was checked against MAXNR */
		return -ENOTTY;
	}
	return retval;

}



/*
 * The "extended" operations -- only seek
 */

loff_t scull_llseek(struct file *filp, loff_t off, int whence)
{
    struct scull_dev *dev = filp->private_data;
    loff_t newpos;

    switch(whence)
    {
      case 0: /* SEEK_SET */
        newpos = off;
        break;

      case 1: /* SEEK_CUR */
        newpos = filp->f_pos + off;
        break;

      case 2: /* SEEK_END */
        newpos = dev->size + off;
        break;

      default: /* 不应该发生 */
        return -EINVAL;
    }
    if (newpos < 0)
    {
        return -EINVAL;
    }
    filp->f_pos = newpos;
    return newpos;
}



struct file_operations scull_fops = {
    .owner =    THIS_MODULE,
    .llseek =   scull_llseek,
    .read =     scull_read,
    .write =    scull_write,
    .ioctl =    scull_ioctl,
    .open =     scull_open,
    .release =  scull_release,
};

/*
 * Finally, the module stuff
 */

/*
 * The cleanup function is used to handle initialization failures as well.
 * Thefore, it must be careful to work correctly even if some of the items
 * have not been initialized
 */
void scull_cleanup_module(void)
{
    int i;
    dev_t devno = MKDEV(scull_major, scull_minor);

    /* Get rid of our char dev entries */
    if (scull_devices)
    {
         //主次设备号合成一个dev_t结构，即设备编号   
        for (i = 0; i < scull_nr_devs; i++)
        {
            //释放数据区   
            scull_trim(scull_devices + i);
             //移除cdev   
            cdev_del(&scull_devices[i].cdev);
        }
        //释放scull_devices本身   
        kfree(scull_devices);
    }

#ifdef SCULL_DEBUG /* use proc only if debugging */
    scull_remove_proc();
#endif

    /* cleanup_module is never called if registering failed */
    unregister_chrdev_region(devno, scull_nr_devs);

    /* and call the cleanup functions for friend devices */
    scull_p_cleanup();
    scull_access_cleanup();

}


/*
 * Set up the char_dev structure for this device.
 */
 // 建立char_dev结构  
static void scull_setup_cdev(struct scull_dev *dev, int index)
{
    int err, devno = MKDEV(scull_major, scull_minor + index);
    
    cdev_init(&dev->cdev, &scull_fops);
    dev->cdev.owner = THIS_MODULE;
    dev->cdev.ops = &scull_fops;
    err = cdev_add (&dev->cdev, devno, 1);
    /* Fail gracefully if need be */
    //添加字符设备dev->cdev,立即生效 
    if (err)
    {
        printk(KERN_NOTICE "Error %d adding scull%d", err, index);
    }
}


int scull_init_module(void)
{
    int result, i;
    dev_t dev = 0;

/*
 * Get a range of minor numbers to work with, asking for a dynamic
 * major unless directed otherwise at load time.
 */
  //申请设备编号，若在加载时没有指定主设备号就动态分配 
    if (scull_major)   // insmod 时设置 scull_major 即 主设备的值
    {
        dev = MKDEV(scull_major, scull_minor);
        result = register_chrdev_region(dev, scull_nr_devs, "scull");
    } 
    else
    {   //没有设置主设备号时就动态获取  scull_nr_devs = 4  scull_minor = 0
        result = alloc_chrdev_region(&dev, scull_minor, scull_nr_devs, "scull");
        scull_major = MAJOR(dev);
    }
    if (result < 0)
    {
        printk(KERN_WARNING "scull: can't get major %d\n", scull_major);
        return result;
    }

     /* 
     * allocate the devices -- we can't have them static, as the number
     * can be specified at load time
     */
      //给scull_dev对象申请内存 
    scull_devices = kmalloc(scull_nr_devs * sizeof(struct scull_dev), GFP_KERNEL);
    if (!scull_devices)
    {
        result = -ENOMEM;
        goto fail;  /* Make this more graceful */
    }
    memset(scull_devices, 0, scull_nr_devs * sizeof(struct scull_dev));

        /* Initialize each device. */
    for (i = 0; i < scull_nr_devs; i++)
    {
        scull_devices[i].quantum = scull_quantum;   //4000
        scull_devices[i].qset = scull_qset;         //1000
        //初始化信号量void sema_init(struct semaphore *sem, int val);
        init_MUTEX(&scull_devices[i].sem);
        //建立char_dev结构 
        scull_setup_cdev(&scull_devices[i], i);
    }

        /* At this point call the init function for any friend device */
    dev = MKDEV(scull_major, scull_minor + scull_nr_devs);
    dev += scull_p_init(dev);
    dev += scull_access_init(dev);

#ifdef SCULL_DEBUG /* only when debugging  只有在debug的情形下才会在/proc目录下创建*/
    scull_create_proc();
#endif

    return 0; /* succeed */

  fail:
    scull_cleanup_module();
    return result;
}

module_init(scull_init_module);
module_exit(scull_cleanup_module);
