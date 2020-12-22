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
#include "serial_send_msg.h"
#include "msg_netc.h"

/*
本文件内部使用函数
*/
static void set_cmd_send_time(THREAD_ARG *thread_arg,int cmd_type)
{
	struct timeval begin;
	gettimeofday(&begin,NULL);
	long long beginTime = (long long)begin.tv_sec * 1000 + (long long)begin.tv_usec / 1000;
	printf("beginTime is:%lld\n",beginTime);
	switch(cmd_type)
	{
		case WRITE_LIGHT:
			thread_arg->cmd_answer_falg.time_write_light_result = beginTime;
			thread_arg->cmd_answer_falg.write_light_result = 1;
			thread_arg->cmd_exec_timeout[WRITE_LIGHT] = 0;
			break;
		case READ_LIGHT:
			thread_arg->cmd_answer_falg.time_read_light_result = beginTime;
			thread_arg->cmd_answer_falg.read_light_result = 1;
			thread_arg->cmd_exec_timeout[READ_LIGHT] = 0;
			break;
		case WRITE_CONTRAST:			
			thread_arg->cmd_answer_falg.time_write_contrast_result = beginTime;
			thread_arg->cmd_answer_falg.write_contrast_result = 1;
			thread_arg->cmd_exec_timeout[WRITE_CONTRAST] = 0;
			break;
		case READ_CONTRAST:				
			thread_arg->cmd_answer_falg.time_read_contrast_result = beginTime;
			thread_arg->cmd_answer_falg.read_contrast_result = 1;
			thread_arg->cmd_exec_timeout[READ_CONTRAST] = 0;
			break;
		case BOARD_TEMP:
			thread_arg->cmd_answer_falg.time_answer_get_bit_board_temp = beginTime;
			thread_arg->cmd_answer_falg.answer_get_bit_board_temp = 1;
			thread_arg->cmd_exec_timeout[BOARD_TEMP] = 0;
			break;
		case TEMP_SENSOR1:
			thread_arg->cmd_answer_falg.time_answer_get_bit_sensor1_temp = beginTime;
			thread_arg->cmd_answer_falg.answer_get_bit_sensor1_temp = 1;
			thread_arg->cmd_exec_timeout[TEMP_SENSOR1] = 0;
			break;
		case TEMP_SENSOR2:
			thread_arg->cmd_answer_falg.time_answer_get_bit_sensor2_temp = beginTime;
			thread_arg->cmd_answer_falg.answer_get_bit_sensor2_temp = 1;
			thread_arg->cmd_exec_timeout[TEMP_SENSOR2] = 0;
			break;
		case BOARD_5V:
			thread_arg->cmd_answer_falg.time_answer_get_bit_5 = beginTime;
			thread_arg->cmd_answer_falg.answer_get_bit_5 = 1;
			thread_arg->cmd_exec_timeout[BOARD_5V] = 0;
			break;
		case BOARD_12V:
			thread_arg->cmd_answer_falg.time_answer_get_bit_12 = beginTime;
			thread_arg->cmd_answer_falg.answer_get_bit_12 = 1;
			thread_arg->cmd_exec_timeout[BOARD_12V] = 0;
			break;
		case BOARD_3_3:
			thread_arg->cmd_answer_falg.time_answer_get_bit_3_3 = beginTime;
			thread_arg->cmd_answer_falg.answer_get_bit_3_3 = 1;
			thread_arg->cmd_exec_timeout[BOARD_3_3] = 0;
			break;
		case BOARD_1_2:
			thread_arg->cmd_answer_falg.time_answer_get_bit_1_2 = beginTime;
			thread_arg->cmd_answer_falg.answer_get_bit_1_2 = 1;
			thread_arg->cmd_exec_timeout[BOARD_1_2] = 0;
			break;		
		case GET_STATUS:
			thread_arg->cmd_answer_falg.time_answer_get_status = beginTime;
			thread_arg->cmd_answer_falg.answer_get_status = 1;
			thread_arg->cmd_exec_timeout[GET_STATUS] = 0;
			break;
		default:
			
			printf("cmd error\n");
			break;
	}
		
	return ;
}




