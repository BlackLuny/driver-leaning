/*
 * $Id: usbmouse.c,v 1.15 2001/12/27 10:37:41 vojtech Exp $
 *
 *  Copyright (c) 1999-2001 Vojtech Pavlik
 *
 *  USB HIDBP Mouse support
 */

#include <linux/kernel.h> 
#include <linux/slab.h> 
#include <linux/module.h> 
#include <linux/init.h> 
#include <linux/usb/input.h> 
#include <linux/hid.h> 

/*
 * Version Information
 */
#define DRIVER_VERSION "v1.6" 
#define DRIVER_AUTHOR "Vojtech Pavlik <vojtech@ucw.cz>" 
#define DRIVER_DESC "USB HID Boot Protocol mouse driver" 
#define DRIVER_LICENSE "GPL" 

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE(DRIVER_LICENSE);

/*
 * ���ṹ�壬������������豸��
 */
struct usb_mouse 
{
    /* ����豸�����ƣ������������̡���Ʒ��𡢲�Ʒ����Ϣ */
    char name[128]; 
    /* �豸�ڵ����� */
    char phys[64];  
    /* USB �����һ�� USB �豸����Ҫ��Ƕһ�� USB �豸�ṹ���������� USB ���� */
    struct usb_device *usbdev;
    /* USB ���ͬʱ����һ�������豸����Ҫ��Ƕһ�������豸�ṹ���������������豸������ */
    struct input_dev *dev;  
    /* URB ������ṹ�壬���ڴ������� */
    struct urb *irq;
    /* ��ͨ�����õĵ�ַ */
    signed char *data;
    /* dma �����õĵ�ַ */
    dma_addr_t data_dma;        
};

/*
 * urb �ص�������������ύ urb ��urb �ص������������á�
 * �˺�����Ϊ usb_fill_int_urb �������βΣ�Ϊ������ urb �ƶ��Ļص�������
 */
static void usb_mouse_irq(struct urb *urb)
{
    /*
     * urb �е� context ָ������Ϊ USB �������򱣴�һЩ���ݡ�����������ص��������β�û�д����� probe
     * ��Ϊ mouse �ṹ�������ǿ��ڴ�ĵ�ַָ�룬������Ҫ�õ��ǿ��ڴ������е����ݣ�context ָ�������
     * ��æ�ˣ�
     * ����� urb ʱ�� context ָ��ָ�� mouse �ṹ���������������ִ���һ���ֲ� mouse ָ��ָ���� probe
     * ������Ϊ mouse ������ǿ��ڴ棬�ǿ��ڴ汣���ŷǳ���Ҫ���ݡ�
     * �� urb ͨ�� USB core �ύ�� hc ֮��������������mouse->data ָ����ڴ����򽫱��������İ���
     * ���ƶ�������Ϣ��ϵͳ��������Щ��Ϣ��������Ϊ������Ӧ�� 
     * mouse ����Ƕ�� dev ָ�룬ָ�� input_dev �����ڵ��ڴ�����
     */
    struct usb_mouse *mouse = urb->context;
    signed char *data = mouse->data;
    struct input_dev *dev = mouse->dev;
    int status;

    /*
     * status ֵΪ 0 ��ʾ urb �ɹ����أ�ֱ������ѭ��������¼������������ϵͳ��
     * ECONNRESET ������Ϣ��ʾ urb �� usb_unlink_urb ������ unlink �ˣ�ENOENT ������Ϣ��ʾ urb �� 
     * usb_kill_urb ������ kill �ˡ�usb_kill_urb ��ʾ���׽��� urb ���������ڣ��� usb_unlink_urb ��
     * ��ֹͣ urb������������� urb ��ȫ��ֹ�ͻ᷵�ظ��ص����������������жϴ������ʱ���ߵȴ�ĳ������
     * ʱ�ǳ����ã���������������ǲ���˯�ߵģ����ȴ�һ�� urb ��ȫֹͣ�ܿ��ܻ����˯�ߵ������
     * ESHUTDOWN ���ִ����ʾ USB �����������������������صĴ��󣬻����ύ�� urb ��һ˲���豸���γ���
     * ���������������ִ�������Ĵ��󣬽������ش� urb��
     */
    switch (urb->status)
    {
    case 0:     /* success */
        break;
    case -ECONNRESET:   /* unlink */
    case -ENOENT:
    case -ESHUTDOWN:
        return;
    /* -EPIPE:  should clear the halt */
    default:        /* error */
        goto resubmit;
    }

    /*
     * ��������ϵͳ�㱨����¼�������Ա�������Ӧ��
     * data ����ĵ�0���ֽڣ�bit 0��1��2��3��4�ֱ�������ҡ��С�SIDE��EXTRA���İ��������
     * data ����ĵ�1���ֽڣ���ʾ����ˮƽλ�ƣ�
     * data ����ĵ�2���ֽڣ���ʾ���Ĵ�ֱλ�ƣ�
     * data ����ĵ�3���ֽڣ�REL_WHEELλ�ơ�
     */
    input_report_key(dev, BTN_LEFT,   data[0] & 0x01);
    input_report_key(dev, BTN_RIGHT,  data[0] & 0x02);
    input_report_key(dev, BTN_MIDDLE, data[0] & 0x04);
    input_report_key(dev, BTN_SIDE,   data[0] & 0x08);
    input_report_key(dev, BTN_EXTRA,  data[0] & 0x10);
    input_report_rel(dev, REL_X,     data[1]);
    input_report_rel(dev, REL_Y,     data[2]);
    input_report_rel(dev, REL_WHEEL, data[3]);

    /*
     * �����������¼�ͬ�������漸����һ������������¼�������������Ϣ������������Ϣ�͹�����Ϣ��������
     * ϵͳ����ͨ�����ͬ���ź����ڶ�������¼�����������ÿһ�������¼����档ʾ�����£�
     * ������Ϣ ����λ����Ϣ ������Ϣ EV_SYC | ������Ϣ ����λ����Ϣ ������Ϣ EV_SYC ...
     */
    input_sync(dev);

    /*
     * ϵͳ��Ҫ�����Բ��ϵػ�ȡ�����¼���Ϣ������� urb �ص�������ĩβ�ٴ��ύ urb ����飬�����ֻ�
     * �����µĻص��������ܶ���ʼ��
     * �ڻص��������ύ urb һ��ֻ���� GFP_ATOMIC ���ȼ��ģ���Ϊ urb �ص������������ж��������У�����
     * �� urb �����п��ܻ���Ҫ�����ڴ桢�����ź�������Щ��������ᵼ�� USB core ˯�ߣ�һ�е���˯�ߵ���
     * Ϊ���ǲ�����ġ�
     */
resubmit:
    status = usb_submit_urb (urb, GFP_ATOMIC);
    if (status)
        err ("can't resubmit intr, %s-%s/input0, status %d",
                mouse->usbdev->bus->bus_name,
                mouse->usbdev->devpath, status);
}

