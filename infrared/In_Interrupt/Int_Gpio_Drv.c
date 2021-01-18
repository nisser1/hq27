#include <linux/delay.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/gfp.h>
#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/irq.h>
#include <linux/kernel.h>
#include <linux/kfifo.h>
#include <linux/kmod.h>
#include <linux/ktime.h>
#include <linux/major.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/poll.h>
#include <linux/proc_fs.h>
#include <linux/rtc.h>
#include <linux/seq_file.h>
#include <linux/slab.h>
#include <linux/stat.h>
#include <linux/timer.h>
#include <linux/timex.h>
#include <linux/tty.h>
#include <linux/version.h>
#include <linux/wait.h>
#include "./Int_Gpio.c"

/* fakechen Debug Information && operation */
#define DEBUG
#ifdef DEBUG
#define print(format, args...)                      \
    do {                                            \
        printk("[%s:%d] ", __FUNCTION__, __LINE__); \
        printk(format, ##args);                     \
    } while (0)
#else
#define print(format, args...)
#endif
#define Min(a, b) (a < b ? a : b)

#define Fake_Int_DEV_NUM 1                /* 设备号个数 */
#define Fake_Int_MEM_SIZE 2 * 1024 * 1024 /* 2M */
/* 定义IOCTl控制命令 */
#define Fake_MAGIC 'g'
#define Fake_IOC_RNEC _IOR(Fake_MAGIC, 0, int)

/* 创建操作的IOBUF */
static void *Io_Buf = NULL;

static int Gpio_Int_Count = 0;  // 中断计数
module_param(Gpio_Int_Count, int, S_IRUGO);

struct pin_desc {
    unsigned int pin;
    unsigned int key_val;
    unsigned int irq_pin;
};

struct pin_desc pins_desc[8] = {
    {Hi3519_GPIO5_6,0x00},
    {Hi3519_GPIO5_7,0x01},
    {Hi3519_GPIO16_0,0x02},
    {Hi3519_GPIO16_1,0x03},
    {Hi3519_GPIO16_2,0x04},
    {Hi3519_GPIO16_3,0x05},
    {Hi3519_GPIO16_4,0x06},
    {Hi3519_GPIO16_5,0x07},
};

static struct pin_desc *irq_pd;

/* 键值：按下时，0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07 */
/* 键值：松开时，0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87 */
static unsigned char key_val;


struct FakeIntGpioDev {
    dev_t devid;                          /* 设备号 */
    struct cdev cdev;                     /* cdev */
    struct class *class;                  /* 类  /sys/class下的类 */
    struct device *device;                /* 设备 /dev 下的设备 */
    int major;                            /* 主设备号 */
    int minor;                            /* 次设备号 */
    void __iomem *iobase;                 /* io基base */
    struct ztGpioPrivate FakeChenGI;      /* Gpio Int操作模块 */
    int interrupt_gpio1;                  /* 中断引脚1 */
    int interrupt_pin1;                   /* 中断号1 */
    struct i2c_adapter *Int_Gpio_Adapter; /* 适配器 */
    struct mutex lock;                    /* 互斥锁 */
    struct timer_list fake_timer;         /* 设备要使用的定时器 */
    unsigned char NecPrivate[4];          /* 红外私有数据 */
    struct fasync_struct *Nec_queue;      /* 异步通知 */
};

/* ① 确定主设备号，也可以让内核自动分配 */
/* 创建Hello设备结构体 */
struct FakeIntGpioDev FakeIntGpio;

/* ② 定义自己的 file_operations 结构体 */
/* ③ 实现对应的 drv_open/drv_read/drv_write 等函数，填入 file_operations 结构体
 */
static int FakeIntGpioOpen(struct inode *inode, struct file *file);
static int FakeIntGpioRelease(struct inode *inode, struct file *file);
static ssize_t FakeIntGpioWrite(struct file *file, const char __user *buf,
                                size_t size, loff_t *off);
static ssize_t FakeIntGpioRead(struct file *file, char __user *buf, size_t size,
                               loff_t *off);
static long FakeIntGpioIoctl(struct file *file, unsigned int cmd,
                             unsigned long arg);
static loff_t FakeIntGpioLseek(struct file *file, loff_t offset, int orig);
static int FakeIntGpioFasync(int fd, struct file *filp, int mode);

// ②定义自己的file_operation结构体
static const struct file_operations Fake_Int_ops = {
    .owner = THIS_MODULE,
    .open = FakeIntGpioOpen,
    .release = FakeIntGpioRelease,
    .write = FakeIntGpioWrite,
    .read = FakeIntGpioRead,
    .unlocked_ioctl = FakeIntGpioIoctl,
    .llseek = FakeIntGpioLseek,
    .fasync = FakeIntGpioFasync,
};

static void buttons_timer_function(unsigned long data)
{

    struct pin_desc * pindesc = (struct pin_desc *)irq_pd;
    unsigned int pinval;

    if(!pindesc) 
	return;

    pinval = gpio_get_value(pindesc->pin);

/* 键值：按下时，0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07 */
/* 键值：松开时，0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87 */
    if (pinval) {
        /* 松开 */
        key_val = 0x80 | pindesc->key_val;
    }
    else {
        /* 按下 */
        key_val = pindesc->key_val;
    }

    if (FakeIntGpio.Nec_queue) {
        kill_fasync(&FakeIntGpio.Nec_queue, SIGUSR2, POLL_IN);
    }

}

static irqreturn_t FakeChenIntIsr(int irq, void *dev_id)
{
    /* 10ms 启动定时器 */
   //disable_irq_nosync(irq);
    irq_pd = (struct pin_desc *)dev_id;
    mod_timer(&FakeIntGpio.fake_timer,jiffies+HZ/100);
   //enable_irq(irq);
    return IRQ_RETVAL(IRQ_HANDLED);

}


static int __init FakeIntGpioInit(void)
{
    int ret = 0;
    int i = 0;
    char* sbuf[8] = {"s0","s1","s2","s3","s4","s5","s6","s7"};
    memset(&FakeIntGpio, 0, sizeof(struct FakeIntGpioDev));

    FakeIntGpio.major = 0;  // 主设备号写0让内核自动分配

    if (!FakeIntGpio.major) {
        FakeIntGpio.devid = MKDEV(FakeIntGpio.major, 0);
        // Fake_Int_DEV_NUM 1
        ret = alloc_chrdev_region(&FakeIntGpio.devid, 0, Fake_Int_DEV_NUM,
                                  "FakeIntGpio_driver");
        FakeIntGpio.major = MAJOR(FakeIntGpio.devid);
        FakeIntGpio.minor = MINOR(FakeIntGpio.devid);
        print(
            "FakeIntGpio.devid = %d FakeIntGpio.major = %d  "
            "FakeIntGpio.minor = "
            "%d.	\r\n",
            FakeIntGpio.devid, FakeIntGpio.major, FakeIntGpio.minor);
    }

    FakeIntGpio.cdev.owner = THIS_MODULE;
    cdev_init(
        &FakeIntGpio.cdev,
        &Fake_Int_ops);

    ret = cdev_add(&FakeIntGpio.cdev, FakeIntGpio.devid, Fake_Int_DEV_NUM);

    FakeIntGpio.class = class_create(
        THIS_MODULE, "fakeChenIntGpio_class"); 

    FakeIntGpio.device =
        device_create(FakeIntGpio.class, NULL, FakeIntGpio.devid, NULL,
                      "fakeChenIntGpio_dev"); 

    /* 初始化Int_Gpio     8个按键输入中断 */
    ztInitGpioMap(&FakeIntGpio.FakeChenGI);

    /* 7.注册八个Gpio_Int 中断 */
    for ( i = 0 ; i < 8; i++ ){
       sprintf(sbuf[i],"s%d",i);
       pins_desc[i].irq_pin = gpio_to_irq(pins_desc[i].pin);
       ret = request_irq(pins_desc[i].irq_pin,FakeChenIntIsr,IRQF_TRIGGER_RISING || IRQF_TRIGGER_FALLING,sbuf[i],&pins_desc[i]);
       print("key%d   -----> irqname-%s  -----> irq_pin-%d\r\n",i,sbuf[i],pins_desc[i].irq_pin);
    }

    /* 8.初始化互斥锁 */
    mutex_init(&FakeIntGpio.lock);

    /* 9.初始化定时器 */
    init_timer(&FakeIntGpio.fake_timer);
    FakeIntGpio.fake_timer.function = buttons_timer_function;
    add_timer(&FakeIntGpio.fake_timer);

    return 0;
}

/* ⑥ 有入口函数就应该有出口函数：卸载驱动程序时，出口函数调用释放函数 */
static void __exit FakeIntGpioExit(void)
{
    int i = 0;
    print("Io_Buf = %p\r\n", Io_Buf);
    /* 释放申请的内存 */
    if (Io_Buf != NULL) {
        kfree(Io_Buf);
        Io_Buf = NULL;
    }

    if (FakeIntGpio.devid) {
        /* 释放占用的设备号 */
        unregister_chrdev_region(FakeIntGpio.devid, Fake_Int_DEV_NUM);
        /* 删除 cdev */
        cdev_del(&(FakeIntGpio.cdev));

        /* 删除 device */
        device_destroy(FakeIntGpio.class, FakeIntGpio.devid);
        /* 删除 class */
        class_destroy(FakeIntGpio.class);

        /* 释放Gpio资源 */
        ztReleaseGpioMap(&FakeIntGpio.FakeChenGI);

        /* 释放Gpio中断资源 */
        for ( i = 0 ; i < 8; i++ ){
        free_irq(pins_desc[i].irq_pin,&pins_desc[i]);
        }

    }
}

MODULE_LICENSE("GPL");
module_init(FakeIntGpioInit);
module_exit(FakeIntGpioExit);

static int FakeIntGpioOpen(struct inode *inode, struct file *file)
{
    file->private_data = &FakeIntGpio;
    return 0;
}

static int FakeIntGpioRelease(struct inode *inode, struct file *file)
{
    /* 从异步通知列表中删除 */
    FakeIntGpioFasync(-1, file, 0);
    return 0;
}

static ssize_t FakeIntGpioWrite(struct file *file, const char __user *buf,
                                size_t size, loff_t *off)
{
    unsigned long p = *off;
    unsigned int count = size;
    int ret = 0;
    struct FakeIntGpioDev *dev = (struct FakeIntGpioDev *)file->private_data;
    if (p >= Fake_Int_MEM_SIZE) {
        return 0;
    }
    if (count > (Fake_Int_MEM_SIZE - p)) {
        count = Fake_Int_MEM_SIZE - p;
    }

    if (copy_from_user(dev->iobase, buf, count)) {
        ret = -EFAULT;
    }
    else {
        *off += count;
        ret = count;
        print("write %u bytes(s) from %lu\r\nIo_Buf_Data:", count, p);
        {
            int i = 0;
            for (i = 0; i < count; i++) {
                printk("%d", *(char *)(dev->iobase + i));
            }
        }
    }
    return ret;
}

static ssize_t FakeIntGpioRead(struct file *file, char __user *buf, size_t size,
                               loff_t *off)
{
    int ret = 0;
    if (size != 1)
	return -EINVAL;

    ret = copy_to_user(buf,&key_val,1);

    return ret;
}

static loff_t FakeIntGpioLseek(struct file *file, loff_t offset, int orig)
{
    loff_t ret = 0;
    print("orig = %d	 offset = %d.\r\n", orig, (unsigned int)offset);
    switch (orig) {
        case 0: /* 从文件开头位置 seek */

            if (offset < 0) {
                ret = -EINVAL;
                break;
            }

            if ((unsigned int)offset > Fake_Int_MEM_SIZE) {
                ret = -EINVAL;
                break;
            }

            file->f_pos = offset;
            ret = (unsigned int)(file->f_pos);
            break;

        case 1: /* 从文件当前位置开始 seek */

            if ((file->f_pos + offset) < 0) {
                ret = -EINVAL;
                break;
            }

            if ((file->f_pos + offset) > Fake_Int_MEM_SIZE) {
                ret = -EINVAL;
                break;
            }

            file->f_pos += offset;
            ret = (unsigned int)(file->f_pos);
            break;
        default:
            ret = -EINVAL;
            break;
    }

    return ret;
}


static long FakeIntGpioIoctl(struct file *file, unsigned int cmd,
                             unsigned long arg)
{
    switch (cmd) {
        case Fake_IOC_RNEC:
            break;
        default:
            return -EINVAL;
    }
    return 0;
}

static int FakeIntGpioFasync(int fd, struct file *file, int mode)
{
    struct FakeIntGpioDev *pdev = (struct FakeIntGpioDev *)file->private_data;

    return fasync_helper(fd, file, mode, &pdev->Nec_queue);
}

