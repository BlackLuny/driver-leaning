/*
 * access.c -- the files with access control on open
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
 * $Id: access.c,v 1.17 2004/09/26 07:29:56 gregkh Exp $
 */

/* FIXME: cloned devices as a use for kobjects? */
 
#include <linux/kernel.h> /* printk() */
#include <linux/module.h>
#include <linux/slab.h>   /* kmalloc() */
#include <linux/fs.h>     /* everything... */
#include <linux/errno.h>  /* error codes */
#include <linux/types.h>  /* size_t */
#include <linux/fcntl.h>
#include <linux/cdev.h>
#include <linux/tty.h>
#include <asm/atomic.h>
#include <linux/list.h>

#include "scull.h"        /* local definitions */

static dev_t scull_a_firstdev;  /* Where our range begins */

/*
 * These devices fall back on the main scull operations. They only
 * differ in the implementation of open() and close()
 */



/************************************************************************
 *
 * The first device is the single-open one,
 *  it has an hw structure and an open count
 */

static struct scull_dev scull_s_device;
static atomic_t scull_s_available = ATOMIC_INIT(1);

//限制每次只能由一个进程打开
//只允许一个进程打开设备
static int scull_s_open(struct inode *inode, struct file *filp)
{
	struct scull_dev *dev = &scull_s_device; /* 设备信息 */

	/*
	此设备中维护一个atomic_t变量 称为scull_s_available 该变量值初始化值为1，表明该设备真正可用
	open 会减小并测试scull_s_available 并在其他进程打开该设备时拒绝访问
	*/
	//int atomic_dec_and_test(atomic_t *v); 原子类型的变量v原子地减1，并判断结果是否为0，如果为0，返回真，否则返回假
	if (! atomic_dec_and_test (&scull_s_available))
	{
		//void atomic_inc(atomic_t *v); 原子类型变量v原子地增加1。
		atomic_inc(&scull_s_available);
		return -EBUSY; /* 已打开 */
	}

	/* 然后 从裸的scull设备中复制所有其他数据 */
	if ( (filp->f_flags & O_ACCMODE) == O_WRONLY)
	{
		scull_trim(dev);
	}
	filp->private_data = dev;
	return 0;          /* 成功 */
}

static int scull_s_release(struct inode *inode, struct file *filp)
{
	//void atomic_inc(atomic_t *v); 原子类型变量v原子地增加1。
	atomic_inc(&scull_s_available); /* 释放该设备 */
	return 0;
}


/*
 * The other operations for the single-open device come from the bare device
 */
struct file_operations scull_sngl_fops = {
	.owner =	THIS_MODULE,
	.llseek =     	scull_llseek,
	.read =       	scull_read,
	.write =      	scull_write,
	.ioctl =      	scull_ioctl,
	.open =       	scull_s_open,
	.release =    	scull_s_release,
};


/************************************************************************
 *
 * Next, the "uid" device. It can be opened multiple times by the
 * same user, but access is denied to other users if the device is open
 */

static struct scull_dev scull_u_device;
static int scull_u_count;	/* initialized to 0 by default */
static uid_t scull_u_owner;	/* initialized to 0 by default */
static spinlock_t scull_u_lock = SPIN_LOCK_UNLOCKED; //自旋锁

//限制每次只有一个用户访问
//允许单个用户在多个进程中打开设备，但是每次只允许一个用户打开此设备
static int scull_u_open(struct inode *inode, struct file *filp)
{
	struct scull_dev *dev = &scull_u_device; /* device information */

	//锁的拥有时间非常短 所以可以使用自旋锁
	spin_lock(&scull_u_lock);
	if (scull_u_count && 
			(scull_u_owner != current->uid) &&  /* 允许用户 */
			(scull_u_owner != current->euid) && /* 允许执行su命令的用户 */
			!capable(CAP_DAC_OVERRIDE)) /* 也允许root用户 */
	{
		spin_unlock(&scull_u_lock);
		return -EBUSY;   /* 返回 -EPERM 会让用户混淆 */
	}

	if (scull_u_count == 0)
		scull_u_owner = current->uid; /* 获得所有者 */

	scull_u_count++;
	spin_unlock(&scull_u_lock);

/* then, everything else is copied from the bare scull device */

	if ((filp->f_flags & O_ACCMODE) == O_WRONLY)
	{
		scull_trim(dev);
	}
	filp->private_data = dev;
	return 0;          /* success */
}

