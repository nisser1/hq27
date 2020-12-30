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
#include <linux/ktime.h>
#include "./Nec.c"

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

/* GPIO12_2 interupt   */
static int Gpio_Int_Count = 0; // 中断计数
module_param(Gpio_Int_Count, int, S_IRUGO);

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
	struct timer_list fake_timer;		  /* 设备要使用的定时器 */
	unsigned char NecPrivate[4];		  /* 红外私有数据 */ 
	struct fasync_struct *Nec_queue;	  /* 异步通知 */
 
};

/* ① 确定主设备号，也可以让内核自动分配 */
/* 创建Hello设备结构体 */
struct FakeIntGpioDev FakeIntGpio;
/* 红外私有数据解析 */
int necflag = 0;               // 表示数据帧的开始
int necnum = 0;                // 表示数据帧里的第几位数据
static long long prev = 0;  // 64bit,记录上次的时间
volatile unsigned int times[40];     // 记录每位数据的时间	 


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
static int FakeIntGpioFasync(int fd, struct file*filp, int mode);

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

/* 红外NEC除引导码外后面的数据解析 */
void ztDateProcess(void)
{
	int i, j, tmp;
	
	for(i = 0; i < 4; i++){ // 一共4个字节
		   tmp = 0;
		   for(j = 0; j < 8; j++){
			   if(times[i * 8 + j] > 2250 && times[i * 8 + j] < 2488) // 如果数据位的信号周期大于2.25ms,则是二进制数据1
				   tmp |= 1 << j;
		   }
		 FakeIntGpio.NecPrivate[i] = tmp; 
		 //printk("%02x ",FakeIntGpio.NecPrivate[i]);
	   }
	 	 //printk("\r\n");
	 	if(FakeIntGpio.Nec_queue && (FakeIntGpio.NecPrivate[0] == 0xff) && (FakeIntGpio.NecPrivate[0] == FakeIntGpio.NecPrivate[1]) && ((unsigned char)~FakeIntGpio.NecPrivate[2] == FakeIntGpio.NecPrivate[3])) {
	 	kill_fasync(&FakeIntGpio.Nec_queue, SIGIO, POLL_IN);
	 	}
}


static irqreturn_t FakeChenIntIsr(int irq, void *dev_id)
{
	struct timeval time;
	long long now = 0;
    unsigned int offset;
	disable_irq_nosync(irq);
	
	do_gettimeofday(&time); // 获取时间来解码
	now = (long)time.tv_usec;

    if(!necflag){  // 数据开始
        necflag = 1;
        prev = now;

		goto GOUT;
    }
	
    offset = now - prev;
    prev = now;
    if((offset > 12800) && (offset < 14000)){   // 判断是否收到引导码  引导码9+4.5ms ---> 13.5ms
        necnum = 0;
        goto GOUT;
    }
 
    if(necnum < 32)
        times[necnum++] = offset;  	// 引导码后面接32个数据 用户码L + 用户码H + 键数据码 + 键数据码反码
 
    if(necnum >= 32){ 
        ztDateProcess();			// 解析数据
        necflag = 0;
	 	necnum = 0;
		//memset((void *)times,0,sizeof(times));
    }

GOUT:
	enable_irq(irq);
    return IRQ_HANDLED;


}


/* 定时器处理函数 */
static void fake_do_timer(unsigned long arg)
{
	print("irq_count = %d num = %d.\r\n",Gpio_Int_Count,necnum);
	Gpio_Int_Count = 0;
	FakeIntGpio.fake_timer.expires = jiffies + 500;
	add_timer(&FakeIntGpio.fake_timer); //添加注册定时器

}

