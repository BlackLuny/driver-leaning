/*
 * short.c -- Simple Hardware Operations and Raw Tests
 * short.c -- also a brief example of interrupt handling ("short int")
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
 * $Id: short.c,v 1.16 2004/10/29 16:45:40 corbet Exp $
 */

/*
 * FIXME: this driver is not safe with concurrent readers or
 * writers.
 */

#include <linux/config.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>

#include <linux/sched.h>
#include <linux/kernel.h>	/* printk() */
#include <linux/fs.h>		/* everything... */
#include <linux/errno.h>	/* error codes */
#include <linux/delay.h>	/* udelay */
#include <linux/kdev_t.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/poll.h>
#include <linux/wait.h>

#include <asm/io.h>

#define SHORT_NR_PORTS	8	/* use 8 ports by default */

/*
 * all of the parameters have no "short_" prefix, to save typing when
 * specifying them at load time
 */
static int major = 0;	/* dynamic by default */
module_param(major, int, 0);

static int use_mem = 0;	/* default is I/O-mapped */
module_param(use_mem, int, 0);

/* default is the first printer port on PC's. "short_base" is there too
   because it's what we want to use in the code */
static unsigned long base = 0x378;
unsigned long short_base = 0;
module_param(base, long, 0);

/* The interrupt line is undefined by default. "short_irq" is as above */
static int irq = -1;
volatile int short_irq = -1;
module_param(irq, int, 0);

static int probe = 0;	/* select at load time how to probe irq line */
module_param(probe, int, 0);

static int wq = 0;	/* select at load time whether a workqueue is used */
module_param(wq, int, 0);

static int tasklet = 0;	/* select whether a tasklet is used */
module_param(tasklet, int, 0);

static int share = 0;	/* select at load time whether install a shared irq */
module_param(share, int, 0);

MODULE_AUTHOR ("Alessandro Rubini");
MODULE_LICENSE("Dual BSD/GPL");


unsigned long short_buffer = 0;
unsigned long volatile short_head;
volatile unsigned long short_tail;
DECLARE_WAIT_QUEUE_HEAD(short_queue);

/* Set up our tasklet if we're doing that. */
void short_do_tasklet(unsigned long);
DECLARE_TASKLET(short_tasklet, short_do_tasklet, 0);

/*
 * Atomicly increment an index into short_buffer
 */
static inline void short_incr_bp(volatile unsigned long *index, int delta)
{
	/*
	��������ǳ��Ͻ�������ָ��������ѭ��������֮�ڣ����Ҳ�����Ϊ����һ������ȷ��ֵ������
	*/
	unsigned long new = *index + delta;
	barrier();  /* ��ֹ��ǰ�����������Ż� */
	*index = (new >= (short_buffer + PAGE_SIZE)) ? short_buffer : new;
}


/*
 * The devices with low minor numbers write/read burst of data to/from
 * specific I/O ports (by default the parallel ones).
 * 
 * The device with 128 as minor number returns ascii strings telling
 * when interrupts have been received. Writing to the device toggles
 * 00/FF on the parallel data lines. If there is a loopback wire, this
 * generates interrupts.  
 */

int short_open (struct inode *inode, struct file *filp)
{
	extern struct file_operations short_i_fops;

	if (iminor (inode) & 0x80)
		filp->f_op = &short_i_fops; /* the interrupt-driven node */
	return 0;
}


int short_release (struct inode *inode, struct file *filp)
{
	return 0;
}


/* first, the port-oriented device */

enum short_modes {SHORT_DEFAULT=0, SHORT_PAUSE, SHORT_STRING, SHORT_MEMORY};

