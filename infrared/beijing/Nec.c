#include "Nec.h"

/* 
 * 初始化GPIO，讲GPIO复用模式关掉，配置成输入中断模式 
 */
int ztInitGpioMap(struct ztGpioPrivate *Private)
{
    // 复用控制寄存器基址 0x1F00106C
    Private->IR_GPIO_CONF = (volatile unsigned int *)ioremap(IR_GPIO12_CONF_BASE, 4);

	// GPIO 寄存器基址         0x1214C000
    Private->IR_GPIO = (volatile unsigned char *)ioremap(IR_GPIO12_BASE, 0xFFF);
   
    if (NULL == Private->IR_GPIO_CONF || NULL == Private->IR_GPIO ) {
        printk("NULL pointer!\n");
        return -1;
    }

   
    /* ① GPIO12_2配置成GPIO输入模式 */
	//1.复用为GPIO
	printk("Private->IR_GPIO_CONF[0] = 0x%x\r\n",Private->IR_GPIO_CONF[0]);
	Private->IR_GPIO_CONF[0] = 0x14f0;
	printk("Private->IR_GPIO_CONF[0] = 0x%x\r\n",Private->IR_GPIO_CONF[0]);
	//2.配置成GPIO输入模式
	Private->IR_GPIO[IR_GPIO_DIR] &= ~(0x1 << GPIO12_2);

	/* GPIO12_2配置成中断模式 */
	//1.配成管脚触发电平方式
	Private->IR_GPIO[IR_GPIO_IS]  &= ~(0x1 << GPIO12_2);

	//3.配置成单边沿触发
	Private->IR_GPIO[IR_GPIO_IBE] &= ~(0x1 << GPIO12_2);

	//4.配置成下降沿触发
	Private->IR_GPIO[IR_GPIO_IEV] &= ~(0x1 << GPIO12_2);

	//5.清除中断
	Private->IR_GPIO[IR_GPIO_IC]  = 0xff;

	//6.使能中断
	Private->IR_GPIO[IR_GPIO_IE] |= (0x1 << GPIO12_2);
	

    return 0;
}

void ztReleaseGpioMap(struct ztGpioPrivate *Private)
{
    iounmap(Private->IR_GPIO_CONF);
    iounmap(Private->IR_GPIO);
}

/*FT GPIO一共分为0 1 两组，配组又分为portA 和 portB */
/*第0组GPIO的操作函数*/
unsigned char ztSetGpioDir(unsigned char chan, unsigned char mode,
                            volatile unsigned char *IR_GPIO)
{
    
        if (1 == mode) {
            IR_GPIO[IR_GPIO_DIR] |= 1 << chan;
        }
        else if (0 == mode) {
            IR_GPIO[IR_GPIO_DIR] &= ~(1 << chan);
        }
    
   		return 0;
}

unsigned char ztSetGpioValue(unsigned char chan, unsigned char value,
                              volatile unsigned char *IR_GPIO)
{
   
        if (1 == value) {
            IR_GPIO[IR_GPIO_DATA] |= 1 << chan;
        }
        else if (0 == value) {
            IR_GPIO[IR_GPIO_DATA] &= ~(1 << chan);
        }
  
    	return 0;
}

unsigned char ztGetGpioValue(unsigned char chan,
                              volatile unsigned char *IR_GPIO)
{
        return ((IR_GPIO[IR_GPIO_DATA] & (1 << chan)) >> chan);
  
}


