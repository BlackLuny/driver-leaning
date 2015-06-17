/*
 * scull.h -- definitions for the char module
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
 * $Id: scull.h,v 1.15 2004/11/04 17:51:18 rubini Exp $
 */

#ifndef _SCULL_H_
#define _SCULL_H_

#include <linux/ioctl.h> /* needed for the _IOW etc stuff used later */

/*
 * Macros to help debugging
 */

#undef PDEBUG             /* undef it, just in case */
#ifdef SCULL_DEBUG
#  ifdef __KERNEL__
     /* This one if debugging is on, and kernel space */
#    define PDEBUG(fmt, args...) printk( KERN_DEBUG "scull: " fmt, ## args)
#  else
     /* This one for user space */
#    define PDEBUG(fmt, args...) fprintf(stderr, fmt, ## args)
#  endif
#else
#  define PDEBUG(fmt, args...) /* not debugging: nothing */
#endif

#undef PDEBUGG
#define PDEBUGG(fmt, args...) /* nothing: it's a placeholder */

#ifndef SCULL_MAJOR
#define SCULL_MAJOR 0   /* dynamic major by default */
#endif

#ifndef SCULL_NR_DEVS
#define SCULL_NR_DEVS 4    /* scull0 through scull3 */
#endif

#ifndef SCULL_P_NR_DEVS
#define SCULL_P_NR_DEVS 4  /* scullpipe0 through scullpipe3 */
#endif

/*
 * The bare device is a variable-length region of memory.
 * Use a linked list of indirect blocks.
 *
 * "scull_dev->data" points to an array of pointers, each
 * pointer refers to a memory area of SCULL_QUANTUM bytes.
 *
 * The array (quantum-set) is SCULL_QSET long.
 */
#ifndef SCULL_QUANTUM
#define SCULL_QUANTUM 4000
#endif

#ifndef SCULL_QSET
#define SCULL_QSET    1000
#endif

/*
 * The pipe device is a simple circular buffer. Here its default size
 */
#ifndef SCULL_P_BUFFER
#define SCULL_P_BUFFER 4000
#endif

/*
 * Representation of scull quantum sets.
 data指向一个指针数组的首地址 这个数组有1000个指针，此数组成为量子集 每个指针指向一块内存区域，这块内存区域有4000字节大小 此内存区域称为量子
 所以scull_qset一个结构可以最多引用1000*4000个字节
 */
struct scull_qset {
	void **data;				//量子集    *data量子
	struct scull_qset *next;	//下一个量子集
};

struct scull_dev {
	struct scull_qset *data;  /* Pointer to first quantum set  指向第一个量子集的指针*/
	int quantum;              /* the current quantum size 当前量子的大小 默认4000*/
	int qset;                 /* the current array size 当前数组的大小 默认1000*/
	unsigned long size;       /* amount of data stored here 保存在其中的数据总量*/
	unsigned int access_key;  /* used by sculluid and scullpriv 由sculluid和scullpriv使用*/
	struct semaphore sem;     /* mutual exclusion semaphore 互斥信号量*/
	struct cdev cdev;		/* Char device structure 字符设备结构*/
};

/*
 * Split minors in two parts
 */
#define TYPE(minor)	(((minor) >> 4) & 0xf)	/* high nibble */
#define NUM(minor)	((minor) & 0xf)		/* low  nibble */


/*
 * The different configurable parameters
 */
extern int scull_major;     /* main.c */
extern int scull_nr_devs;
extern int scull_quantum;
extern int scull_qset;

extern int scull_p_buffer;	/* pipe.c */


/*
 * Prototypes for shared functions
 */

int     scull_p_init(dev_t dev);
void    scull_p_cleanup(void);
int     scull_access_init(dev_t dev);
void    scull_access_cleanup(void);

int     scull_trim(struct scull_dev *dev);

ssize_t scull_read(struct file *filp, char __user *buf, size_t count,
                   loff_t *f_pos);
ssize_t scull_write(struct file *filp, const char __user *buf, size_t count,
                    loff_t *f_pos);
loff_t  scull_llseek(struct file *filp, loff_t off, int whence);
int     scull_ioctl(struct inode *inode, struct file *filp,
                    unsigned int cmd, unsigned long arg);


/*
 * Ioctl definitions
 */

/* 使用 k 作为幻数 */
#define SCULL_IOC_MAGIC  'k'
/* Please use a different 8-bit number in your code */

#define SCULL_IOCRESET    _IO(SCULL_IOC_MAGIC, 0)

