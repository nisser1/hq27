#include <unistd.h>
#include "serial.h"
#include <stdio.h>      /*标准输入输出定义*/  
#include <stdlib.h>     /*标准函数库定义*/ 
#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include  <fcntl.h> 	
#include "linkqueue.h"
#include "service.h"
#include "serial_send_msg.h"
#include "serial_recv_msg.h"
#include "msg_netc.h"



int main(int argc,char *argv[])
{
	pthread_t tid[3]={-1};
	int ret = -1;
	THREAD_ARG *thread_arg;
	
	int udp_sockfd;
	struct sockaddr_in serveraddr, clientaddr;
	socklen_t addrlen = sizeof(serveraddr);
	

	//初始化结构体 memset bzero
	bzero(&serveraddr, addrlen);
	bzero(&clientaddr, addrlen);


	//第一步：创建套接字
	if((udp_sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		printf("fail to socket");
	}
	//第二步：填充服务器网络信息结构体
	//inet_addr：将点分十进制ip地址转化为网络字节序的整型数据
	//htons：将主机字节序转化为网络字节序
	//atoi：将数字型字符串转化为整型数据
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = inet_addr("192.168.10.111");
	serveraddr.sin_port = htons(12666);
	//第三步：将套接字与服务器网络信息结构体绑定
	if(bind(udp_sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0)
	{
		printf("fail to bind\n");
	}


	
	
	
	//初始化
	thread_arg = (THREAD_ARG *)malloc(sizeof(THREAD_ARG));
	memset(thread_arg,0,sizeof(THREAD_ARG));
	ret = pthread_cond_init(&(thread_arg->cond_serial),NULL);
	ret = pthread_cond_init(&(thread_arg->cond2_iic),NULL);
	ret = pthread_mutex_init(&(thread_arg->mutex),NULL);
	//打开串口
	//thread_arg->link_queue 	= linkqueue_create();
	thread_arg->serial_fd 	= serial_open(1, 115200);			//open serial port 0, /etc/ttyS0  
	thread_arg->udp_sockfd 	= udp_sockfd;
	printf("serial_fd = %d udp_sock_fd = %d\n",thread_arg->serial_fd,udp_sockfd);
	
	
	
	
	
	/*开启线程*/
	if(thread_arg->serial_fd > 0) 
	{
        serial_set_attr(thread_arg->serial_fd, 8, PARITY_NONE, 1, FLOW_CONTROL_NONE);
	}
	ret = pthread_create(&(tid[0]),NULL,(void *)serial_send_msg,(void *)thread_arg);
	if(ret != 0)
		printf("pthread_create serial send cmd thread ret = %d\n",ret);
	
	pthread_create(&tid[1] , NULL , (void *)serial_recv_msg , (void *)thread_arg);
	if(ret != 0)
		printf("pthread_create serial recv msg thread ret = %d\n",ret);
	
	pthread_create(&tid[2],NULL,(void *)net_msg,(void *)thread_arg);
	if(ret != 0)
		printf("pthread_create net msg thread ret = %d\n",ret);
	
	
	
	
	/*回收线程*/
	ret = pthread_join(tid[0],NULL);
	if(ret != 0)
		printf("pthread_join serial send cmd thread ret = %d",ret);

	ret = pthread_join(tid[1],NULL);
	if(ret != 0)
		printf("pthread_join serial recv msg thread ret = %d",ret);
	
	ret = pthread_join(tid[2],NULL);
	if(ret != 0)
		printf("pthread_join net msg thread ret = %d",ret);
	
	
	return 0;
}
