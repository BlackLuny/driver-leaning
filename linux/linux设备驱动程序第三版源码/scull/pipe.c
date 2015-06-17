/*
 * pipe.c -- fifo driver for scull
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
 
#include <linux/module.h>
#include <linux/moduleparam.h>

#include <linux/kernel.h>	/* printk(), min() */
#include <linux/slab.h>		/* kmalloc() */
#include <linux/fs.h>		/* everything... */
#include <linux/proc_fs.h>
#include <linux/errno.h>	/* error codes */
#include <linux/types.h>	/* size_t */
#include <linux/fcntl.h>
#include <linux/poll.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>

#include "scull.h"		/* local definitions */

/**
等待队列的头
struct __wait_queue_head {
	由于等待队列可能由中断处理程序和内核函数修改，所以必须对双向链表进行保护，以免对其进行同时访问。
	其同步是由lock自旋锁达到的。
	spinlock_t lock;
	等待进程链表的头。
	struct list_head task_list;
};
typedef struct __wait_queue_head wait_queue_head_t;
*/
struct scull_pipe
{
        wait_queue_head_t inq, outq;       /* read and write queues 读取和写入队列*/
        char *buffer, *end;                /* begin of buf, end of buf 缓冲区的起始和结尾*/
        int buffersize;                    /* used in pointer arithmetic 用于指针计算*/
        char *rp, *wp;                     /* where to read, where to write 读取和写入位置*/
        int nreaders, nwriters;            /* number of openings for r/w 用于读写打开的数据*/
        struct fasync_struct *async_queue; /* asynchronous readers 异步读取者*/
        struct semaphore sem;              /* mutual exclusion semaphore 互斥信号量*/
        struct cdev cdev;                  /* Char device structure 字符设备结构*/
};

/* parameters */
static int scull_p_nr_devs = SCULL_P_NR_DEVS;	/* number of pipe devices */
int scull_p_buffer =  SCULL_P_BUFFER;	/* buffer size 4000 */
dev_t scull_p_devno;			/* Our first device number */

module_param(scull_p_nr_devs, int, 0);	/* FIXME check perms */
module_param(scull_p_buffer, int, 0);

static struct scull_pipe *scull_p_devices;

static int scull_p_fasync(int fd, struct file *filp, int mode);
static int spacefree(struct scull_pipe *dev);
/*
 * Open and close
 */


static int scull_p_open(struct inode *inode, struct file *filp)
{
	struct scull_pipe *dev;

	dev = container_of(inode->i_cdev, struct scull_pipe, cdev);
	filp->private_data = dev;

	if (down_interruptible(&dev->sem))
	{
		return -ERESTARTSYS;
	}
	if (!dev->buffer)
	{
		/* allocate the buffer scull_p_buffer = 4000*/
		dev->buffer = kmalloc(scull_p_buffer, GFP_KERNEL);
		if (!dev->buffer)
		{
			up(&dev->sem);
			return -ENOMEM;
		}
	}
	dev->buffersize = scull_p_buffer;
	dev->end = dev->buffer + dev->buffersize;
	dev->rp = dev->wp = dev->buffer; /* rd and wr from the beginning */

	/* use f_mode,not  f_flags: it's cleaner (fs/open.c tells why) */
	if (filp->f_mode & FMODE_READ)
	{
		dev->nreaders++;
	}
	if (filp->f_mode & FMODE_WRITE)
	{
		dev->nwriters++;
	}
	up(&dev->sem);
	/*
	int nonseekable_open(struct inode *inode; struct file *filp); 通知内核设备不支持lseek
	这个调用标识了给定的filp为不可移位的;内核就不会允许一个lseek调用在这样一个文件上成功.通过用这样的方式标识这个文件,
	你可确定不会有通过pread和pwrite系统调用的方式来试图移位这个文件
	*/
	return nonseekable_open(inode, filp);
}



static int scull_p_release(struct inode *inode, struct file *filp)
{
	struct scull_pipe *dev = filp->private_data;

	/* 从异步通知列表中删除 filp */
	scull_p_fasync(-1, filp, 0);
	down(&dev->sem);
	if (filp->f_mode & FMODE_READ)
	{
		dev->nreaders--;
	}
	if (filp->f_mode & FMODE_WRITE)
	{
		dev->nwriters--;
	}
	if (dev->nreaders + dev->nwriters == 0)
	{
		kfree(dev->buffer);
		dev->buffer = NULL; /* the other fields are not checked on open */
	}
	up(&dev->sem);
	return 0;
}


/*
 * Data management: read and write
 */