/*
 * ������豸ʱ����ʼ�ύ�� probe �����й����� urb������ urb ���ڡ�
 */
static int usb_mouse_open(struct input_dev *dev)
{
    struct usb_mouse *mouse = dev->private;

    mouse->irq->dev = mouse->usbdev;
    if (usb_submit_urb(mouse->irq, GFP_KERNEL))
        return -EIO;

    return 0;
}

/*
 * �ر�����豸ʱ������ urb �������ڡ�
 */
static void usb_mouse_close(struct input_dev *dev)
{
    struct usb_mouse *mouse = dev->private;

    usb_kill_urb(mouse->irq);
}

/*
 * ���������̽�⺯��
 */
static int usb_mouse_probe(struct usb_interface *intf, const struct usb_device_id *id)
{
    /* 
     * �ӿڽṹ��������豸�ṹ���У�interface_to_usbdev ��ͨ���ӿڽṹ���������豸�ṹ�塣
     * usb_host_interface �����������ӿ����õĽṹ�壬��Ƕ�ڽӿڽṹ�� usb_interface �С�
     * usb_endpoint_descriptor �Ƕ˵��������ṹ�壬��Ƕ�ڶ˵�ṹ�� usb_host_endpoint �У����˵�
     * �ṹ����Ƕ�ڽӿ����ýṹ���С�
     */
    struct usb_device *dev = interface_to_usbdev(intf);
    struct usb_host_interface *interface;
    struct usb_endpoint_descriptor *endpoint;
    struct usb_mouse *mouse;
    struct input_dev *input_dev;
    int pipe, maxp;

    interface = intf->cur_altsetting;

    /* ������һ�� interrupt ���͵� in �˵㣬�������Ҫ����豸������ */
    if (interface->desc.bNumEndpoints != 1)
        return -ENODEV;

    endpoint = &interface->endpoint[0].desc;
    if (!usb_endpoint_is_int_in(endpoint))
        return -ENODEV;

    /*
     * ���ض�Ӧ�˵��ܹ�������������ݰ������ķ��ص�������ݰ�Ϊ4���ֽڣ����ݰ����������� urb
     * �ص�����������ϸ˵����
     */
    pipe = usb_rcvintpipe(dev, endpoint->bEndpointAddress);
    maxp = usb_maxpacket(dev, pipe, usb_pipeout(pipe));

    /* Ϊ mouse �豸�ṹ������ڴ� */
    mouse = kzalloc(sizeof(struct usb_mouse), GFP_KERNEL);
    /* input_dev */
    input_dev = input_allocate_device();
    if (!mouse || !input_dev)
        goto fail1;

    /*
     * �����ڴ�ռ��������ݴ��䣬data Ϊָ��ÿռ�ĵ�ַ��data_dma ��������ڴ�ռ�� dma ӳ�䣬
     * ������ڴ�ռ��Ӧ�� dma ��ַ����ʹ�� dma ���������£���ʹ�� data_dma ָ��� dma ����
     * ����ʹ�� data ָ�����ͨ�ڴ�������д��䡣
     * GFP_ATOMIC ��ʾ���ȴ���GFP_KERNEL ����ͨ�����ȼ�������˯�ߵȴ����������ʹ���жϴ��䷽ʽ��
     * ������˯��״̬��data ���������Ի�ȡ����¼��Ĵ洢�������ʹ�� GFP_ATOMIC ���ȼ����������
     * ���䵽�ڴ����������� 0��
     */
    mouse->data = usb_buffer_alloc(dev, 8, GFP_ATOMIC, &mouse->data_dma);
    if (!mouse->data)
        goto fail1;

    /*
     * Ϊ urb �ṹ�������ڴ�ռ䣬��һ��������ʾ��ʱ����ʱ��Ҫ���Ͱ����������������䷽ʽ��Ϊ0��
     * ������ڴ潫ͨ�����漴�������� usb_fill_int_urb ����������䡣 
     */
    mouse->irq = usb_alloc_urb(0, GFP_KERNEL);
    if (!mouse->irq)
        goto fail2;

    /* ��� usb �豸�ṹ��������豸�ṹ�� */
    mouse->usbdev = dev;
    mouse->dev = input_dev;

    /* ��ȡ����豸������ */
    if (dev->manufacturer)
        strlcpy(mouse->name, dev->manufacturer, sizeof(mouse->name));

    if (dev->product) 
    {
        if (dev->manufacturer)
            strlcat(mouse->name, " ", sizeof(mouse->name));
        strlcat(mouse->name, dev->product, sizeof(mouse->name));
    }

    if (!strlen(mouse->name))
        snprintf(mouse->name, sizeof(mouse->name),
             "USB HIDBP Mouse %04x:%04x",
             le16_to_cpu(dev->descriptor.idVendor),
             le16_to_cpu(dev->descriptor.idProduct));

    /*
     * �������豸�ṹ���еĽڵ�����usb_make_path ������ȡ USB �豸�� Sysfs �е�·������ʽ
     * Ϊ��usb-usb ���ߺ�-·������
     */
    usb_make_path(dev, mouse->phys, sizeof(mouse->phys));
    strlcat(mouse->phys, "/input0", sizeof(mouse->phys));

    /* ������豸�����Ƹ�������豸��Ƕ��������ϵͳ�ṹ�� */
    input_dev->name = mouse->name;
    /* ������豸���豸�ڵ�����������豸��Ƕ��������ϵͳ�ṹ�� */
    input_dev->phys = mouse->phys;
    /*
     * input_dev �е� input_id �ṹ�壬�����洢���̡��豸���ͺ��豸�ı�ţ���������ǽ��豸������
     * �еı�Ÿ�����Ƕ��������ϵͳ�ṹ��
     */
    usb_to_input_id(dev, &input_dev->id);
    /* cdev ���豸�������class device�� */
    input_dev->cdev.dev = &intf->dev;

    /* evbit ���������¼���EV_KEY �ǰ����¼���EV_REL ����������¼� */
    input_dev->evbit[0] = BIT(EV_KEY) | BIT(EV_REL);
    /* keybit ��ʾ��ֵ������������Ҽ����м� */
    input_dev->keybit[LONG(BTN_MOUSE)] = BIT(BTN_LEFT) | BIT(BTN_RIGHT) | BIT(BTN_MIDDLE);
    /* relbit ���ڱ�ʾ�������ֵ */
    input_dev->relbit[0] = BIT(REL_X) | BIT(REL_Y);
    /* �е���껹���������� */
    input_dev->keybit[LONG(BTN_MOUSE)] |= BIT(BTN_SIDE) | BIT(BTN_EXTRA);
    /* �м����ֵĹ���ֵ */
    input_dev->relbit[0] |= BIT(REL_WHEEL);

    /* input_dev �� private ���������ڱ�ʾ��ǰ�����豸�����࣬���ｫ���ṹ����󸳸��� */
    input_dev->private = mouse;
    /* ��������豸�򿪺���ָ�� */
    input_dev->open = usb_mouse_open;
    /* ��������豸�رպ���ָ�� */
    input_dev->close = usb_mouse_close;

    /*
     * ��乹�� urb�����ղ����õ� mouse �ṹ����������� urb �ṹ���У��� open �еݽ� urb��
     * �� urb ����һ����������� DMA ������ʱӦ������ URB_NO_TRANSFER_DMA_MAP��USB����ʹ��
     * transfer_dma������ָ��Ļ�������������transfer_buffer������ָ��ġ�
     * URB_NO_SETUP_DMA_MAP ���� Setup ����URB_NO_TRANSFER_DMA_MAP �������� Data ����
     */
    usb_fill_int_urb(mouse->irq, dev, pipe, mouse->data,
             (maxp > 8 ? 8 : maxp),
             usb_mouse_irq, mouse, endpoint->bInterval);
    mouse->irq->transfer_dma = mouse->data_dma;
    mouse->irq->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;

    /* ��ϵͳע�������豸 */
    input_register_device(mouse->dev);

    /*
     * һ���� probe �����У�����Ҫ���豸�����Ϣ������һ�� usb_interface �ṹ���У��Ա��Ժ�ͨ��
     * usb_get_intfdata ��ȡʹ�á���������豸�ṹ����Ϣ�������� intf �ӿڽṹ����Ƕ���豸�ṹ����
     * �� driver_data ���ݳ�Ա�У��� intf->dev->dirver_data = mouse��
     */
    usb_set_intfdata(intf, mouse);
    return 0;

fail2:  usb_buffer_free(dev, 8, mouse->data, mouse->data_dma);
fail1:  input_free_device(input_dev);
    kfree(mouse);
    return -ENOMEM;
}