//发送串口命令线程
void *serial_send_msg(void *arg)
{
	unsigned int ret;
	//ret = 0;
	//printf("dd %d\n",ret);
	MSG msg;
	unsigned short contrast = 0;
	int i = 0;
	THREAD_ARG *thread_arg = (THREAD_ARG *)arg;
	
	/*
	*逻辑	等待接受iic的命令再通过串口转发至给到大显示器
	*			目前只是模拟
	*/	
	pause();
	while(thread_arg->thread_flag == 0)
	{
		pthread_mutex_lock(&(thread_arg->mutex));
		pthread_cond_wait(&(thread_arg->cond_serial),&(thread_arg->mutex));
		pthread_mutex_unlock(&(thread_arg->mutex));
		switch(thread_arg->serial_cmd)
		{
			
			case 0:
				//发送亮度命令		
				ret = serial_cmd_packet(WRITE_LIGHT,&msg,50,contrast);		
				if(serial_send(thread_arg->serial_fd, msg.data,msg.size) > 0)
				{
					printf("serial_send sucess\n");
				}
				set_cmd_send_time(thread_arg,WRITE_LIGHT);
				break;
			case 1:
				//读取亮度命令
				ret = serial_cmd_packet(READ_LIGHT,&msg,50,0);		
				if(serial_send(thread_arg->serial_fd, msg.data,msg.size) > 0)
				{
					printf("serial_send sucess\n");
				}
				set_cmd_send_time(thread_arg,READ_LIGHT);
				break;
			case 2:
				//写对比度命令	
				ret = serial_cmd_packet(WRITE_CONTRAST,&msg,50,50);		
				if(serial_send(thread_arg->serial_fd, msg.data,msg.size) > 0)
				{
					printf("serial_send sucess\n");
				}
				set_cmd_send_time(thread_arg,WRITE_CONTRAST);
				break;
			case 3:
				//读对比度命令		
				ret = serial_cmd_packet(READ_CONTRAST,&msg,50,50);		
				if(serial_send(thread_arg->serial_fd, msg.data,msg.size) > 0)
				{
					printf("serial_send sucess\n");
				}
				set_cmd_send_time(thread_arg,READ_CONTRAST);
				break;
			case 4:
				////板卡温度
				ret = serial_cmd_packet(4,&msg,50,50);		
				if(serial_send(thread_arg->serial_fd, msg.data,msg.size) > 0)
				{
					printf("serial_send sucess\n");
				}
				set_cmd_send_time(thread_arg,BOARD_TEMP);
				break;
			case 5:
				//温度传感器1
				ret = serial_cmd_packet(5,&msg,50,50);		
				if(serial_send(thread_arg->serial_fd, msg.data,msg.size) > 0)
				{
					printf("serial_send sucess\n");
				}
				set_cmd_send_time(thread_arg,TEMP_SENSOR1);
				break;
			case 6:
				//温度传感器2
				ret = serial_cmd_packet(6,&msg,50,50);		
				if(serial_send(thread_arg->serial_fd, msg.data,msg.size) > 0)
				{
					printf("serial_send sucess\n");
				}
				set_cmd_send_time(thread_arg,TEMP_SENSOR2);
				break;
			case 7:
				//板卡+5V电压
				ret = serial_cmd_packet(7,&msg,50,50);	
				if(serial_send(thread_arg->serial_fd, msg.data,msg.size) > 0)
				{
					printf("serial_send sucess\n");
				}
				set_cmd_send_time(thread_arg,BOARD_5V);
				break;
			case 8:
				//板卡+12V电压
				ret = serial_cmd_packet(8,&msg,50,50);		
				if(serial_send(thread_arg->serial_fd, msg.data,msg.size) > 0)
				{
					printf("serial_send sucess\n");
				}
				set_cmd_send_time(thread_arg,BOARD_12V);
				break;
			case 9:
				//板卡+3.3V电压
				ret = serial_cmd_packet(9,&msg,50,50);		
				if(serial_send(thread_arg->serial_fd, msg.data,msg.size) > 0)
				{
					printf("serial_send sucess\n");
				}
				set_cmd_send_time(thread_arg,BOARD_3_3);
				break;
			case 10:
				//板卡+1.2V电压
				ret = serial_cmd_packet(10,&msg,50,50);		
				if(serial_send(thread_arg->serial_fd, msg.data,msg.size) > 0)
				{
					printf("serial_send sucess\n");
				}
				set_cmd_send_time(thread_arg,BOARD_1_2);
				break;
			case 11:
				//查询状态
				ret = serial_cmd_packet(GET_STATUS,&msg,50,50);	
				if(serial_send(thread_arg->serial_fd, msg.data,msg.size) > 0)
				{
					printf("serial_send sucess\n");
				}
				set_cmd_send_time(thread_arg,GET_STATUS);
				break;
			default:
				//printf("net to serial cmd error %d !!\n",ret);
				break;
		}
		
			for(i=0;i<msg.size;i++)
			{
				printf("%02x ",(unsigned char)msg.data[i]);
			}
			printf("\n");
	}	
	return NULL;
}	