ssize_t do_short_read (struct inode *inode, struct file *filp, char __user *buf,
		size_t count, loff_t *f_pos)
{
	int retval = count, minor = iminor (inode);
	unsigned long port = short_base + (minor&0x0f);
	void *address = (void *) short_base + (minor&0x0f);
	int mode = (minor&0x70) >> 4;
	unsigned char *kbuf = kmalloc(count, GFP_KERNEL), *ptr;
    
	if (!kbuf)
		return -ENOMEM;
	ptr = kbuf;

	if (use_mem)
		mode = SHORT_MEMORY;
	
	switch(mode) {
	    case SHORT_STRING:
		insb(port, ptr, count);
		rmb();
		break;

	    case SHORT_DEFAULT:
		while (count--) {
			*(ptr++) = inb(port);
			rmb();
		}
		break;

	    case SHORT_MEMORY:
		while (count--) {
			*ptr++ = ioread8(address);
			rmb();
		}
		break;
	    case SHORT_PAUSE:
		while (count--) {
			*(ptr++) = inb_p(port);
			rmb();
		}
		break;

	    default: /* no more modes defined by now */
		retval = -EINVAL;
		break;
	}
	if ((retval > 0) && copy_to_user(buf, kbuf, retval))
		retval = -EFAULT;
	kfree(kbuf);
	return retval;
}


/*
 * Version-specific methods for the fops structure.  FIXME don't need anymore.
 */
ssize_t short_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
	return do_short_read(filp->f_dentry->d_inode, filp, buf, count, f_pos);
}



ssize_t do_short_write (struct inode *inode, struct file *filp, const char __user *buf,
		size_t count, loff_t *f_pos)
{
	int retval = count, minor = iminor(inode);
	unsigned long port = short_base + (minor&0x0f);
	void *address = (void *) short_base + (minor&0x0f);
	int mode = (minor&0x70) >> 4;
	unsigned char *kbuf = kmalloc(count, GFP_KERNEL), *ptr;

	if (!kbuf)
		return -ENOMEM;
	if (copy_from_user(kbuf, buf, count))
		return -EFAULT;
	ptr = kbuf;

	if (use_mem)
		mode = SHORT_MEMORY;

	switch(mode) {
	case SHORT_PAUSE:
		while (count--) {
			outb_p(*(ptr++), port);
			wmb();
		}
		break;

	case SHORT_STRING:
		outsb(port, ptr, count);
		wmb();
		break;

	case SHORT_DEFAULT:
		while (count--) {
			outb(*(ptr++), port);
			wmb();
		}
		break;

	case SHORT_MEMORY:
		while (count--) {
			iowrite8(*ptr++, address);
			wmb();
		}
		break;

	default: /* no more modes defined by now */
		retval = -EINVAL;
		break;
	}
	kfree(kbuf);
	return retval;
}


ssize_t short_write(struct file *filp, const char __user *buf, size_t count,
		loff_t *f_pos)
{
	return do_short_write(filp->f_dentry->d_inode, filp, buf, count, f_pos);
}




unsigned int short_poll(struct file *filp, poll_table *wait)
{
	return POLLIN | POLLRDNORM | POLLOUT | POLLWRNORM;
}






struct file_operations short_fops = {
	.owner	 = THIS_MODULE,
	.read	 = short_read,
	.write	 = short_write,
	.poll	 = short_poll,
	.open	 = short_open,
	.release = short_release,
};

/* then,  the interrupt-related device */

ssize_t short_i_read (struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
	int count0;
	DEFINE_WAIT(wait);

	while (short_head == short_tail) {
		prepare_to_wait(&short_queue, &wait, TASK_INTERRUPTIBLE);
		if (short_head == short_tail)
			schedule();
		finish_wait(&short_queue, &wait);
		if (signal_pending (current))  /* ĳ���ź��Ѿ����� */
			return -ERESTARTSYS; /* ����fs������һ������ */
	} 
	/* count0 �ǿɶ�ȡ�����ֽ��� */
	count0 = short_head - short_tail;
	if (count0 < 0) /* �ѽ��� */
		count0 = short_buffer + PAGE_SIZE - short_tail;
	if (count0 < count) count = count0;

	if (copy_to_user(buf, (char *)short_tail, count))
		return -EFAULT;
	short_incr_bp (&short_tail, count);
	return count;
}

