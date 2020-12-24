

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <math.h>
#include <unistd.h>
#include <signal.h>
#include <sys/prctl.h>
#include "fifo_buffer.h"

pthread_mutex_t mutex;
pthread_cond_t cond ;
/*
	参数	无
	返回值 	成功返回0 失败返回-1
*/
int create_fifo(char *path)
{
	//fifo创建	
	if(mkfifo(path,O_RDWR) < 0)
	{
		if(errno == EEXIST)
		{
			return 0 ;
		}
		printf("failed mkfifo create!! %s    %s    %d\n",__FILE__, __func__, __LINE__);
		return -1;
	}
	return 0;
}
/*
参数	无
返回值	失败return -1 成功返回文件描述符 
*/
int open_fifo(int *fd)
{
	//打开fifo
	fd[0] = open("fifo",O_RDWR);
	if((fd[0]) < 0)
	{
		printf("fd = %d\n",*fd);
		printf("failed open fifo!! %s    %s    %d\n",__FILE__, __func__, __LINE__);
		return -1;
	}
	return 0;
}
int read_fifo(unsigned char *recv_from_live555_data, int fd, int read_size)
{
	int read_len;
	if((read_len = read(fd,recv_from_live555_data,read_size)) < 0)
	{
		return -1;
	}
	return read_len;
}
int write_fifo(u_int8_t *send_to_vdec_data,int fd,int write_size)
{
	int write_len = 0;
	printf("write fifo before\n");
	if((write_len = write(fd, send_to_vdec_data,write_size))< 0)
	 {
		 printf("write fifo fffs\n");
		 return -1;
	 }
	 printf("write fifo end  = %d\n",write_len);
	 return 0;
}



int indexSubtract(int a,int b)
{
	if(a>=b)
	{
		return a-b;
	}else
	{
		return a+BUF_SIZE -b;
	}
}
int indexAdd(int a,int b)
{
	if((a+b) >= BUF_SIZE)
	{
		return a+b-BUF_SIZE;
	}
	else
	{
		return a+b;
	}
}
int buffDataNum(int rindex,int windex)
{
	return indexSubtract(windex,rindex);//如果w小于r  那么读到的是
}
int buffFreeSpace(int rindex,int windex)
{
	return indexSubtract(BUF_SIZE-1,buffDataNum(rindex,windex));
}
int buffFull(int rindex,int windex)
{
	if(indexAdd(windex,1) == rindex)
	{
		return 1;
	}
	return 0;
}
int buffNull(int rindex,int windex)
{
	if(windex = rindex)
	{
		return  1;					//队列
	}
	return 0;
}


/*int readDataToBuff(int nread, unsigned char * dest_buff, unsigned char *src_buf,int rindex,int windex,MESSAGE *msg,int dataindex)
{
	while(nread>0)
	{
		int readbuffFree=0;
		readbuffFree = buffFreeSpace(rindex, windex);

		//剩余和连续空间选小的	
		readbuffFree = (readbuffFree <= indexSubtract(BUF_SIZE,windex))?readbuffFree : indexSubtract(BUF_SIZE,windex);
		//剩余空间与可以读取的个数选择小的	
		readbuffFree = (readbuffFree <= nread) ? readbuffFree : nread;
		if(readbuffFree)
		{
			memcpy(&dest_buff[windex],src_buf,readbuffFree);
			windex = indexAdd(windex,readbuffFree);
			//printf("windex = %d\n",windex);
			msg->windex[dataindex] = windex;
			//msg->rindex[dataindex] = rindex;			
		}
		else
		{
			break;
		}
		nread -=readbuffFree;
	}
	return 0;
}*/

int readDataToBuff(int nread, unsigned char * dest_buff, unsigned char *src_buf,int rindex,int windex,MESSAGE *msg,int dataindex)
{
    unsigned char start_code[4] = {0x00,0x00,0x00,0x01};
    memcpy(&dest_buff[windex],start_code,4);
    windex = indexAdd(windex,4);
    memcpy(&dest_buff[windex],src_buf,nread);
    windex = indexAdd(windex,nread);
    msg->windex[dataindex] = windex;
	return 0;
}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
