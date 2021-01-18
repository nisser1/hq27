#ifndef _ZTZ245_GPIO_H
#define _ZTZ245_GPIO_H


/* GPIO Reg Addr */
#define IR_GPIO5_6_CONF_BASE  0x1F0000A0 //gpio interrupt
#define IR_GPIO5_7_CONF_BASE  0x1F0000B4 //gpio interrupt
#define IR_GPIO16_CONF_BASE   0x1F0010C8 //gpio interrupt
#define IR_GPIO12_CONF_BASE	  0x1F00106C
#define IR_GPIO5_BASE         0x12145000
#define IR_GPIO12_BASE        0x1214C000
#define IR_GPIO16_BASE        0x12150000
#define IR_GPIO_DATA          0x3FC
#define IR_GPIO_DIR           0x400
#define IR_GPIO_IS            0x404
#define IR_GPIO_IBE           0x408
#define IR_GPIO_IEV           0x40C
#define IR_GPIO_IE            0x410
#define IR_GPIO_RIS           0x414
#define IR_GPIO_MIS           0x418
#define IR_GPIO_IC            0x41C

/* System Gpio Pinnum */
#define Hi3519_GPIO5_6 5*8+6
#define Hi3519_GPIO5_7 5*8+7
#define Hi3519_GPIO16_0 16*8+0
#define Hi3519_GPIO16_1 16*8+1
#define Hi3519_GPIO16_2 16*8+2
#define Hi3519_GPIO16_3 16*8+3
#define Hi3519_GPIO16_4 16*8+4
#define Hi3519_GPIO16_5 16*8+5

/* direction */
#define OUT 1
#define IN 0

/* num pin */
#define GPIO16_0 0
#define GPIO16_1 1
#define GPIO16_2 2
#define GPIO12_2 2
#define GPIO16_3 3
#define GPIO16_4 4
#define GPIO16_5 5
#define GPIO5_6  6
#define GPIO5_7  7

struct ztGpioPrivate {
    volatile unsigned char *IR_GPIO5;
    volatile unsigned char *IR_GPIO16;
    volatile unsigned int  *IR_GPIO5_6_CONF;
    volatile unsigned int  *IR_GPIO5_7_CONF;
    volatile unsigned int  *IR_GPIO16_CONF;
};



int ztInitGpioMap(struct ztGpioPrivate *Private);

void ztReleaseGpioMap(struct ztGpioPrivate *Private);


#endif