ssize_t short_i_write (struct file *filp, const char __user *buf, size_t count,
		loff_t *f_pos)
{
	int written = 0, odd = *f_pos & 1;
	unsigned long port = short_base; /* ��������ڵ��������Ĵ��� */
	void *address = (void *) short_base;

	if (use_mem) {
		while (written < count)
			iowrite8(0xff * ((++written + odd) & 1), address);
	} else {
		while (written < count)
			outb(0xff * ((++written + odd) & 1), port);
	}

	*f_pos += count;
	return written;
}




struct file_operations short_i_fops = {
	.owner	 = THIS_MODULE,
	.read	 = short_i_read,
	.write	 = short_i_write,
	.open	 = short_open,
	.release = short_release,
};

irqreturn_t short_interrupt(int irq, void *dev_id, struct pt_regs *regs)
{
	struct timeval tv;
	int written;

	do_gettimeofday(&tv);

	    /* Write a 16 byte record. Assume PAGE_SIZE is a multiple of 16 */
	written = sprintf((char *)short_head,"%08u.%06u\n",
			(int)(tv.tv_sec % 100000000), (int)(tv.tv_usec));
	BUG_ON(written != 16);
	short_incr_bp(&short_head, written);
	wake_up_interruptible(&short_queue); /* awake any reading process */
	return IRQ_HANDLED;
}

/*
 * The following two functions are equivalent to the previous one,
 * but split in top and bottom half. First, a few needed variables
 */

#define NR_TIMEVAL 512 /* length of the array of time values */

struct timeval tv_data[NR_TIMEVAL]; /* too lazy to allocate it */
volatile struct timeval *tv_head=tv_data;
volatile struct timeval *tv_tail=tv_data;

static struct work_struct short_wq;


int short_wq_count = 0;

/*
 * Increment a circular buffer pointer in a way that nobody sees
 * an intermediate value.
 */
static inline void short_incr_tv(volatile struct timeval **tvp)
{
	if (*tvp == (tv_data + NR_TIMEVAL - 1))
		*tvp = tv_data;	 /* Wrap */
	else
		(*tvp)++;
}



void short_do_tasklet (unsigned long unused)
{
	int savecount = short_wq_count, written;
	short_wq_count = 0; /* �Ѿ��Ӷ������Ƴ� */
	/*
	 �װ벿��ȡ�ɶ��벿����tv����
	 ����ѭ�����黺������ӡ��Ϣ��������������Ϣ���ɶ�ȡ���̻��
	 */

	/*���ȵ��ô�bh֮ǰ�������ն�����д�� */
	written = sprintf((char *)short_head,"bh after %6i\n",savecount);
	short_incr_bp(&short_head, written);

	/*
	 * Ȼ��д��ʱ��ֵ ÿ��д��16�ֽ�
	 * ��������PAGE_SIZE�Ƕ����
	 */

	do {
		written = sprintf((char *)short_head,"%08u.%06u\n",
				(int)(tv_tail->tv_sec % 100000000),
				(int)(tv_tail->tv_usec));
		short_incr_bp(&short_head, written);
		short_incr_tv(&tv_tail);
	} while (tv_tail != tv_head);

	wake_up_interruptible(&short_queue); /* �����κζ�ȡ���� */
}


irqreturn_t short_wq_interrupt(int irq, void *dev_id, struct pt_regs *regs)
{
	/* ��ȡ��ǰʱ����Ϣ. */
	do_gettimeofday((struct timeval *) tv_head);
	short_incr_tv(&tv_head);

	/* ����bh ���ص��Ķ�ε��ȵ���� */
	schedule_work(&short_wq);

	short_wq_count++; /* ��¼�жϵĵ��� */
	return IRQ_HANDLED;
}


/*
 * Tasklet top half
 */

