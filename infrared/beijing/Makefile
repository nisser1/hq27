KERNDIR =/work/fakeChen/kernel/linux-4.9.y_multi-core 
PWD := $(shell pwd)
obj-m +=Nec_Drv.o

all:
	make -C  $(KERNDIR) M=$(PWD) modules CROSS_COMPILE=aarch64-himix100-linux- ARCH=arm64
	aarch64-himix100-linux-gcc -o Nec_Drv_App Nec_Drv_App.c
clean:
	make -C $(KERNDIR) M=$(PWD) CROSS_COMPILE=aarch64-himix100-linux- ARCH=arm64 clean
	rm Nec_Drv_App