static ssize_t scull_p_read (struct file *filp, char __user *buf, size_t count,
                loff_t *f_pos)
{
	struct scull_pipe *dev = filp->private_data;

	if (down_interruptible(&dev->sem))
	{
		return -ERESTARTSYS;
	}
	while (dev->rp == dev->wp)  /* 无数据可读 */
	{
		up(&dev->sem); /* 释放锁 */
		if (filp->f_flags & O_NONBLOCK)  //非阻塞直接返回
		{
			return -EAGAIN;
		}
		PDEBUG("\"%s\" reading: going to sleep\n", current->comm);
		//将当前进程的状态设置成TASK_INTERRUPTIBLE，然后调用schedule schedule会将位于TASK_INTERRUPTIBLE状态的当前进程从runqueue 队列中删除
		//成功地唤醒一个被wait_event_interruptible(wq, condition)的进程 条件 1)condition为真的前提下，2) 调用wake_up()。
		if (wait_event_interruptible(dev->inq, (dev->rp != dev->wp)))   //等待有数据可读
		{
			return -ERESTARTSYS; /* 信号 通知fs层做相应处理 */
		}
		/* 否则循环 但首先获得锁 */
		if (down_interruptible(&dev->sem))
		{
			return -ERESTARTSYS;
		}
	}
	/* 数据已就绪 返回 */
	if (dev->wp > dev->rp)
	{
		count = min(count, (size_t)(dev->wp - dev->rp));
	}
	else /* 写入数据回卷 返回数据直到 dev->end */
	{
		count = min(count, (size_t)(dev->end - dev->rp));
	}
	if (copy_to_user(buf, dev->rp, count))
	{
		up (&dev->sem);
		return -EFAULT;
	}
	dev->rp += count;
	if (dev->rp == dev->end)
	{
		dev->rp = dev->buffer; /* 回卷 */
	}
	up (&dev->sem);
	/* 最后 唤醒所有写入者并返回 */
	wake_up_interruptible(&dev->outq);
	PDEBUG("\"%s\" did read %li bytes\n",current->comm, (long)count);
	return count;
}

/*
等待有可用于写入的空间；调用者必须拥有设备信号量
在错误情况下，信号量将在返回前释放
*/
static int scull_getwritespace(struct scull_pipe *dev, struct file *filp)
{
	while (spacefree(dev) == 0)	/* full */
	{
		DEFINE_WAIT(wait);
		up(&dev->sem);
		if (filp->f_flags & O_NONBLOCK)
		{
			return -EAGAIN;
		}
		PDEBUG("\"%s\" writing: going to sleep\n",current->comm);
		prepare_to_wait(&dev->outq, &wait, TASK_INTERRUPTIBLE);
		if (spacefree(dev) == 0)
		{
			schedule();
		}
		finish_wait(&dev->outq, &wait);
		if (signal_pending(current))
		{
			return -ERESTARTSYS; /* 信号 通知fs层做相应处理 */
		}
		if (down_interruptible(&dev->sem))
		{
			return -ERESTARTSYS;
		}
	}
	return 0;
}	

/* How much space is free? */
static int spacefree(struct scull_pipe *dev)
{
	if (dev->rp == dev->wp)
	{
		return dev->buffersize - 1;
	}
	return ((dev->rp + dev->buffersize - dev->wp) % dev->buffersize) - 1;
}

static ssize_t scull_p_write(struct file *filp, const char __user *buf, size_t count,
                loff_t *f_pos)
{
	struct scull_pipe *dev = filp->private_data;
	int result;
	if (down_interruptible(&dev->sem))
	{
		return -ERESTARTSYS;
	}
	/* 确保用空间可写入 */
	result = scull_getwritespace(dev, filp);
	if (result)
	{
		return result; /* scull_getwritespace called up(&dev->sem) */
	}
	/* 有空间可用 接收数据 */
	count = min(count, (size_t)spacefree(dev));
	if (dev->wp >= dev->rp)
	{
		count = min(count, (size_t)(dev->end - dev->wp)); /* 直到缓冲区结尾 */
	}
	else /* 写入指针回卷 填充到 rp-1*/
	{
		count = min(count, (size_t)(dev->rp - dev->wp - 1));
	}
	PDEBUG("Going to accept %li bytes to %p from %p\n", (long)count, dev->wp, buf);
	if (copy_from_user(dev->wp, buf, count))
	{
		up (&dev->sem);
		return -EFAULT;
	}
	dev->wp += count;
	if (dev->wp == dev->end)
	{
		dev->wp = dev->buffer; /* 回卷 */
	}
	up(&dev->sem);
	/* 最后唤醒读者 */
	wake_up_interruptible(&dev->inq);  /* blocked in read() and select() */
	/* 通知异步读取者 将在本章后面解释*/
	if (dev->async_queue)
	{
		kill_fasync(&dev->async_queue, SIGIO, POLL_IN);
	}
	PDEBUG("\"%s\" did write %li bytes\n",current->comm, (long)count);
	return count;
}

/*

*/
static unsigned int scull_p_poll(struct file *filp, poll_table *wait)
{
	struct scull_pipe *dev = filp->private_data;
	unsigned int mask = 0;

	/*
	 * 缓冲区是环形的 也就是说 入伙wp在rp之后 则表明缓冲区已满
	 * 而如果他们两个相等 则说明是空的
	 */
	down(&dev->sem);
	//poll_wait 把当前进程添加到wait参数指定的等待列表(poll_table)中
	poll_wait(filp, &dev->inq,  wait);
	poll_wait(filp, &dev->outq, wait);
	if (dev->rp != dev->wp)
	{
		mask |= POLLIN | POLLRDNORM;	/* 可读取 */
	}
	if (spacefree(dev))
	{
		mask |= POLLOUT | POLLWRNORM;	/* 可写入 */
	}
	up(&dev->sem);
	return mask;
}