irqreturn_t short_tl_interrupt(int irq, void *dev_id, struct pt_regs *regs)
{
	do_gettimeofday((struct timeval *) tv_head); /* cast to stop 'volatile' warning */
	short_incr_tv(&tv_head);
	tasklet_schedule(&short_tasklet);
	short_wq_count++; /* record that an interrupt arrived */
	return IRQ_HANDLED;
}




irqreturn_t short_sh_interrupt(int irq, void *dev_id, struct pt_regs *regs)
{
	int value, written;
	struct timeval tv;

	/* �������short���������������� */
	value = inb(short_base);
	if (!(value & 0x80))
		return IRQ_NONE;
	
	/* ����ж�λ */
	outb(value & 0x7F, short_base);

	/* ���ಿ��û��ʲô�仯 */

	do_gettimeofday(&tv);
	//д��һ��16�ֽڵļ�¼���ٶ�PAGE_SIZE��16�ı���
	written = sprintf((char *)short_head,"%08u.%06u\n",
			(int)(tv.tv_sec % 100000000), (int)(tv.tv_usec));
	short_incr_bp(&short_head, written);
	wake_up_interruptible(&short_queue); /* �����κζ�ȡ���� */
	return IRQ_HANDLED;
}

void short_kernelprobe(void)
{
	int count = 0;
	do {
		unsigned long mask;

		mask = probe_irq_on();
		outb_p(0x10,short_base+2); /* �����жϱ��� */
		outb_p(0x00,short_base);   /* �����λ */
		outb_p(0xFF,short_base);   /* ���ø�λ �ж� */
		outb_p(0x00,short_base+2); /* �����жϱ��� */
		udelay(5);  /* �����ն�̽��һЩʱ�� */
		short_irq = probe_irq_off(mask);

		if (short_irq == 0)/* û���ҵ�? */
		{ 
			printk(KERN_INFO "short: no irq reported by probe\n");
			short_irq = -1;
		}
		/*
		 ����Ѿ��ж���ж��߱��������Ϊ��ֵ
		 ����Ӧ�÷�����жϣ�lpt�˿ڲ���Ҫ�����ٴ�����
		 �������5�Σ�Ȼ�����
		 */
	} while (short_irq < 0 && count++ < 5);
	if (short_irq < 0)
	{
		printk("short: probe failed %i times, giving up\n", count);
	}
}

irqreturn_t short_probing(int irq, void *dev_id, struct pt_regs *regs)
{
	if (short_irq == 0)
	{
		short_irq = irq;	/* �ҵ� */
	}
	if (short_irq != irq)
	{
		short_irq = -irq; /* ���ֶ����� */
	}
	return IRQ_HANDLED;
}

/*
irq̽����ͨ��Ϊÿ��ȱ���жϴ������̵�IRQ����IRQ_WAITING״̬λ����ɵ�
�������жϲ���ʱ����Ϊû��ע�ᴦ�����̣�do_IRQ�����λȻ�󷵻ء���probe_irq_off��
һ������������õ�ʱ��ֻ��Ҫ������Щû������IRQ_WAITINGλ��IRQ
*/
void short_selfprobe(void)
{
	int trials[] = {3, 5, 7, 9, 0};
	int tried[]  = {0, 0, 0, 0, 0};
	int i, count = 0;

	/*
	 Ϊ���е��ж��߰�װ̽�⴦������
	 ��¼��ЩĿǰ���еĽ����0 �ɹ� -EBUSY��������Ϊ���ͷ��ѻ�õ���Դ
      */
	for (i = 0; trials[i]; i++)
	{
		tried[i] = request_irq(trials[i], short_probing,
				SA_INTERRUPT, "short probe", NULL);
	}

	do {
		short_irq = 0; /* ʲôҲδ��� */
		outb_p(0x10,short_base+2); /* �����ж� */
		outb_p(0x00,short_base);
		outb_p(0xFF,short_base); /* �л��ж�λ */
		outb_p(0x00,short_base+2); /* �����ж� */
		udelay(5);  /* ����̽��һЩʱ�� */

		/* ���������Ѿ���������Ӧ��ֵ */
		if (short_irq == 0)/* û���ҵ�? */
		{ 
			printk(KERN_INFO "short: no irq reported by probe\n");
		}
		/*
		 ������ж���ж��߱��������Ϊ��
		 ����Ӧ�÷�����жϣ�lpt�˿ڲ�����Ҫ�����ٴ�����
		 ����������
		 */
	} while (short_irq <=0 && count++ < 5);

	/* ��ѭ�������� ж�ش������� */
	for (i = 0; trials[i]; i++)
	{
		if (tried[i] == 0)
		{
			free_irq(trials[i], NULL);
		}
	}

	if (short_irq < 0)
	{
		printk("short: probe failed %i times, giving up\n", count);
	}
}



