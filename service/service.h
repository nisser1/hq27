#ifndef __SERVICE_H_
#define __SERVICE_H_
#include <arpa/inet.h> 	//inet_addr htons
#include <sys/types.h>
#include <sys/socket.h> //socket bind listen accept connect
#include <netinet/in.h> //sockaddr_in
#include <stdlib.h> 	//exit
#include <unistd.h> 	//close
#include <pthread.h>
#include <sys/time.h>

#define PARAM_ERR 0x400ff001

#define DIRECT_TYPE_NUM 12
#define NET_MESSAGE_LEN 64

#define SERIAL_CMD_SUCESS 		0xffff0100
#define SERIAL_FRAME_HEADER_ERR 0xffff0101
#define SERIAL_COMMA_SOURCE_ERR 0xffff0102
#define SERIAL_FRAME_LENGTH_ERR 0xffff0103
#define SERIAL_CMDSE_CRCCRC_ERR 0xffff0104
#define SERIAL_CMDCM_ILLEGL_ERR 0xffff0105
#define SERIAL_CMDCM_EXCFAI_ERR 0xffff0106
#define SERIAL_CMDRE_CRCCRC_ERR 0xffff0107

unsigned char contrast_net;
unsigned char light;


typedef struct node{
	unsigned char data[8];
	struct node *next;
}linknode_t;

typedef struct{
	linknode_t *front;
	linknode_t *rear;//队列尾部
}LINK_QUEUE;


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

//相继
typedef enum
{
	ALL_BOARDS_RF1, // 0
	ALL_BOARDS_RF2, // 1
	ALL_BOARDS_CVBS1, // 2
	ALL_BOARDS_CVBS2, // 3
	ALL_BOARDS_CVBS3, // 4
	ALL_BOARDS_SVIDEO1, // 5
	ALL_BOARDS_SVIDEO2, // 6
	ALL_BOARDS_SVIDEO3, // 7
	ALL_BOARDS_COMP1, // 8
	ALL_BOARDS_COMP2, // 9
	ALL_BOARDS_COMP3, // 10
	ALL_BOARDS_SIG1_SCART1, // 11
	ALL_BOARDS_SIG2_SCART1, // 12
	ALL_BOARDS_SIG3_SCART1, // 13
	ALL_BOARDS_SIG4_SCART1, // 14
	ALL_BOARDS_SIG1_SCART2, // 15
	ALL_BOARDS_SIG2_SCART2, // 16
	ALL_BOARDS_SIG3_SCART2, // 17
	ALL_BOARDS_SIG4_SCART2, // 18
	ALL_BOARDS_SIG1_SCART3, // 19
	ALL_BOARDS_SIG2_SCART3, // 20
	ALL_BOARDS_SIG3_SCART3, // 21
	ALL_BOARDS_SIG4_SCART3, // 22
	ALL_BOARDS_VGA1, // 23
	ALL_BOARDS_VGA2, // 24
	ALL_BOARDS_DVI1, // 25
	ALL_BOARDS_DIP1, // 26
	ALL_BOARDS_HDMI, // 27
	ALL_BOARDS_HDMI2, // 28
	ALL_BOARDS_HDMI3, // 29
	ALL_BOARDS_DP1, // 30
	ALL_BOARDS_DP2, // 31
	NOT_USED, // 32ALL_BOARDS_SLAVE1, // 33
	NUM_MASTER_LIST, // 34
} ALL_BOARDS_INPUT_PORT;







//这个暂时先不做
typedef struct ip_info{
	char *ipaddr[32];
	unsigned short write_light;
	unsigned short read_light;
	unsigned short write_contrast;
	unsigned short read_contrast;
	unsigned short board_temp;
	unsigned short temp_sersor1;
	unsigned short temp_sersor2;
	unsigned short board_5v;
	unsigned short board_12v;
	unsigned short board_3_3v;
	unsigned short board_1_2v;
	unsigned short board_status;
	struct ip_info *next;
}LINK_IP_INFO;

//命令类型
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
//显示器命令报文回复标志
typedef struct hqMONITOR_ANSWER_FLAG
{
	long long time_write_light_result;
	long long time_read_light_result;
	long long time_write_contrast_result;
	long long time_read_contrast_result;
	long long time_answer_get_bit_board_temp;
	long long time_answer_get_bit_sensor1_temp;
	long long time_answer_get_bit_sensor2_temp;
	long long time_answer_get_bit_5;
	long long time_answer_get_bit_12;
	long long time_answer_get_bit_3_3;
	long long time_answer_get_bit_1_2;
	long long time_answer_get_status;
	
	unsigned short write_light_result;
	unsigned short read_light_result;
	unsigned short write_contrast_result;
	unsigned short read_contrast_result;
	unsigned short answer_get_bit_board_temp;
	unsigned short answer_get_bit_sensor1_temp;
	unsigned short answer_get_bit_sensor2_temp;
	unsigned short answer_get_bit_5;
	unsigned short answer_get_bit_12;
	unsigned short answer_get_bit_3_3;
	unsigned short answer_get_bit_1_2;
	unsigned short answer_get_status;
	
}MONITOR_ANSWER_FLAG;

typedef struct{
    int size;
    unsigned char data[8];
}MSG;

typedef struct hqTHREAD_ARG
{
	int					serial_fd;								//串口描述符
	int					udp_sockfd;
	int					thread_flag;							//线程运行标志
	unsigned char		serial_cmd;
	unsigned char		iic_cmd;
	MONITOR_ANSWER_FLAG cmd_answer_falg;						//发送线程命令是否收到回复标志
	pthread_mutex_t 	mutex;									//线程锁
	pthread_cond_t 		cond_serial;									//串口线程变量
	pthread_cond_t 		cond2_iic;									//iic线程变量
	//LINK_QUEUE 		link_queue;								//数据队列
	struct sockaddr_in  clientaddr;
	unsigned char 		cmd_exec_timeout[DIRECT_TYPE_NUM];		//数据超时列表
	SERIAL_DATA			serial_message_tonet;
}THREAD_ARG;

#endif
