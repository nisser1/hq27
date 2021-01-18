#include<stdio.h>
#include<stdlib.h>
#include "msg_handle.h"


unsigned int serial_cmd_packet(CMD_TYPE cmd,MSG *msg,unsigned char light,unsigned short contrast)
{
	unsigned int check_sum= 0;
	if(msg == NULL)
	{
		return PARAM_ERR;
	}
	msg->size = 7;
	//帧头
	msg->data[0] = 0x55;
	msg->data[1] = 0xAA;
	
	//参数个数
	msg->data[3] = 0x03;
	
	//指令+参数
	switch(cmd)
	{
		case WRITE_LIGHT:
			msg->data[2] = 0x80;
			msg->data[4] = 0;
			msg->data[5] = light;
			msg->data[6] = 0;
			break;
		case READ_LIGHT:
			msg->data[2] = 0x40;
			msg->data[4] = 0;
			msg->data[5] = 0;
			msg->data[6] = 0;
			break;
		case WRITE_CONTRAST:			//对比度
			msg->data[2] = 0x20;
			msg->data[4] = 03;
			msg->data[5] = (unsigned char)(contrast);			//对比度高八位
			msg->data[6] = 0;					//对比度低八位
			break;
		case READ_CONTRAST:				//对比度
			msg->data[3] = 0x10;
			msg->data[4] = 0;
			msg->data[5] = 0;
			msg->data[6] = 0;
			break;
			
		case BOARD_TEMP:
			msg->data[2] = 0x50;
			msg->data[4] = 0x00;			//bit类型
			msg->data[5] = 0;
			msg->data[6] = 0;
			break;
		case TEMP_SENSOR1:
			msg->data[2] = 0x50;
			msg->data[4] = 0x01;			//bit类型
			msg->data[5] = 0;
			msg->data[6] = 0;
			break;
		case TEMP_SENSOR2:
			msg->data[2] = 0x50;
			msg->data[4] = 0x02;			//bit类型
			msg->data[5] = 0;
			msg->data[6] = 0;
			break;
		case BOARD_5V:
			msg->data[2] = 0x50;
			msg->data[4] = 0x04;			//bit类型
			msg->data[5] = 0;
			msg->data[6] = 0;
			break;
		case BOARD_12V:
			msg->data[2] = 0x50;
			msg->data[4] = 0x03;			//bit类型
			msg->data[5] = 0;
			msg->data[6] = 0;
			break;
		case BOARD_3_3:
			msg->data[2] = 0x50;
			msg->data[4] = 0x05;			//bit类型
			msg->data[5] = 0;
			msg->data[6] = 0;
			break;
		case BOARD_1_2:
			msg->data[2] = 0x50;
			msg->data[4] = 0x06;			//bit类型
			msg->data[5] = 0;
			msg->data[6] = 0;
			break;		
		case GET_STATUS:
			msg->data[2] = 0x08;
			msg->data[4] = 0;
			msg->data[5] = 0;
			msg->data[6] = 0;
			break;
		default:
			msg->size-=4;
			printf("msg packet cmd error\n");
			break;
	}
	//校验和
	int  i = 0;
	for(i=0;i<msg->size;i++)
	{
		check_sum += msg->data[i];
	}
	msg->data[7] = (unsigned char)(check_sum & 0x000000ff);	
	(msg->size)++;
	
	
	
	return (unsigned int)0;
}

