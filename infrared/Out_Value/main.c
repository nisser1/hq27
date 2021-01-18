#include "gpio.h"

struct ztGpioPrivate ztPrivate;
const unsigned int LED_NUM[4] = {
	LED1_NUM,LED2_NUM,LED3_NUM,LED4_NUM
};


void Usage()
{
	printf("./Out_Value_test 0 ---- on\r\n");
	printf("./Out_Value_test 1 ---- off\r\n");
	
}

int main(int argc, char *argv[])
{
    int i = 0;
	int value = 0;
	if (2 != argc) {		Usage();
		return -1;	}
	else {
    memset(&ztPrivate,0,sizeof(struct ztGpioPrivate));
    ztInitGpioMap(&ztPrivate);
	value = atoi(argv[1]);

    for(i = 0; i < 4;i++){
		printf("LED_NUM = %d\r\n",LED_NUM[i]);
        ztGpioExport(LED_NUM[i]);
        ztGpioSetDir(LED_NUM[i],GPIO_OUT);
        if((i == 0) || (i==2))
        {
            ztGpioSetLevel(LED_NUM[i],1);
        }
        if((i == 1) || (i == 3 ))
        {
            ztGpioSetLevel(LED_NUM[i],0);
        }
            
    }


    for(i = 0; i < 4;i++){
        ztGpioUnexport(LED_NUM[i]);
    }
    ztReleaseGpioMap(&ztPrivate);
	}
    return 0;
}
