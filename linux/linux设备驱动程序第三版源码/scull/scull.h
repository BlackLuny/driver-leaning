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
 dataָ��һ��ָ��������׵�ַ ���������1000��ָ�룬�������Ϊ���Ӽ� ÿ��ָ��ָ��һ���ڴ���������ڴ�������4000�ֽڴ�С ���ڴ������Ϊ����
 ����scull_qsetһ���ṹ�����������1000*4000���ֽ�
 */
struct scull_qset {
	void **data;				//���Ӽ�    *data����
	struct scull_qset *next;	//��һ�����Ӽ�
};

struct scull_dev {
	struct scull_qset *data;  /* Pointer to first quantum set  ָ���һ�����Ӽ���ָ��*/
	int quantum;              /* the current quantum size ��ǰ���ӵĴ�С Ĭ��4000*/
	int qset;                 /* the current array size ��ǰ����Ĵ�С Ĭ��1000*/
	unsigned long size;       /* amount of data stored here ���������е���������*/
	unsigned int access_key;  /* used by sculluid and scullpriv ��sculluid��scullprivʹ��*/
	struct semaphore sem;     /* mutual exclusion semaphore �����ź���*/
	struct cdev cdev;		/* Char device structure �ַ��豸�ṹ*/
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

/* ʹ�� k ��Ϊ���� */
#define SCULL_IOC_MAGIC  'k'
/* Please use a different 8-bit number in your code */

#define SCULL_IOCRESET    _IO(SCULL_IOC_MAGIC, 0)

/*
 * S ��ʾͨ��ָ������,
 * T ��ʾֱ���ò���ֵ ֪ͨ Tell
 * G ��ʾ��ȡ Get ͨ������ָ����Ӧ��
 * Q ��ʾ��ѯ Query ͨ������ֵӦ��
 * X ���� Exchange ԭ�ӵĽ��� G �� S
 * H �л� Shift ԭ�ӵ��л� T �� Q
 */
// int cmd �߰�λ ����
//#define _IOC_NRBITS 8 //������number���ֶε���λ��ȣ�8bits
//#define _IOC_TYPEBITS 8 //������type���ֶε���λ��ȣ�8bits
//#define _IOC_SIZEBITS 14 //��С��size���ֶε���λ��ȣ�14bits
//#define _IOC_DIRBITS 2 //����direction���ֶε���λ��ȣ�2bits
//#define _IOC_NRMASK ((1 << _IOC_NRBITS)-1) //�����ֶε����룬0x000000FF
//#define _IOC_TYPEMASK ((1 << _IOC_TYPEBITS)-1) //�����ֶε����룬0x000000FF
//#define _IOC_SIZEMASK ((1 << _IOC_SIZEBITS)-1) //��С�ֶε����룬0x00003FFF
//#define _IOC_DIRMASK ((1 << _IOC_DIRBITS)-1) //�����ֶε����룬0x00000003
//#define _IOC_NRSHIFT 0 //�����ֶ��������ֶ��е�λ�ƣ�0
//#define _IOC_TYPESHIFT (_IOC_NRSHIFT+_IOC_NRBITS) //�����ֶε�λ�ƣ�8
//#define _IOC_SIZESHIFT (_IOC_TYPESHIFT+_IOC_TYPEBITS) //��С�ֶε�λ�ƣ�16
//#define _IOC_DIRSHIFT (_IOC_SIZESHIFT+_IOC_SIZEBITS) //�����ֶε�λ�ƣ�30
//#define _IOC(dir,type,nr,size) (((dir) << _IOC_DIRSHIFT) | ((type) << _IOC_TYPESHIFT) | ((nr) << _IOC_NRSHIFT) | ((size) << _IOC_SIZESHIFT))
//���2λ�Ƿ���λ ȡֵ=_IOC_NONE 0U //û�����ݴ��� _IOC_WRITE 1U //���豸д�����ݣ��������������û��ռ�������� _IOC_READ 2U //���豸�ж�ȡ���ݣ���������������û��ռ�д������
//������8λ�ǻ���λ ��������������ʹ���������
//������8λ������λ
//��������14λ���û����ݴ�С ϵͳ����ǿ��ʹ������ֶ�

//�����޲����������� #define _IO(type,nr) _IOC(_IOC_NONE,(type),(nr),0)
//��������������ж�ȡ���ݵ������� #define _IOR(type,nr,size) _IOC(_IOC_READ,(type),(nr),sizeof(size))
//��������������д���������� #define _IOW(type,nr,size) _IOC(_IOC_WRITE,(type),(nr),sizeof(size))
//����˫���� #define _IOWR(type,nr,size) _IOC(_IOC_READ|_IOC_WRITE,(type),(nr),sizeof(size))
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
