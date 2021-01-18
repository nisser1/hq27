#include<stdio.h>
#include<stdlib.h>
#include "msg_handle.h"
#include <string.h>


void check_net_message(char *cmd_word,THREAD_ARG *thread_arg)
{
	if((cmd_word == NULL) && (thread_arg == NULL))
		return ;
	if((cmd_word[0] == 'w') && (cmd_word[1] == 'l'))
		thread_arg->serial_cmd = 0;
	if((cmd_word[0] == 'r') && (cmd_word[1] == 'l'))
		thread_arg->serial_cmd = 1;
	if((cmd_word[0] == 'w') && (cmd_word[1] == 'c'))
		thread_arg->serial_cmd = 2;
	if((cmd_word[0] == 'w') && (cmd_word[1] == 'c'))
		thread_arg->serial_cmd = 3;
	if((cmd_word[0] == 'b') && (cmd_word[1] == 't'))
		thread_arg->serial_cmd = 4;
	if((cmd_word[0] == 't') && (cmd_word[1] == '1'))
		thread_arg->serial_cmd = 5;
	if((cmd_word[0] == 't') && (cmd_word[1] == '2'))
		thread_arg->serial_cmd = 6;
	if((cmd_word[0] == 'b') && (cmd_word[1] == '5'))
		thread_arg->serial_cmd = 7;
	if((cmd_word[0] == 'b') && (cmd_word[1] == '2'))
		thread_arg->serial_cmd = 8;
	if((cmd_word[0] == 'b') && (cmd_word[1] == '3'))
		thread_arg->serial_cmd = 9;
	if((cmd_word[0] == 'b') && (cmd_word[1] == '1'))
		thread_arg->serial_cmd = 10;
	if((cmd_word[0] == 'b') && (cmd_word[1] == 's'))
		thread_arg->serial_cmd = 11;
	return ;
}

void* net_msg(void *arg)
{
	int bytes;
	struct sockaddr_in clientaddress;
	THREAD_ARG *thread_arg = (THREAD_ARG *)arg;
	char recvbuf[NET_MESSAGE_LEN];
	int i=0,j=0;
	char cmd_word[4];
	socklen_t addrlen = (socklen_t)sizeof(clientaddress);
	
	
	while(1)
	{
		//这里如何区分是FT2000还是其他网络设备 		取决与上位机程序
		/*逻辑
				记录下来源的地址   如果client地址有重复引用计数+1		回复后引用计数-1 
		*/
		if((bytes = recvfrom(thread_arg->udp_sockfd, recvbuf, NET_MESSAGE_LEN, 0, (struct sockaddr *)&clientaddress,&addrlen)) < 0)
		{			
			printf("net recv message error !!!\n");
		}
		else
		{
			//网络命令转串口命令
			
			//支持多个客户端的以后改进
			//strcpy(thread_arg->ipaddr,inet_ntoa(clientaddr.sin_addr));
			memcpy(&(thread_arg->clientaddr),&clientaddress,sizeof(struct sockaddr_in));
			/*
					wl		WRITE_LIGHT,
					rl      READ_LIGHT,
					wc      WRITE_CONTRAST,
					rc      READ_CONTRAST,	
					bt      BOARD_TEMP,
					t1      TEMP_SENSOR1,
					t2      TEMP_SENSOR2,
					b5      BOARD_5V,
					b2      BOARD_12V,
					b3      BOARD_3_3,
					b1      BOARD_1_2,
					bs      GET_STATUS
			*/
			while(recvbuf[i] != '\0')
			{
				
				j++;
				i++;
				cmd_word[j] = recvbuf[i]; 
				if(j == 1)
				{
					check_net_message(cmd_word,thread_arg);
					pthread_mutex_lock(&(thread_arg->mutex));
					pthread_cond_signal(&(thread_arg->cond_serial));
					pthread_mutex_unlock(&(thread_arg->mutex));
					j = 0;
					usleep(100);
				}
					
			}
			
			//网路命令转iic命令
				
		}
	}
}