unsigned int check_bit4(unsigned char bit4)
{
	unsigned int err_no = 0;
	switch(bit4)
	{
		case 0x01:
			err_no = SERIAL_CMD_SUCESS;
			break;
		case 0x02:
			err_no = SERIAL_CMDSE_CRCCRC_ERR;
			break;
		case 0x03:
			err_no = SERIAL_FRAME_HEADER_ERR;
			break;
		case 0x04:		
			err_no = SERIAL_CMDCM_ILLEGL_ERR;
			break;
		case 0x05:
			err_no = SERIAL_CMDCM_EXCFAI_ERR;
			break;
		default:
			printf("bit4 error\n");
	}
	return err_no;
	
}
int analysis_net_message(unsigned char *serial_recv_buff,SERIAL_DATA *serial_data)
{
	unsigned int ret;
	int i = 0;
	unsigned int check_sum = 0;
		
	for(i=0;i<7;i++)
	{
		check_sum += serial_recv_buff[i];
	}
	if(serial_recv_buff[8] != ((unsigned char)(check_sum & 0x000000ff)))
	{
		serial_data->cmd_exec_result = -1;
		return SERIAL_CMDRE_CRCCRC_ERR;
	}		
	
	if((ret = check_bit4(serial_recv_buff[4])) != SERIAL_CMD_SUCESS)
	{
		serial_data->cmd_exec_result = -1;
		return ret;
	}
	
	if((serial_recv_buff[0] != 0x55) && (serial_recv_buff[1] != 0xAA))
	{
		serial_data->cmd_exec_result = -1;
		return SERIAL_FRAME_HEADER_ERR;
	}
	
	
	if(serial_recv_buff[2] == 0x80)
	{
		serial_data->type = WRITE_LIGHT;
		serial_data->cmd_exec_result = 1;
	}
	else if(serial_recv_buff[2] == 0x40)
	{
		serial_data->type = READ_LIGHT;
		serial_data->cmd_exec_result = 1;
		serial_data->light_value = serial_recv_buff[5];
	}
	else if(serial_recv_buff[2] == 0x20)
	{
		serial_data->type = WRITE_CONTRAST;
		serial_data->cmd_exec_result = 1;
	}
	else if(serial_recv_buff[2] == 0x10)
	{
		serial_data->type = READ_CONTRAST;
		serial_data->cmd_exec_result = 1;
		//未验证
		serial_data->contrast_value = (((serial_recv_buff[6] * 256)+ serial_recv_buff[5]) - 156) / 2 ;
		
	}
	else if(serial_recv_buff[2] == 0x50)
	{
		//bit信息温度
		if(serial_recv_buff[6] == 0x00 )
		{
			serial_data->type = BOARD_TEMP;
			serial_data->cmd_exec_result = 1;
			serial_data->board_temp = ((float)serial_recv_buff[5]) / ((float) 150) - 60;
		}
		else if(serial_recv_buff[6] == 0x01)
		{
			serial_data->type = TEMP_SENSOR1;
			serial_data->cmd_exec_result = 1;
			serial_data->temp_sersor1 = ((float)serial_recv_buff[5]) / ((float) 150) - 60;
		}
		else if(serial_recv_buff[6] == 0x02)
		{
			serial_data->type = TEMP_SENSOR1;
			serial_data->cmd_exec_result = 1;
			serial_data->temp_sersor2 = ((float)serial_recv_buff[5]) / ((float) 150) - 60;
		}
		else if(serial_recv_buff[6] == 0x03)
		{
			serial_data->type = BOARD_12V ;
			serial_data->cmd_exec_result = 1;
			serial_data->board_5v= ((float)serial_recv_buff[5])/((float) 10);
		}
		else if(serial_recv_buff[6] == 0x04)
		{
			serial_data->type = BOARD_5V ;
			serial_data->cmd_exec_result = 1;
			serial_data->board_12v= ((float)serial_recv_buff[5])/((float) 10);
		}
		else if(serial_recv_buff[6] == 0x05)
		{
			serial_data->type = BOARD_3_3;
			serial_data->cmd_exec_result = 1;
			serial_data->board_3_3v= ((float)serial_recv_buff[5])/((float) 10);
		}
		else if(serial_recv_buff[6] == 0x06)
		{
			serial_data->type = BOARD_1_2;
			serial_data->cmd_exec_result = 1;
			serial_data->board_1_2v= ((float)serial_recv_buff[5])/((float) 10);
		}
		else 
		{
			printf("bit message recv error !!\n");
		}
	}
	else if(serial_recv_buff[2] == 0x08)
	{
		serial_data->type = GET_STATUS;
		serial_data->status = serial_recv_buff[5];
	}
	else
	{
		return SERIAL_COMMA_SOURCE_ERR;
	}

	if(serial_recv_buff[3] == 0x03)
	{
		return SERIAL_FRAME_LENGTH_ERR;
	}
	return 0;
}