/*
 * ����豸�γ�ʱ�Ĵ�����
 */
static void usb_mouse_disconnect(struct usb_interface *intf)
{
    /* ��ȡ����豸�ṹ�� */
    struct usb_mouse *mouse = usb_get_intfdata (intf);

    /* intf->dev->dirver_data = NULL�����ӿڽṹ���е�����豸ָ���ÿա�*/
    usb_set_intfdata(intf, NULL);
    if (mouse)
    {
        /* ���� urb �������� */
        usb_kill_urb(mouse->irq);
        /* ������豸��������ϵͳ��ע�� */
        input_unregister_device(mouse->dev);
        /* �ͷ� urb �洢�ռ� */
        usb_free_urb(mouse->irq);
        /* �ͷŴ������¼��� data �洢�ռ� */
        usb_buffer_free(interface_to_usbdev(intf), 8, mouse->data, mouse->data_dma);
        /* �ͷŴ�����ṹ��Ĵ洢�ռ� */
        kfree(mouse);
    }
}

/*
 * usb_device_id �ṹ�����ڱ�ʾ������������֧�ֵ��豸��USB_INTERFACE_INFO ��������ƥ���ض����͵Ľӿڣ�
 * �����Ĳ�����˼Ϊ (���, �����, Э��)��
 * USB_INTERFACE_CLASS_HID ��ʾ��һ�� HID (Human Interface Device)�����˻������豸���
 * USB_INTERFACE_SUBCLASS_BOOT ������𣬱�ʾ��һ�� boot �׶�ʹ�õ� HID��
 * USB_INTERFACE_PROTOCOL_MOUSE ��ʾ������豸����ѭ����Э�顣
 */