static int scull_p_fasync(int fd, struct file *filp, int mode)
{
	struct scull_pipe *dev = filp->private_data;

	return fasync_helper(fd, filp, mode, &dev->async_queue);
}



/* FIXME this should use seq_file */
#ifdef SCULL_DEBUG
static void scullp_proc_offset(char *buf, char **start, off_t *offset, int *len)
{
	if (*offset == 0)
		return;
	if (*offset >= *len) {	/* Not there yet */
		*offset -= *len;
		*len = 0;
	}
	else {			/* We're into the interesting stuff now */
		*start = buf + *offset;
		*offset = 0;
	}
}


static int scull_read_p_mem(char *buf, char **start, off_t offset, int count,
		int *eof, void *data)
{
	int i, len;
	struct scull_pipe *p;

#define LIMIT (PAGE_SIZE-200)	/* don't print any more after this size */
	*start = buf;
	len = sprintf(buf, "Default buffersize is %i\n", scull_p_buffer);
	for(i = 0; i<scull_p_nr_devs && len <= LIMIT; i++) {
		p = &scull_p_devices[i];
		if (down_interruptible(&p->sem))
			return -ERESTARTSYS;
		len += sprintf(buf+len, "\nDevice %i: %p\n", i, p);
/*		len += sprintf(buf+len, "   Queues: %p %p\n", p->inq, p->outq);*/
		len += sprintf(buf+len, "   Buffer: %p to %p (%i bytes)\n", p->buffer, p->end, p->buffersize);
		len += sprintf(buf+len, "   rp %p   wp %p\n", p->rp, p->wp);
		len += sprintf(buf+len, "   readers %i   writers %i\n", p->nreaders, p->nwriters);
		up(&p->sem);
		scullp_proc_offset(buf, start, &offset, &len);
	}
	*eof = (len <= LIMIT);
	return len;
}


#endif



/*
 * The file operations for the pipe device
 * (some are overlayed with bare scull)
 */
struct file_operations scull_pipe_fops = {
	.owner =	THIS_MODULE,
	.llseek =	no_llseek,  //内核辅助函数 在open时指定nonseekable_open(inode, filp);不会让lseek调用成功
	.read =		scull_p_read,
	.write =	scull_p_write,
	.poll =		scull_p_poll,
	.ioctl =	scull_ioctl,
	.open =		scull_p_open,
	.release =	scull_p_release,
	.fasync =	scull_p_fasync,
};


/*
 * Set up a cdev entry.
 */
static void scull_p_setup_cdev(struct scull_pipe *dev, int index)
{
	int err, devno = scull_p_devno + index;
    
	cdev_init(&dev->cdev, &scull_pipe_fops);
	dev->cdev.owner = THIS_MODULE;
	err = cdev_add (&dev->cdev, devno, 1);
	/* Fail gracefully if need be */
	if (err)
		printk(KERN_NOTICE "Error %d adding scullpipe%d", err, index);
}

 

/*
 * Initialize the pipe devs; return how many we did.
 */
int scull_p_init(dev_t firstdev)
{
	int i, result;

	result = register_chrdev_region(firstdev, scull_p_nr_devs, "scullp");
	if (result < 0) {
		printk(KERN_NOTICE "Unable to get scullp region, error %d\n", result);
		return 0;
	}
	scull_p_devno = firstdev;
	scull_p_devices = kmalloc(scull_p_nr_devs * sizeof(struct scull_pipe), GFP_KERNEL);
	if (scull_p_devices == NULL)
	{
		unregister_chrdev_region(firstdev, scull_p_nr_devs);
		return 0;
	}
	memset(scull_p_devices, 0, scull_p_nr_devs * sizeof(struct scull_pipe));
	for (i = 0; i < scull_p_nr_devs; i++)
	{
		//初始化等待队列头 void init_waitqueue_head(wait_queue_head_t *q)
		init_waitqueue_head(&(scull_p_devices[i].inq));
		init_waitqueue_head(&(scull_p_devices[i].outq));
		init_MUTEX(&scull_p_devices[i].sem);
		scull_p_setup_cdev(scull_p_devices + i, i);
	}
#ifdef SCULL_DEBUG
	create_proc_read_entry("scullpipe", 0, NULL, scull_read_p_mem, NULL);
#endif
	return scull_p_nr_devs;
}

/*
 * This is called by cleanup_module or on failure.
 * It is required to never fail, even if nothing was initialized first
 */
void scull_p_cleanup(void)
{
	int i;

#ifdef SCULL_DEBUG
	remove_proc_entry("scullpipe", NULL);
#endif

	if (!scull_p_devices)
		return; /* nothing else to release */

	for (i = 0; i < scull_p_nr_devs; i++) {
		cdev_del(&scull_p_devices[i].cdev);
		kfree(scull_p_devices[i].buffer);
	}
	kfree(scull_p_devices);
	unregister_chrdev_region(scull_p_devno, scull_p_nr_devs);
	scull_p_devices = NULL; /* pedantic */
}
