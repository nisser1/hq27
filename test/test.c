#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h> 
#include <stdlib.h>
typedef struct MESSAGE_CMD{
	int display_mode;
        int display_position;        
}MSG_CMD;
int flag = 0;

int main(int argc,char *argv[])
{
	char send_buf[8] = {0};
	int tmpold,tmp;
	int cmd;
	int fd = 0,ret = 0,i = 0; 
	fd = open("../h27vdec/fifo",O_RDWR);
	if(fd < 0 )
	{
		perror("error open fifo");
		return -1;
	}
	for(i = 0 ; i < 7 ; i++ )
	{
		send_buf[i] = 1;
	}
	

	char buf_tmp[32];
	while(1)
	{
		fgets(buf_tmp,32,stdin);
		//printf("%s",buf_tmp);
		cmd = atoi(buf_tmp);
		send_buf[7] = cmd;
		write(fd,send_buf,8);
		printf("send cmd = %d\n",send_buf[7]);
	}


sleep(3);	
flag = 20;	
#if  0
//测试切换
	while(1)
	{
		if(flag < 10)
		{
				srand((unsigned)time( NULL ));  
				send_buf[7] = rand()%9+100;
		}
		if((flag >= 10) && (flag < 20))
		{
				srand((unsigned)time( NULL ));  
				send_buf[7] = rand()%3+140;
				ret = write(fd,send_buf,8);
				
				
				tmp = rand()%9+190;
				tmpold = rand()%9+190;
				while(tmp != tmpold)
				{
					sleep(2);
					if(tmp-tmpold > 0)
					{
						tmpold++;
					}
					else
					{
						tmpold--;
					}
					send_buf[7] = rand()%4 + 50;
					ret = write(fd,send_buf,8);
				}
		}
		if((flag >= 20) && (flag < 30))
		{
				srand((unsigned)time( NULL ));  
				tmpold = rand()%9+190;
				send_buf[7] = tmpold;
				ret = write(fd,send_buf,8);
				printf("write cmd_buf send_buf[1-7] = %d %d %d %d %d %d %d %d %d\n",flag,send_buf[0],send_buf[1],send_buf[2],send_buf[3],send_buf[4],send_buf[5],send_buf[6],send_buf[7]);
				//ret = write(fd,send_buf,8);
				tmp = rand()%9+190;
				while(tmp != tmpold)
				{
					sleep(2);
					if(tmp-tmpold > 0)
					{
						tmpold++;
					}
					else
					{
						tmpold--;
					}
					send_buf[7] = rand()%4 + 50;
					ret = write(fd,send_buf,8);
					printf("write cmd_buf send_buf[1-7] = %d %d %d %d %d %d %d %d %d\n",flag,send_buf[0],send_buf[1],send_buf[2],send_buf[3],send_buf[4],send_buf[5],send_buf[6],send_buf[7]);
				}				
		}
		if(flag >= 30 )
		{
			flag = 0;
		}
		flag++;
		sleep(3);
		send_buf[7] = 60;
		ret = write(fd,send_buf,8);
		if(ret < 0)
		{
			perror("write cmd error");
		}
		printf("write cmd_buf send_buf[1-7] = %d %d %d %d %d %d %d %d %d\n",flag,send_buf[0],send_buf[1],send_buf[2],send_buf[3],send_buf[4],send_buf[5],send_buf[6],send_buf[7]);	
	}
	return 0;
#endif
	
	
#if 0	
	
	flag = 20;
	while(1)
	{
		if(flag < 10)
		{
				srand((unsigned)time( NULL ));  
				send_buf[7] = rand()%9+100;
		}
		if((flag >= 10) && (flag < 20))
		{
				srand((unsigned)time( NULL ));  
				send_buf[7] = rand()%3+140;
		}
		if((flag >= 20) && (flag < 30))
		{
				srand((unsigned)time( NULL ));  
				send_buf[7] = rand()%9+190;
		}
		if(flag >= 30 )
		{
			flag = 0;
		}
		flag++;
		sleep(5);
		ret = write(fd,send_buf,8);
		if(ret < 0)
		{
			perror("write cmd error");
		}
		printf("write cmd_buf send_buf[1-7] = %d %d %d %d %d %d %d %d %d\n",flag,send_buf[0],send_buf[1],send_buf[2],send_buf[3],send_buf[4],send_buf[5],send_buf[6],send_buf[7]);	
	}
	
	return 0;
#endif
}