static int scull_u_release(struct inode *inode, struct file *filp)
{
	spin_lock(&scull_u_lock);
	scull_u_count--; /* 除此之外不做任何事情 */
	spin_unlock(&scull_u_lock);
	return 0;
}



/*
 * The other operations for the device come from the bare device
 */
struct file_operations scull_user_fops = {
	.owner =      THIS_MODULE,
	.llseek =     scull_llseek,
	.read =       scull_read,
	.write =      scull_write,
	.ioctl =      scull_ioctl,
	.open =       scull_u_open,
	.release =    scull_u_release,
};


/************************************************************************
 *
 * Next, the device with blocking-open based on uid
 */

static struct scull_dev scull_w_device;
static int scull_w_count;	/* initialized to 0 by default */
static uid_t scull_w_owner;	/* initialized to 0 by default */
static DECLARE_WAIT_QUEUE_HEAD(scull_w_wait);
static spinlock_t scull_w_lock = SPIN_LOCK_UNLOCKED;   

static inline int scull_w_available(void)
{
	return scull_w_count == 0 ||
		scull_w_owner == current->uid ||
		scull_w_owner == current->euid ||
		capable(CAP_DAC_OVERRIDE);
}

//替代EBUSY的阻塞型open
//当进程不能访问设备时等待设备
static int scull_w_open(struct inode *inode, struct file *filp)
{
	struct scull_dev *dev = &scull_w_device; /* device information */

	spin_lock(&scull_w_lock);
	while (! scull_w_available())
	{
		spin_unlock(&scull_w_lock);
		if (filp->f_flags & O_NONBLOCK)
		{
			return -EAGAIN;
		}
		if (wait_event_interruptible (scull_w_wait, scull_w_available()))
		{
			return -ERESTARTSYS; /* 告诉 fs 层做进一步处理 */
		}
		spin_lock(&scull_w_lock);
	}
	if (scull_w_count == 0)
	{
		scull_w_owner = current->uid; /* 获取所有者 */
	}
	scull_w_count++;
	spin_unlock(&scull_w_lock);

	/* then, everything else is copied from the bare scull device */
	if ((filp->f_flags & O_ACCMODE) == O_WRONLY)
	{
		scull_trim(dev);
	}
	filp->private_data = dev;
	return 0;          /* success */
}

static int scull_w_release(struct inode *inode, struct file *filp)
{
	int temp;

	spin_lock(&scull_w_lock);
	scull_w_count--;
	temp = scull_w_count;
	spin_unlock(&scull_w_lock);

	if (temp == 0)
		wake_up_interruptible_sync(&scull_w_wait); /* 唤醒其他的 uid 进程 */
	return 0;
}


/*
 * The other operations for the device come from the bare device
 */
struct file_operations scull_wusr_fops = {
	.owner =      THIS_MODULE,
	.llseek =     scull_llseek,
	.read =       scull_read,
	.write =      scull_write,
	.ioctl =      scull_ioctl,
	.open =       scull_w_open,
	.release =    scull_w_release,
};

/************************************************************************
 *
 * Finally the `cloned' private device. This is trickier because it
 * involves list management, and dynamic allocation.
 */

/* 和复制相关的数据结构包含一个key成员 */

struct scull_listitem {
	struct scull_dev device;
	dev_t key;
	struct list_head list;
    
};

/* 设备的链表 以及保护他的锁 */
static LIST_HEAD(scull_c_list);
static spinlock_t scull_c_lock = SPIN_LOCK_UNLOCKED;

/* A placeholder scull_dev which really just holds the cdev stuff. */
static struct scull_dev scull_c_device;   

/* 查找设备 没有就创建一个 */
static struct scull_dev *scull_c_lookfor_device(dev_t key)
{
	struct scull_listitem *lptr;

	list_for_each_entry(lptr, &scull_c_list, list) {
		if (lptr->key == key)
			return &(lptr->device);
	}

	/* not found */
	lptr = kmalloc(sizeof(struct scull_listitem), GFP_KERNEL);
	if (!lptr)
		return NULL;

	/* initialize the device */
	memset(lptr, 0, sizeof(struct scull_listitem));
	lptr->key = key;
	scull_trim(&(lptr->device)); /* initialize it */
	init_MUTEX(&(lptr->device.sem));

