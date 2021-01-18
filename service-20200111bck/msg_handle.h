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

unsigned int serial_cmd_packet(CMD_TYPE cmd,MSG *msg,unsigned char light,unsigned short contrast);
int analysis_net_message(unsigned char *serial_recv_buff,SERIAL_DATA *serial_data);

#endif
