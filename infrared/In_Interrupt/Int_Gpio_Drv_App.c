#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>


/* 键值：按下时，0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07 */
/* 键值：松开时，0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87 */
int fd = -1;
struct pin_desc {
	unsigned int key_pin;
	unsigned char key_fall_val;
	unsigned char key_raise_val;
};
struct pin_desc pins_desc[8] = {
    {0x0,0x00,0x80},
    {0x1,0x01,0x81},
    {0x2,0x02,0x82},
    {0x3,0x03,0x83},
    {0x4,0x04,0x84},
    {0x5,0x05,0x85},
    {0x6,0x06,0x86},
    {0x7,0x07,0x87},
};

static int Slot_Key_Map(unsigned char key_val)
{
	int i = 0;
	int index = 0;
	for(i = 0; i < 8; i++) {
		if(pins_desc[i].key_fall_val == key_val) return index;	
		index++;	
	}
	for(i = 0; i < 8; i++) {
		if(pins_desc[i].key_raise_val == key_val) return index;
		index++;
	}
	return index;
}

static void signalio_handler(int signum)
{
	unsigned char key_val;
	int index = 0;
    printf("dddddddd\n");
    /*
	read(fd,&key_val,1);
	index = Slot_Key_Map(key_val);
	if( index >= 8 ) {
	printf("key%d ----> 弹起: 0x%x\r\n",pins_desc[index-8].key_pin,key_val);
	} else {
	printf("key%d ----> 按下: 0x%x\r\n",pins_desc[index].key_pin,key_val);
	}*/

	//printf("key_val: 0x%x\r\n",key_val);
}

int main(int argc, char *argv[])
{
	int  oflags;
	fd = open("/dev/fakeChenIntGpio_dev", O_RDWR, S_IRUSR | S_IWUSR);
	if (fd != -1) {
		signal(SIGUSR2, signalio_handler);
		fcntl(fd, F_SETOWN, getpid());
		oflags = fcntl(fd, F_GETFL);
		fcntl(fd, F_SETFL, oflags | FASYNC);
		fcntl(fd, 10, SIGUSR2);
		while (1) {
			sleep(100);
		}
	} else {
		printf("device open failure\n");
	}
 	return 0;
}
