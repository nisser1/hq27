#ifndef __MSG_PACKAGE_H_
#define __MSG_PACKAGE_H_
#include "service.h"
/*
作用:   封装串口命令,发送
@参数:	1 cmd       命令字 详见CMD_TYPE
		2 msg       串口消息
		3 light     亮度
返回值  
	成功返回0 失败返回错误码	
			错误码	PARAM_ERR		参数错误	
*/
typedef struct serial_d
{
	unsigned char type;
	unsigned char error_type;
	int		  cmd_exec_result;
	unsigned char light_value;
	unsigned char contrast_value;
	float board_temp;
	float temp_sersor1;
	float temp_sersor2;
	float board_5v;
	float board_12v;
	float board_3_3v;
	float board_1_2v;
	int current_chn;
	unsigned char status;
	
}SERIAL_DATA;
unsigned int serial_cmd_packet(CMD_TYPE cmd,MSG *msg,unsigned char light,unsigned short contrast);
int analysis_net_message(unsigned char *serial_recv_buff,SERIAL_DATA *serial_data);

#endif
