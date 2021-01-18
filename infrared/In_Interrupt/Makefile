KERNDIR =/opt/hi3559-kernel/linux-4.9.y_multi-core
PWD := $(shell pwd)
obj-m +=Int_Gpio_Drv.o

all:
	make -C  $(KERNDIR) M=$(PWD) modules CROSS_COMPILE=aarch64-himix100-linux- ARCH=arm64
	aarch64-himix100-linux-gcc -o Int_Gpio_Drv_App Int_Gpio_Drv_App.c
clean:
	make -C $(KERNDIR) M=$(PWD) CROSS_COMPILE=aarch64-himix100-linux- ARCH=arm64 clean
	rm Int_Gpio_Drv_App
