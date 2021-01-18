#include "serial.h"
#include <stdio.h>      /*标准输入输出定义*/  
#include <stdlib.h>     /*标准函数库定义*/ 
#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include  <fcntl.h> 

#include "service.h"
#include "msg_handle.h"
#include "serial_recv_msg.h"
#include "linkqueue.h"
//发送串口命令线程
/*
	内部使用
*/
void check_datavalidity_timeout(THREAD_ARG *thread_arg)
{
	struct timeval begin;
	gettimeofday(&begin,NULL);
	long long endTime = (long long)begin.tv_sec * 1000 + (long long)begin.tv_usec / 1000;
	printf("endTime is:%lld\n",endTime);
	if((thread_arg->cmd_answer_falg.write_light_result == 1) && ((endTime - thread_arg->cmd_answer_falg.time_write_light_result) >= 1000))
	{
		//数据无效
		thread_arg->cmd_exec_timeout[WRITE_LIGHT] = 1;
	}
	if((thread_arg->cmd_answer_falg.read_light_result == 1) && ((endTime - thread_arg->cmd_answer_falg.time_read_light_result) >= 1000))
	{
		//数据无效//超时
		thread_arg->cmd_exec_timeout[READ_LIGHT] = 1;
	}
	if((thread_arg->cmd_answer_falg.write_contrast_result == 1) && ((endTime - thread_arg->cmd_answer_falg.time_write_contrast_result) >= 1000))
	{
		//数据无效//超时
		thread_arg->cmd_exec_timeout[WRITE_CONTRAST] = 1;
	}
	if((thread_arg->cmd_answer_falg.read_contrast_result == 1) && ((endTime - thread_arg->cmd_answer_falg.time_read_contrast_result) >= 1000))
	{
		//数据无效//超时
		thread_arg->cmd_exec_timeout[READ_CONTRAST] = 1;
	}
	if((thread_arg->cmd_answer_falg.answer_get_bit_board_temp == 1) && ((endTime - thread_arg->cmd_answer_falg.time_answer_get_bit_board_temp) >= 1000))
	{
		//数据无效//超时
		thread_arg->cmd_exec_timeout[BOARD_TEMP] = 1;
	}
	if((thread_arg->cmd_answer_falg.answer_get_bit_sensor1_temp == 1) && ((endTime - thread_arg->cmd_answer_falg.time_answer_get_bit_board_temp) >= 1000))
	{
		//数据无效//超时
		thread_arg->cmd_exec_timeout[TEMP_SENSOR1] = 1;
	}
	if((thread_arg->cmd_answer_falg.answer_get_bit_sensor2_temp == 1) && ((endTime - thread_arg->cmd_answer_falg.time_answer_get_bit_sensor2_temp) >= 1000))
	{
		//数据无效//超时
		thread_arg->cmd_exec_timeout[TEMP_SENSOR2] = 1;
	}
	if((thread_arg->cmd_answer_falg.answer_get_bit_5 == 1) && ((endTime - thread_arg->cmd_answer_falg.time_answer_get_bit_5) >= 1000))
	{
		//数据无效//超时
		thread_arg->cmd_exec_timeout[BOARD_5V] = 1;
	}
	if((thread_arg->cmd_answer_falg.answer_get_bit_12 == 1) && ((endTime - thread_arg->cmd_answer_falg.time_answer_get_bit_12) >= 1000))
	{
		//数据无效 //超时
		thread_arg->cmd_exec_timeout[BOARD_12V] = 1;
	}
	if((thread_arg->cmd_answer_falg.answer_get_bit_3_3 == 1) && ((endTime - thread_arg->cmd_answer_falg.time_answer_get_bit_3_3) >= 1000))
	{
		//数据无效  //超时
		thread_arg->cmd_exec_timeout[BOARD_3_3] = 1;
	}
	if((thread_arg->cmd_answer_falg.answer_get_bit_1_2 == 1) && ((endTime - thread_arg->cmd_answer_falg.time_answer_get_bit_1_2) >= 1000))
	{
		//数据无效 //超时
		thread_arg->cmd_exec_timeout[BOARD_1_2] = 1;
	}
	if((thread_arg->cmd_answer_falg.answer_get_status == 1) && ((endTime - thread_arg->cmd_answer_falg.time_answer_get_status) >= 1000))
	{
		//数据无效 //超时
		thread_arg->cmd_exec_timeout[GET_STATUS] = 1;
	}
}

void *serial_recv_msg(void *arg)
{
	int ret;
	struct sockaddr_in testaddr;
	THREAD_ARG *thread_arg = (THREAD_ARG *)arg;
	unsigned char buffer[8];
	char sendbuf[8]={"error"};
	/*
	*逻辑	等待接受数据
	*			
	*/	
	while(thread_arg->thread_flag == 0)
	{
		memset(buffer,0,8);
		//这里设置的是等待1s超时
		//ret = serial_receive(thread_arg->serial_fd, buffer ,8);
		sleep(1);
		if(ret < 0 )
		{
			//去检测谁发出了命令  然后通知其数据超时
			check_datavalidity_timeout(thread_arg); 
			//通知网络串口超时 
			if(sendto(thread_arg->serial_fd, sendbuf, 8, 0, (struct sockaddr *)&(thread_arg->clientaddr), (socklen_t)sizeof(testaddr)) < 0)
			{
				printf("sned errmessage error\n"); 
			}
		}
		else
		{		
			//linkqueue_input(thread_arg->link_queue, buffer);
			//发网络
			//网络的串口命令
			SERIAL_DATA serial_message = {0};
			analysis_net_message(buffer,&serial_message);
			
			serial_message.type = WRITE_CONTRAST;
			serial_message.contrast_value = 10;
			
			if(sendto(thread_arg->serial_fd, &serial_message, sizeof(SERIAL_DATA), 0, (struct sockaddr *)&(thread_arg->clientaddr),  (socklen_t)sizeof(testaddr)) < 0)
			{
				printf("sned message error\n");
			}
		}		

	}
	return NULL;
}	