	/* place it in the list */
	list_add(&lptr->list, &scull_c_list);

	return &(lptr->device);
}

//在打开时复制设备
static int scull_c_open(struct inode *inode, struct file *filp)
{
	struct scull_dev *dev;
	dev_t key;
 
	if (!current->signal->tty) { 
		PDEBUG("Process \"%s\" has no ctl tty\n", current->comm);
		return -EINVAL;
	}
	key = tty_devnum(current->signal->tty);

	/* look for a scullc device in the list */
	spin_lock(&scull_c_lock);
	dev = scull_c_lookfor_device(key);
	spin_unlock(&scull_c_lock);

	if (!dev)
		return -ENOMEM;

	/* 然后 从裸地scull设备中复制其他所有数据 */
	if ( (filp->f_flags & O_ACCMODE) == O_WRONLY)
		scull_trim(dev);
	filp->private_data = dev;
	return 0;          /* success */
}

static int scull_c_release(struct inode *inode, struct file *filp)
{
	/*
	 * 因为设备是持久的 所以不需要做任何操作
	 一个真正的克隆设备应该在最后一次关闭时释放
	 */
	return 0;
}



/*
 * The other operations for the device come from the bare device
 */
struct file_operations scull_priv_fops = {
	.owner =    THIS_MODULE,
	.llseek =   scull_llseek,
	.read =     scull_read,
	.write =    scull_write,
	.ioctl =    scull_ioctl,
	.open =     scull_c_open,
	.release =  scull_c_release,
};

/************************************************************************
 *
 * And the init and cleanup functions come last
 */

static struct scull_adev_info
{
	char *name;
	struct scull_dev *sculldev;
	struct file_operations *fops;
} 
scull_access_devs[] = 
{
	{ "scullsingle", &scull_s_device, &scull_sngl_fops },
	{ "sculluid", &scull_u_device, &scull_user_fops },
	{ "scullwuid", &scull_w_device, &scull_wusr_fops },
	{ "sullpriv", &scull_c_device, &scull_priv_fops }
};
#define SCULL_N_ADEVS 4

/*
 * Set up a single device.
 */
static void scull_access_setup (dev_t devno, struct scull_adev_info *devinfo)
{
	struct scull_dev *dev = devinfo->sculldev;
	int err;

	/* Initialize the device structure */
	dev->quantum = scull_quantum;
	dev->qset = scull_qset;
	init_MUTEX(&dev->sem);

	/* Do the cdev stuff. */
	cdev_init(&dev->cdev, devinfo->fops);
	kobject_set_name(&dev->cdev.kobj, devinfo->name);
	dev->cdev.owner = THIS_MODULE;
	err = cdev_add (&dev->cdev, devno, 1);
        /* Fail gracefully if need be */
	if (err) {
		printk(KERN_NOTICE "Error %d adding %s\n", err, devinfo->name);
		kobject_put(&dev->cdev.kobj);
	} else
		printk(KERN_NOTICE "%s registered at %x\n", devinfo->name, devno);
}


int scull_access_init(dev_t firstdev)
{
	int result, i;

	/* Get our number space */
	result = register_chrdev_region (firstdev, SCULL_N_ADEVS, "sculla");
	if (result < 0) {
		printk(KERN_WARNING "sculla: device number registration failed\n");
		return 0;
	}
	scull_a_firstdev = firstdev;

	/* Set up each device. */
	for (i = 0; i < SCULL_N_ADEVS; i++)
		scull_access_setup (firstdev + i, scull_access_devs + i);
	return SCULL_N_ADEVS;
}

/*
 * This is called by cleanup_module or on failure.
 * It is required to never fail, even if nothing was initialized first
 */
void scull_access_cleanup(void)
{
	struct scull_listitem *lptr, *next;
	int i;

	/* Clean up the static devs */
	for (i = 0; i < SCULL_N_ADEVS; i++) {
		struct scull_dev *dev = scull_access_devs[i].sculldev;
		cdev_del(&dev->cdev);
		scull_trim(scull_access_devs[i].sculldev);
	}

    	/* And all the cloned devices */
	list_for_each_entry_safe(lptr, next, &scull_c_list, list) {
		list_del(&lptr->list);
		scull_trim(&(lptr->device));
		kfree(lptr);
	}

	/* Free up our number space */
	unregister_chrdev_region(scull_a_firstdev, SCULL_N_ADEVS);
	return;
}