/* Finally, init and cleanup */

int short_init(void)
{
	int result;

	/*
	 * first, sort out the base/short_base ambiguity: we'd better
	 * use short_base in the code, for clarity, but allow setting
	 * just "base" at load time. Same for "irq".
	 */
	short_base = base;
	short_irq = irq;

	/* Get our needed resources. */
	if (!use_mem)
	{
		//short_base�ǲ���ʹ��i/o��ַ�ռ�Ļ���ַ���򲢿ڵ�2�żĴ���д����������ն˱���
		if (! request_region(short_base, SHORT_NR_PORTS, "short"))
		{
			printk(KERN_INFO "short: can't get I/O port address 0x%lx\n",
					short_base);
			return -ENODEV;
		}

	}
	else
	{
		if (! request_mem_region(short_base, SHORT_NR_PORTS, "short"))
		{
			printk(KERN_INFO "short: can't get I/O mem address 0x%lx\n",
					short_base);
			return -ENODEV;
		}

		/* also, ioremap it */
		short_base = (unsigned long) ioremap(short_base, SHORT_NR_PORTS);
		/* Hmm... we should check the return value */
	}
	/* Here we register our device - should not fail thereafter */
	result = register_chrdev(major, "short", &short_fops);
	if (result < 0)
	{
		printk(KERN_INFO "short: can't get major number\n");
		release_region(short_base,SHORT_NR_PORTS);  /* FIXME - use-mem case? */
		return result;
	}
	if (major == 0)
	{
		major = result; /* dynamic */
	}

	short_buffer = __get_free_pages(GFP_KERNEL,0); /* never fails */  /* FIXME */
	short_head = short_tail = short_buffer;

	/*
	 * Fill the workqueue structure, used for the bottom half handler.
	 * The cast is there to prevent warnings about the type of the
	 * (unused) argument.
	 */
	/* �������г����� short_init() �� */
	INIT_WORK(&short_wq, (void (*)(void *)) short_do_tasklet, NULL);

	/*
	 * Now we deal with the interrupt: either kernel-based
	 * autodetection, DIY detection or default number
	 */

	if (short_irq < 0 && probe == 1)
	{
		short_kernelprobe();
	}

	if (short_irq < 0 && probe == 2)
	{
		short_selfprobe();
	}

	if (short_irq < 0) /* not yet specified: force the default on */
	{
		switch(short_base)
		{
		    case 0x378:
				short_irq = 7; 
				break;
		    case 0x278:
				short_irq = 2;
				break;
		    case 0x3bc:
				short_irq = 5;
				break;
		}
	}

	/*
	 * If shared has been specified, installed the shared handler
	 * instead of the normal one. Do it first, before a -EBUSY will
	 * force short_irq to -1.
	 */
	if (short_irq >= 0 && share > 0)
	{
		/*
		int request_irq(unsigned int irq, void (*handler)(int irq, void *dev_id, struct pt_regs *regs), unsigned long irqflags, const char * devname, void *dev_id);
		rq��Ҫ�����Ӳ���жϺš���Intelƽ̨����Χ0--15��
		handler����ϵͳ�Ǽǵ��жϴ�����������һ���ص��������жϷ���ʱ��ϵͳ�����������������Ĳ�������Ӳ���жϺţ�device id���Ĵ���ֵ��dev_id���������request_irqʱ���ݸ�ϵͳ�Ĳ���dev_id��
		irqflags���жϴ����һЩ���ԡ��Ƚ���Ҫ����SA_INTERRUPT�������жϴ�������ǿ��ٴ������(����SA_INTERRUPT)�������ٴ������(������SA_INTERRUPT)�����ٴ�����򱻵���ʱ���ε�ǰ�������ϵ������жϣ�������������Ȼ���Դ����жϡ����ٴ���������Ρ�����һ��SA_SHIRQ���ԣ��������Ժ����ж���豸�����жϡ�
		devname ��һ���ַ���������ж�����������ƣ���/proc/interrupt�пɿ�����
		dev_id���жϹ���ʱ���õ���һ������Ϊ����豸�� device�ṹ�������NULL������ע�Ṳ���ж�ʱ���˲�������ΪNULL���жϴ�����������dev_id�ҵ���Ӧ�Ŀ�������жϵ��豸��������irq2dev_map�ҵ��ж϶�Ӧ���豸��
		*/
		result = request_irq(short_irq, short_sh_interrupt,
				SA_SHIRQ | SA_INTERRUPT,"short",
				short_sh_interrupt);
		if (result)
		{
			printk(KERN_INFO "short: can't get assigned irq %i\n", short_irq);
			short_irq = -1;
		}
		else /* ���������ж� �ٶ�����һ������ */
		{
			outb(0x10, short_base+2);
		}
		return 0; /* the rest of the function only installs handlers */
	}

	if (short_irq >= 0)
	{
		result = request_irq(short_irq, short_interrupt,
				SA_INTERRUPT, "short", NULL);
		if (result)
		{
			printk(KERN_INFO "short: can't get assigned irq %i\n",
					short_irq);
			short_irq = -1;
		}
		else /* ���������ж� �ٶ�����һ������ */
		{ 
			outb(0x10,short_base+2);
		}
	}

	/*
	 * Ok, now change the interrupt handler if using top/bottom halves
	 * has been requested
	 */
	if (short_irq >= 0 && (wq + tasklet) > 0)
	{
		free_irq(short_irq,NULL);
		result = request_irq(short_irq,
				tasklet ? short_tl_interrupt :
				short_wq_interrupt,
				SA_INTERRUPT,"short-bh", NULL);
		if (result)
		{
			printk(KERN_INFO "short-bh: can't get assigned irq %i\n",
					short_irq);
			short_irq = -1;
		}
	}

	return 0;
}

void short_cleanup(void)
{
	if (short_irq >= 0)
	{
		outb(0x0, short_base + 2);   /* disable the interrupt */
		if (!share)
		{
			//void free_irq(unsigned int irq,void *dev_id);
			free_irq(short_irq, NULL);
		}
		else
		{
			free_irq(short_irq, short_sh_interrupt);
		}
	}
	/* Make sure we don't leave work queue/tasklet functions running */
	if (tasklet)
	{
		tasklet_disable(&short_tasklet);
	}
	else
	{
		flush_scheduled_work();
	}
	unregister_chrdev(major, "short");
	if (use_mem)
	{
		iounmap((void __iomem *)short_base);
		release_mem_region(short_base, SHORT_NR_PORTS);
	}
	else 
	{
		release_region(short_base,SHORT_NR_PORTS);
	}
	if (short_buffer)
	{
		free_page(short_buffer);
	}
}

module_init(short_init);
module_exit(short_cleanup);

//request_irq() ���������ж�Դ��װ�жϴ������
//free_irq()    �ͷŷ�����Ѷ��жϵ��ڴ�
//enable_irq()  �����жϿ��ƺ���ʹ�����ж�����Ч
//disable_irq() ʹ�����ж���ʧЧ