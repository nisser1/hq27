#ifndef __MSG_H_
#define __MSG_H_
#pragma pack (1)
typedef struct serial_data
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
#pragma pack ()
typedef enum hqCMD_TYPE
{
	WRITE_LIGHT,
	READ_LIGHT,
	WRITE_CONTRAST,			//对比度
	READ_CONTRAST,			//对比度
	BOARD_TEMP,
	TEMP_SENSOR1,
	TEMP_SENSOR2,
	BOARD_5V,
	BOARD_12V,
	BOARD_3_3,
	BOARD_1_2,
	GET_STATUS
}CMD_TYPE;

typedef struct CPU_PACKED
{
    char name[20];
    unsigned int user;
    unsigned int nice;
    unsigned int system;
    unsigned int idle;
}CPU_OCCUPY;

#endif
