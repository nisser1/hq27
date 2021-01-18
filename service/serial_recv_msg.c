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
	//printf("endTime is:%lld\n",endTime);
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
	int i = 0, loop_serch= 0;
	int led_flags = 0;
	FILE *fp;
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
		ret = serial_receive(thread_arg->serial_fd, buffer ,8);
		
		if(ret < 0){
			system("sed -i 's/ok/no/' /led_ctl.txt");

		}else{
			loop_serch++;
            /*
			printf("recv: ");
			for(i=0;i<8;i++)
			{
				printf("%02x ",buffer[i]);
			}
			printf("\n");*/
			//system("sed -i 's/no/ok/' /led_ctl.txt");
			analysis_net_message(buffer,&(thread_arg->serial_message_tonet));		
			
#if 0
		printf("lightVal=%d contrastValue=%d boardTmp=%02f tempSensor1=%f tempSensor2=%f board5V=%f borad12V=%f Board3.3=%f board1.2=%f disCurrentChn=%d status=%d type=%d\
		errorType=%d ExecRes=%d\n",\
		thread_arg->serial_message_tonet.light_value,\
		thread_arg->serial_message_tonet.contrast_value,\
		thread_arg->serial_message_tonet.board_temp,\
		thread_arg->serial_message_tonet.temp_sersor1,\
		thread_arg->serial_message_tonet.temp_sersor2,\
		thread_arg->serial_message_tonet.board_5v,\
		thread_arg->serial_message_tonet.board_12v,\
		thread_arg->serial_message_tonet.board_3_3v,\
		thread_arg->serial_message_tonet.board_1_2v,\
		thread_arg->serial_message_tonet.current_chn,\
		thread_arg->serial_message_tonet.status,\
		thread_arg->serial_message_tonet.type,\
		thread_arg->serial_message_tonet.error_type,\
		thread_arg->serial_message_tonet.cmd_exec_result);
#endif
			//逻辑	
		}		
		if(loop_serch < 10){
			continue;
        }
		else{

		    if(thread_arg->serial_message_tonet.board_temp > 100.0)
				led_flags = 1;
		    if(thread_arg->serial_message_tonet.temp_sersor1 > 100.0)
				led_flags = 1;
		    if(thread_arg->serial_message_tonet.temp_sersor2 > 100.0)
				led_flags = 1;
		    if(thread_arg->serial_message_tonet.board_5v > 8.0)
				led_flags = 1;
		    if(thread_arg->serial_message_tonet.board_12v > 15.0)
				led_flags = 1;
		    if(thread_arg->serial_message_tonet.board_3_3v > 5.0)
				led_flags = 1;
		    if(thread_arg->serial_message_tonet.board_1_2v > 2)
				led_flags = 1;
		    if(thread_arg->serial_message_tonet.status == 0)
				led_flags = 1;
		
		    if(led_flags == 1)
		    {
			    system("sed -i 's/ok/no/' /led_ctl.txt");
			    led_flags = 0;			
		    }
		    else
		    {
			    system("sed -i 's/no/ok/' /led_ctl.txt");
		    }	
			    memset(&(thread_arg->serial_message_tonet),0,sizeof(SERIAL_DATA));
			    loop_serch = 0;
            }
	
#if 0
		if(ret < 0 )
		{
			//去检测谁发出了命令  然后通知其数据超时
			check_datavalidity_timeout(thread_arg); 
			//通知网络串口超时 
			if(sendto(thread_arg->udp_sockfd, sendbuf, 8, 0, (struct sockaddr *)&(thread_arg->clientaddr), (socklen_t)sizeof(testaddr)) < 0)
			{
				printf("sned errmessage error\n"); 
			}
		}
		else
		{		
			//linkqueue_input(thread_arg->link_queue, buffer);
			//发网络
			//网络的串口命令
			//SERIAL_DATA serial_message;
			//memset(&serial_message,0,sizeof(serial_message));

#if 0			
			//printf("clinetip=%s clentport=%d  serial_mesageSize=%d\n",inet_ntoa(thread_arg->clientaddr.sin_addr),ntohs(thread_arg->clientaddr.sin_port),sizeof(SERIAL_DATA));	
			if(sendto(thread_arg->udp_sockfd, &(thread_arg->serial_message_tonet), sizeof(SERIAL_DATA), 0, (struct sockaddr *)&(thread_arg->clientaddr),  (socklen_t)sizeof(testaddr)) < 0)
			{
				printf("sned message error\n");
			}
#endif
		}
#endif
	}		
	return NULL;
}	
