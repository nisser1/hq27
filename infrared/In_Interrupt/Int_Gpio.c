#include "Int_Gpio.h"

/*
 * 初始化GPIO，将GPIO复用模式关掉，配置成输入中断模式
 */
int ztInitGpioMap(struct ztGpioPrivate *Private)
{
    // 复用控制寄存器基址
    Private->IR_GPIO5_6_CONF =
        (volatile unsigned int *)ioremap(IR_GPIO5_6_CONF_BASE, 4);
    Private->IR_GPIO5_7_CONF =
        (volatile unsigned int *)ioremap(IR_GPIO5_7_CONF_BASE, 4);
    Private->IR_GPIO16_CONF =
        (volatile unsigned int *)ioremap(IR_GPIO16_CONF_BASE, 4 * 6);

    // GPIO 寄存器基址
    Private->IR_GPIO5 = (volatile unsigned char *)ioremap(IR_GPIO5_BASE, 0xFFF);
    Private->IR_GPIO16 =
        (volatile unsigned char *)ioremap(IR_GPIO16_BASE, 0xFFF);

    if (NULL == Private->IR_GPIO5 ||
        NULL == Private->IR_GPIO16 || NULL == Private->IR_GPIO5_6_CONF ||
        NULL == Private->IR_GPIO5_7_CONF || NULL == Private->IR_GPIO16_CONF) {
        printk("NULL pointer!\n");
        return -1;
    }

    /* ① GPIO5_6 GPIO5_7 GPIO16_0~6配置成GPIO输入模式 */
    // 1.复用为GPIO
    // 启动强上拉和上拉使能
    Private->IR_GPIO5_6_CONF[0] = 0x1400;
    Private->IR_GPIO5_7_CONF[0] = 0x1400;
    Private->IR_GPIO16_CONF[0] = 0x1400;
    Private->IR_GPIO16_CONF[1] = 0x1400;
    Private->IR_GPIO16_CONF[2] = 0x1400;
    Private->IR_GPIO16_CONF[3] = 0x1400;
    Private->IR_GPIO16_CONF[4] = 0x1400;
    Private->IR_GPIO16_CONF[5] = 0x1400;

    // 2.配置成GPIO输入模式
    Private->IR_GPIO5[IR_GPIO_DIR] &= ~(0x1 << GPIO5_6 || 0x1 << GPIO5_7);
    Private->IR_GPIO16[IR_GPIO_DIR] &=
        ~(0x1 << GPIO16_0 || 0x1 << GPIO16_1 || 0x1 << GPIO16_2 ||
          0x1 << GPIO16_3 || 0x1 << GPIO16_4 || 0x1 << GPIO16_5);

    /* GPIO12_2配置成中断模式 */
    // 1.配成管脚触发电平方式
    Private->IR_GPIO5[IR_GPIO_IS] &= ~(0x1 << GPIO5_6 || 0x1 << GPIO5_7);
    Private->IR_GPIO16[IR_GPIO_IS] &=
        ~(0x1 << GPIO16_0 || 0x1 << GPIO16_1 || 0x1 << GPIO16_2 ||
          0x1 << GPIO16_3 || 0x1 << GPIO16_4 || 0x1 << GPIO16_5);

    // 3.配置成双边沿触发
    Private->IR_GPIO5[IR_GPIO_IBE] |= (0x1 << GPIO5_6 || 0x1 << GPIO5_7);
    Private->IR_GPIO16[IR_GPIO_IBE] |=
         (0x1 << GPIO16_0 || 0x1 << GPIO16_1 || 0x1 << GPIO16_2 ||
          0x1 << GPIO16_3 || 0x1 << GPIO16_4 || 0x1 << GPIO16_5);
#if 0
    // 3.配置成单边沿触发
    Private->IR_GPIO5[IR_GPIO_IBE] &= ~(0x1 << GPIO5_6 || 0x1 << GPIO5_7);
    Private->IR_GPIO16[IR_GPIO_IBE] &=
        ~(0x1 << GPIO16_0 || 0x1 << GPIO16_1 || 0x1 << GPIO16_2 ||
          0x1 << GPIO16_3 || 0x1 << GPIO16_4 || 0x1 << GPIO16_5);
#endif

    // 4.配置成下降沿触发 需要3.设置成单边沿触发才有作用
    Private->IR_GPIO5[IR_GPIO_IEV] &= ~(0x1 << GPIO5_6 || 0x1 << GPIO5_7);
    Private->IR_GPIO16[IR_GPIO_IEV] &=
        ~(0x1 << GPIO16_0 || 0x1 << GPIO16_1 || 0x1 << GPIO16_2 ||
          0x1 << GPIO16_3 || 0x1 << GPIO16_4 || 0x1 << GPIO16_5);

    // 5.清除中断
    Private->IR_GPIO5[IR_GPIO_IC] = 0xff;
    Private->IR_GPIO16[IR_GPIO_IC] = 0xff;

    // 6.使能中断
    Private->IR_GPIO5[IR_GPIO_IE] |= (0x1 << GPIO5_6 || 0x1 << GPIO5_7);
    Private->IR_GPIO16[IR_GPIO_IE] |=
        (0x1 << GPIO16_0 || 0x1 << GPIO16_1 || 0x1 << GPIO16_2 ||
         0x1 << GPIO16_3 || 0x1 << GPIO16_4 || 0x1 << GPIO16_5);
    return 0;
}

void ztReleaseGpioMap(struct ztGpioPrivate *Private)
{
    iounmap(Private->IR_GPIO5);
    iounmap(Private->IR_GPIO16);
    iounmap(Private->IR_GPIO5_6_CONF);
    iounmap(Private->IR_GPIO5_7_CONF);
    iounmap(Private->IR_GPIO16_CONF);
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