/*
 * S 表示通过指针设置,
 * T 表示直接用参数值 通知 Tell
 * G 表示获取 Get 通过设置指针来应答
 * Q 表示查询 Query 通过返回值应答
 * X 交换 Exchange 原子的交换 G 和 S
 * H 切换 Shift 原子的切换 T 和 Q
 */
// int cmd 高八位 幻数
//#define _IOC_NRBITS 8 //序数（number）字段的字位宽度，8bits
//#define _IOC_TYPEBITS 8 //幻数（type）字段的字位宽度，8bits
//#define _IOC_SIZEBITS 14 //大小（size）字段的字位宽度，14bits
//#define _IOC_DIRBITS 2 //方向（direction）字段的字位宽度，2bits
//#define _IOC_NRMASK ((1 << _IOC_NRBITS)-1) //序数字段的掩码，0x000000FF
//#define _IOC_TYPEMASK ((1 << _IOC_TYPEBITS)-1) //幻数字段的掩码，0x000000FF
//#define _IOC_SIZEMASK ((1 << _IOC_SIZEBITS)-1) //大小字段的掩码，0x00003FFF
//#define _IOC_DIRMASK ((1 << _IOC_DIRBITS)-1) //方向字段的掩码，0x00000003
//#define _IOC_NRSHIFT 0 //序数字段在整个字段中的位移，0
//#define _IOC_TYPESHIFT (_IOC_NRSHIFT+_IOC_NRBITS) //幻数字段的位移，8
//#define _IOC_SIZESHIFT (_IOC_TYPESHIFT+_IOC_TYPEBITS) //大小字段的位移，16
//#define _IOC_DIRSHIFT (_IOC_SIZESHIFT+_IOC_SIZEBITS) //方向字段的位移，30
//#define _IOC(dir,type,nr,size) (((dir) << _IOC_DIRSHIFT) | ((type) << _IOC_TYPESHIFT) | ((nr) << _IOC_NRSHIFT) | ((size) << _IOC_SIZESHIFT))
//最高2位是方向位 取值=_IOC_NONE 0U //没有数据传输 _IOC_WRITE 1U //向设备写入数据，驱动程序必须从用户空间读入数据 _IOC_READ 2U //从设备中读取数据，驱动程序必须向用户空间写入数据
//接下来8位是幻数位 整个驱动程序中使用这个号码
//接下来8位是序数位
//接下来的14位是用户数据大小 系统并不强制使用这个字段

//构造无参数的命令编号 #define _IO(type,nr) _IOC(_IOC_NONE,(type),(nr),0)
//构造从驱动程序中读取数据的命令编号 #define _IOR(type,nr,size) _IOC(_IOC_READ,(type),(nr),sizeof(size))
//用于向驱动程序写入数据命令 #define _IOW(type,nr,size) _IOC(_IOC_WRITE,(type),(nr),sizeof(size))
//用于双向传输 #define _IOWR(type,nr,size) _IOC(_IOC_READ|_IOC_WRITE,(type),(nr),sizeof(size))
#define SCULL_IOCSQUANTUM _IOW(SCULL_IOC_MAGIC,  1, int)
#define SCULL_IOCSQSET    _IOW(SCULL_IOC_MAGIC,  2, int)
#define SCULL_IOCTQUANTUM _IO(SCULL_IOC_MAGIC,   3)
#define SCULL_IOCTQSET    _IO(SCULL_IOC_MAGIC,   4)
#define SCULL_IOCGQUANTUM _IOR(SCULL_IOC_MAGIC,  5, int)
#define SCULL_IOCGQSET    _IOR(SCULL_IOC_MAGIC,  6, int)
#define SCULL_IOCQQUANTUM _IO(SCULL_IOC_MAGIC,   7)
#define SCULL_IOCQQSET    _IO(SCULL_IOC_MAGIC,   8)
#define SCULL_IOCXQUANTUM _IOWR(SCULL_IOC_MAGIC, 9, int)
#define SCULL_IOCXQSET    _IOWR(SCULL_IOC_MAGIC,10, int)
#define SCULL_IOCHQUANTUM _IO(SCULL_IOC_MAGIC,  11)
#define SCULL_IOCHQSET    _IO(SCULL_IOC_MAGIC,  12)

/*
 * The other entities only have "Tell" and "Query", because they're
 * not printed in the book, and there's no need to have all six.
 * (The previous stuff was only there to show different ways to do it.
 */
#define SCULL_P_IOCTSIZE _IO(SCULL_IOC_MAGIC,   13)
#define SCULL_P_IOCQSIZE _IO(SCULL_IOC_MAGIC,   14)
/* ... more to come */

#define SCULL_IOC_MAXNR 14

#endif /* _SCULL_H_ */
