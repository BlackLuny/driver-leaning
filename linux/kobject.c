/******************************************************************************
 * @file    : test_kobject_default_attr.c
 * @author  : wangyubin
 * @date    : Tue Dec 24 10:28:09 2013
 * 
 * @brief   : 测试 kobject 的默认属性的创建和删除
 * history  : init
 ******************************************************************************/

#include<linux/init.h>
#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/kobject.h>
#include<linux/sysfs.h>

MODULE_LICENSE("Dual BSD/GPL");

static void myobj_release(struct kobject*);
static ssize_t my_show(struct kobject *, struct attribute *, char *);
static ssize_t my_store(struct kobject *, struct attribute *, const char *, size_t);

/* 自定义的结构体，2个属性，并且嵌入了kobject */
struct my_kobj 
{
    int ival;
    char* cname;
    struct kobject kobj;
};

static struct my_kobj *myobj = NULL;

/* my_kobj 的属性 ival 所对应的sysfs中的文件，文件名 val */
static struct attribute val_attr = {
    .name = "val",
    .owner = NULL,
    .mode = 0666,
};

/* my_kobj 的属性 cname 所对应的sysfs中的文件，文件名 name */
static struct attribute name_attr = {
    .name = "name",
    .owner = NULL,
    .mode = 0666,
};

static int test_kobject_default_attr_init(void)
{
    struct attribute *myattrs[] = {NULL, NULL, NULL};
    struct sysfs_ops *myops = NULL;
    struct kobj_type *mytype = NULL;

    /* 初始化 myobj */
    myobj = kmalloc(sizeof(struct my_kobj), GFP_KERNEL);
    if (myobj == NULL)
        return -ENOMEM;

    /* 配置文件 val 的默认值 */
    myobj->ival = 100;
    myobj->cname = "test";

    /* 初始化 ktype */
    mytype = kmalloc(sizeof(struct kobj_type), GFP_KERNEL);
    if (mytype == NULL)
        return -ENOMEM;

    /* 增加2个默认属性文件 */
    myattrs[0] = &val_attr;
    myattrs[1] = &name_attr;

    /* 初始化ktype的默认属性和析构函数 */
    mytype->release = myobj_release;
    mytype->default_attrs = myattrs;

    /* 初始化ktype中的 sysfs */
    myops = kmalloc(sizeof(struct sysfs_ops), GFP_KERNEL);
    if (myops == NULL)
        return -ENOMEM;

    myops->show = my_show;
    myops->store = my_store;
    mytype->sysfs_ops = myops;

    /* 初始化kobject，并加入到sysfs中 */
    memset(&myobj->kobj, 0, sizeof(struct kobject)); /* 这一步非常重要，没有这一步init kobject会失败 */
    if (kobject_init_and_add(&myobj->kobj, mytype, NULL, "test_kobj_default_attr"))
        kobject_put(&myobj->kobj);

    printk(KERN_ALERT "*************************\n");
    printk(KERN_ALERT "test_kobject_default_attr is inited!\n");
    printk(KERN_ALERT "*************************\n");
    
    return 0;
}

static void test_kobject_default_attr_exit(void)
{
    kobject_del(&myobj->kobj);
    kfree(myobj);
    
    /* 退出内核模块 */
    printk(KERN_ALERT "*************************\n");
    printk(KERN_ALERT "test_kobject_default_attr is exited!\n");
    printk(KERN_ALERT "*************************\n");
    printk(KERN_ALERT "\n\n\n\n\n");
}

static void myobj_release(struct kobject *kobj) 
{
    printk(KERN_ALERT, "release kobject");
    kobject_del(kobj);
}

/* 读取属性文件 val 或者name时会执行此函数 */
static ssize_t my_show(struct kobject *kboj, struct attribute *attr, char *buf) 
{
    printk(KERN_ALERT "SHOW -- attr-name: [%s]\n", attr->name);    
    if (strcmp(attr->name, "val") == 0)
        return sprintf(buf, "%d\n", myobj->ival);
    else
        return sprintf(buf, "%s\n", myobj->cname);
}

/* 写入属性文件 val 或者name时会执行此函数 */
static ssize_t my_store(struct kobject *kobj, struct attribute *attr, const char *buf, size_t len) 
{
    printk(KERN_ALERT "STORE -- attr-name: [%s]\n", attr->name);
    if (strcmp(attr->name, "val") == 0)
        sscanf(buf, "%d\n", &myobj->ival);
    else
        sscanf(buf, "%s\n", myobj->cname);        
    return len;
}

module_init(test_kobject_default_attr_init);
module_exit(test_kobject_default_attr_exit);