static int __init FakeIntGpioInit(void)
{
    int ret = 0;
    memset(&FakeIntGpio, 0, sizeof(struct FakeIntGpioDev));
	
    FakeIntGpio.major = 0;  // 主设备号写0让内核自动分配

        /* 1.获取设备号 */
        /* 主设备号不确定，写为0->申请让系统自动分配 */
        if (!FakeIntGpio.major) {
            FakeIntGpio.devid = MKDEV(FakeIntGpio.major, 0);
            // Fake_Int_DEV_NUM 1
            ret = alloc_chrdev_region(
                &FakeIntGpio.devid, 0, Fake_Int_DEV_NUM,
                "FakeIntGpio_driver"); /* (major,0) 对应 FakeIntGpio_driver,
                                        (major,
                                        1~255)都不对应FakeIntGpio_driver */
                                       /* 获取主设备号 */
            FakeIntGpio.major = MAJOR(FakeIntGpio.devid);
            /* 获取次设备号 */
            FakeIntGpio.minor = MINOR(FakeIntGpio.devid);
            print(
                "FakeIntGpio.devid = %d FakeIntGpio.major = %d  "
                "FakeIntGpio.minor = "
                "%d.	\r\n",
                FakeIntGpio.devid, FakeIntGpio.major, FakeIntGpio.minor);
        }
  
        /* 2.初始化cdev,添加驱动 */
        FakeIntGpio.cdev.owner = THIS_MODULE;
        cdev_init(
            &FakeIntGpio.cdev,
            &Fake_Int_ops); /* ls /proc/devices  ----->244 FakeIntGpio_driver */

        /* 3.添加cdev 注册设备以及驱动 */
        ret = cdev_add(&FakeIntGpio.cdev, FakeIntGpio.devid, Fake_Int_DEV_NUM);
    

    /* 4.创建类 */
    FakeIntGpio.class = class_create(
        THIS_MODULE, "FakeIntGpio_class"); /* /sys/class/FakeIntGpio_class */

    /* 5.创建设备 */
    FakeIntGpio.device =
        device_create(FakeIntGpio.class, NULL, FakeIntGpio.devid, NULL,
                      "FakeIntGpio_dev"); /* /dev/FakeIntGpio_dev */

    /* 6.初始化Int_Gpio     GPIO12_2输入中断 */
    ztInitGpioMap(&FakeIntGpio.FakeChenGI);


    /* 7.注册一个Gpio_Int 中断 */
    FakeIntGpio.interrupt_gpio1 = 98;  // GPIO12_2 -> 98
    FakeIntGpio.interrupt_pin1 = gpio_to_irq(FakeIntGpio.interrupt_gpio1);
    print(" irq:%d-%d\n", FakeIntGpio.interrupt_gpio1,FakeIntGpio.interrupt_pin1);
    ret = request_irq(FakeIntGpio.interrupt_pin1, FakeChenIntIsr,
                IRQF_TRIGGER_FALLING, "Fake_Gpio_Int", NULL);

    /* 8.初始化互斥锁 */
    mutex_init(&FakeIntGpio.lock);

	/* 9.初始化定时器 */
	init_timer(&FakeIntGpio.fake_timer);
	FakeIntGpio.fake_timer.function = fake_do_timer;
	FakeIntGpio.fake_timer.expires = jiffies + 500;
	//add_timer(&FakeIntGpio.fake_timer); //添加注册定时器
	

    return 0;
}

/* ⑥ 有入口函数就应该有出口函数：卸载驱动程序时，出口函数调用释放函数 */
static void __exit FakeIntGpioExit(void)
{
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
		free_irq(FakeIntGpio.interrupt_pin1, NULL);

		/* 删除定时器 */
		del_timer(&FakeIntGpio.fake_timer);

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
	FakeIntGpioFasync(-1,file,0);
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
   // struct FakeIntGpioDev *pdev = (struct FakeIntGpioDev *)file->private_data;

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

void NecToApp(unsigned char * nec)
{
	memcpy(nec,FakeIntGpio.NecPrivate,sizeof(FakeIntGpio.NecPrivate));
	
}

static long FakeIntGpioIoctl(struct file *file, unsigned int cmd,
                             unsigned long arg)
{
    switch (cmd) {
        case Fake_IOC_RNEC:
   				NecToApp((unsigned char*)arg);
   				break;
        default:
            return -EINVAL;
    }
    return 0;
}


static int FakeIntGpioFasync(int fd, struct file*file, int mode)
{
	struct FakeIntGpioDev *pdev = (struct FakeIntGpioDev *)file->private_data;

	return fasync_helper(fd,file,mode,&pdev->Nec_queue);

}


