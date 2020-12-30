#ifndef _ZTZ245_GPIO_H
#define _ZTZ245_GPIO_H

/* GPIO Reg Addr */
#define IR_GPIO12_CONF_BASE	  0x1F00106C
#define IR_GPIO12_BASE        0x1214C000
#define IR_GPIO_DATA          0x3FC
#define IR_GPIO_DIR           0x400
#define IR_GPIO_IS            0x404
#define IR_GPIO_IBE           0x408
#define IR_GPIO_IEV           0x40C
#define IR_GPIO_IE            0x410
#define IR_GPIO_RIS           0x414
#define IR_GPIO_MIS           0x418
#define IR_GPIO_IC            0x41C

/* direction */
#define OUT 1
#define IN 0

/* num pin */
#define GPIO12_2 2


struct ztGpioPrivate {
    volatile unsigned char *IR_GPIO;
    volatile unsigned int  *IR_GPIO_CONF;
};



int ztInitGpioMap(struct ztGpioPrivate *Private);

void ztReleaseGpioMap(struct ztGpioPrivate *Private);


#endif