static struct usb_device_id usb_mouse_id_table [] = {
    { USB_INTERFACE_INFO(USB_INTERFACE_CLASS_HID, USB_INTERFACE_SUBCLASS_BOOT,
        USB_INTERFACE_PROTOCOL_MOUSE) },
    { } /* Terminating entry */
};

/*
 * ������������������û��ռ�ĳ���֪��������������ܹ�֧�ֵ��豸������ USB ����������˵����һ����������
 * �� usb��
 */
MODULE_DEVICE_TABLE (usb, usb_mouse_id_table);

/*
 * �����������ṹ��
 */
static struct usb_driver usb_mouse_driver = {
    .name       = "usbmouse",
    .probe      = usb_mouse_probe,
    .disconnect = usb_mouse_disconnect,
    .id_table   = usb_mouse_id_table,
};

/*
 * ���������������ڵĿ�ʼ�㣬�� USB core ע����������������
 */
static int __init usb_mouse_init(void)
{
    int retval = usb_register(&usb_mouse_driver);
    if (retval == 0)
        info(DRIVER_VERSION ":" DRIVER_DESC);
    return retval;
}

/*
 * ���������������ڵĽ����㣬�� USB core ע����������������
 */
static void __exit usb_mouse_exit(void)
{
    usb_deregister(&usb_mouse_driver);
}

module_init(usb_mouse_init);
module_exit(usb_mouse_exit);
