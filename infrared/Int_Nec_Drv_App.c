#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#define Fake_MAGIC 'g'
#define Fake_IOC_RENC _IOR(Fake_MAGIC,0,int)

unsigned char Key_Value[12] = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B};
unsigned char Key_Opp_Value[12] = {0xff,0xfe,0xfd,0xfc,0xfb,0xfa,0xf9,0xf8,0xf7,0xf6,0xf5,0xf4};
char * Key_Str_Name[12] = {"多屏 ","四屏 ","上 ","左 ","确定 ","右 ","下 ","测试 ","返回 ",
"本车 ","战车 ","摄像头 "};

int fd = -1;
unsigned char NecPrivate[4] = {0};

int KeyValueSlot(unsigned char value)
{
   int i = 0;
   int index = 0;
   for(i = 0; i < 12; i++) {
	if(value == Key_Value[i])
	{
		index = i;
		break;
	}
   }
   if( index >= 0 || index <= 11) return index;
   else return 0xff;
}

int KeyOppSlot(unsigned char value)
{
   int i = 0;
   int index = 0;
   for(i = 0; i < 12; i++) {
	if(value == Key_Opp_Value[i])
	{
		index = i;
		break;
	}
   }
   if( index >= 0 || index <= 11) return index;
   else return 0xff;
}

static void signalio_handler(int signum)
{
   int i = 0;
   int key_index_value = 0;
   int key_index_opp = 0;
   ioctl(fd,Fake_IOC_RENC,NecPrivate);
   key_index_value = KeyValueSlot(NecPrivate[2]);
   key_index_opp   = KeyOppSlot(NecPrivate[3]);
   if(NecPrivate[0] == 0xff && NecPrivate[1] == 0xff && key_index_value == key_index_opp) {
	printf("%s -------> 键码值[%02x]\r\n",Key_Str_Name[key_index_value],Key_Value[key_index_value]);
   }
}

int main(int argc, char *argv[])
{
	int  oflags;
	fd = open("/dev/FakeIntGpio_dev", O_RDWR, S_IRUSR | S_IWUSR);
	if (fd != -1) {
		signal(SIGIO, signalio_handler);
		fcntl(fd, F_SETOWN, getpid());
		oflags = fcntl(fd, F_GETFL);
		fcntl(fd, F_SETFL, oflags | FASYNC);
		while (1) {
			sleep(100);
		}
	} else {
		printf("device open failure\n");
	}
 	return 0;
}
