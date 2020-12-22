#ifndef STATUSCTRLLIB_GLOBAL_H
#define STATUSCTRLLIB_GLOBAL_H

#define OK_EXECUTE	 (1)			//执行成功

#define ERROR_GET_MSG_ID_FAIL	(-14)			//初始化失败
#define ERROR_PARAMETER          (-22)		//参数错误
#define ERROR_THREAD_CANCEL_FAIL (-23)		//线程取消失败
#define ERROR_THREAD_JOIN_FAIL (-24)			//线程回收资源失败
#define ERROR_THREAD_CREATE_FAIL (-26)		//线程创建失败
#define ERROR_SEND_FAIL (-25)                    //消息发送失败
#define ERROR_COM_CS (2)			//串口命令校验错误
#define ERROR_COM_FH (3)			//串口命令帧头错误
#define ERROR_COM_CMD (4)			//串口命令无效
#define ERROR_COM_EXE_FAIL (5)		//串口命令执行失败

typedef enum
{
		halt = 0x00,
		powerup = 0x01,
		reboot = 0x03
} POWERSTATUS;

typedef struct PowerBoardSensorData_
{
		float voltage_12v;//12V电压
		float voltage_5v; //5V电压
		float voltage_3_3v; //3.3V电压
		float electric_current_12v; //12V电流
		float board_temp;//电源板温度
} PowerBoardSensorData;


typedef struct StoreBoardSensorData_
{
		float voltage_5v;//5v电压
		float electric_current_5v; //5v电流
		float board_temp; //外存板温度
} StoreBoardSensorData;


typedef struct MainBoardSensorData_
{
		float voltage_12v;//12v电压
		float voltage_5v; //5v电压
		float voltage_3_3v; //3.3v电压
		float voltage_1_8v; //1.8v电压
		float voltage_1_5_dd3_v;//1.5v dd3电压
		float voltage_1_0_5_v; //1.05v电压
		float voltage_1_0_v; //1.0v电压
		float voltage_0_95_v; //0.95v电压
		float voltage_0_9_core_v; //0.9v电压
		float voltage_0_75_v; //0.75v电压
		float cpu_temp;//CPU温度
		float board_temp;//主板温度
} MainBoardSensorData; 


typedef struct POWER_ON_PARA_
{
        int firstHw;    //第一加电设备ID
        int secondHw;   //第二加电设备ID
        int thirdHw;    //第三加电设备ID
        int firstDelay;   //第一加电设备延时
        int secondDelay;//第二加电设备延时
        int thirdDelay;  //第三加电设备延时
}POWER_ON_PARA;

typedef struct DISPLAY_DISK_INFO_
{
    int disk_size;//剩余容量,单位MB
    int disk_full;//是否满 ,0：未满，1：满
    int disk_alarm;//是否报警 0：未报警，1：报警
}DISPLAY_DISK_INFO;


#ifdef __cplusplus
extern "C"{
#endif 
/*************************************************************************************
Description:加载状态控制板驱动,加载时以异步115200bps波特率、偶校验、无流控方式打开该串口，向状态控制
            板发送握手协议，进行建链
Input:无
Output:无
Return: 执行正常返回OK_EXECUTE(1),失败返回失败返回错误码：
        创建消息队列失败ERROR_GET_MSG_ID_FAIL（-14）
        创建接收消息线程失败ERROR_THREAD_CREATE_FAIL (-26)
**************************************************************************************/
int StatusCtrlBoardLoad(void);

/*************************************************************************************
Description: 注册按键按下抬起时的响应函数
Input:BtnCallback(int keycode,int status)，回调函数
      keycode为按键码 ，status为按键状态，
	  序号	按键名称	  keycode		status			  	Lampcode
		1	标准/应急切换	0		0为应急、1为标准			-
		2	硬功能键1		1		0为按下、1为抬起			1
		3	硬功能键2		2		0为按下、1为抬起			2
		4	硬功能键3		3		0为按下、1为抬起			3
		5	硬功能键4		4		0为按下、1为抬起			4
		6	硬功能键5		5		0为按下、1为抬起			5
		7	硬功能键6		6		0为按下、1为抬起			6
		8	硬功能键7		7		0为按下、1为抬起			7
		9	硬功能键8		8		0为按下、1为抬起			8
Output:无
Return:成功返回OK_EXECUTE(1),失败返回错误码：
		创建接收按键消息线程失败ERROR_THREAD_CREATE_FAIL (-26)      
**************************************************************************************/
int BtnStatus(int (*BtnCallback)(int keycode,int status));
   
/*************************************************************************************
Description: 按键指示灯控制
Input: key -输入参数，灯码 （同上）                     
       value -输入参数，0为灭灯，1为亮灯
Output:无	   
Return:命令执行正常返回OK_EXECUTE(1) 
		命令执行失败返回错误码：
		参数错误 ERROR_PARAMETER（-22）
		创建消息队列失败ERROR_GET_MSG_ID_FAIL（-14）
		命令发送失败ERROR_SEND_FAIL (-25)
		命令CRC校验错误ERROR_COM_CS（2）
		命令帧错误ERROR_COM_FH（3）
		命令码无效ERROR_COM_CMD（4）
		命令执行失败ERROR_COM_EXE_FAIL（5）

**************************************************************************************/
int LampCtrl(const int key, const int value);

/*************************************************************************************
Description:读取当前主板BIOS时间,设置状态控制板的RTC时间
Input:无
Output:无
Return: 命令执行正常返回OK_EXECUTE(1)
        执行失败返回错误码：
        创建消息队列失败ERROR_GET_MSG_ID_FAIL（-14）
        命令发送失败ERROR_SEND_FAIL (-25)
        命令CRC校验错误ERROR_COM_CS（2）
        命令帧头错误ERROR_COM_FH（3）
        命令码错误ERROR_COM_CMD（4）
        命令执行失败ERROR_COM_EXE_FAIL（5）
**************************************************************************************/
int StatusCtrlBoardRTCSet(void);

/*************************************************************************************
Description:状态控制板自检
Input:无
Output:无
Return:状态控制板正常返回(0),状态控制板故障返回（0xFF）
		命令执行失败返回错误码：
		创建消息队列失败ERROR_GET_MSG_ID_FAIL（-14）
		命令发送失败ERROR_SEND_FAIL (-25)
		命令CRC校验错误ERROR_COM_CS（2）
		命令帧错误ERROR_COM_FH（3）
		命令码无效ERROR_COM_CMD（4）
		命令执行失败ERROR_COM_EXE_FAIL（5）
**************************************************************************************/
int StatusCtrlBoardSelfTest(void);

/*************************************************************************************
Description:显示器自检
Input:screenID-显示器序号，0表示下显示器，1表示上显示器
Output:无
Return:	显示器正常返回(0x00), 显示器故障返回（0xFF）（无法检测到信号源输入时显示器为故障）
        命令执行失败返回错误码：
        参数错误 ERROR_PARAMETER（-22）
        创建消息队列失败ERROR_GET_MSG_ID_FAIL（-14）
        命令发送失败ERROR_SEND_FAIL (-25)
		命令CRC校验错误ERROR_COM_CS（2）
        命令帧头错误ERROR_COM_FH（3）
        命令码错误ERROR_COM_CMD（4）
        命令执行失败ERROR_COM_EXE_FAIL（5）
**************************************************************************************/
 int DisplaySelfTest(int screenID);      

/*************************************************************************************
Description:加电控制模块自检
Input:无
Output:无
Return:加电控制模块正常返回(0x00), 加电控制模块故障返回（0xFF）
        命令执行失败返回错误码：
        创建消息队列失败ERROR_GET_MSG_ID_FAIL（-14）
        命令发送失败ERROR_SEND_FAIL (-25)
		命令CRC校验错误ERROR_COM_CS（2）
        命令帧头错误ERROR_COM_FH（3）
        命令码错误ERROR_COM_CMD（4）
        命令执行失败ERROR_COM_EXE_FAIL（5）
**************************************************************************************/
int PowerCtrlSelfTest(void);

/*************************************************************************************
Description:状态显示屏自检
Input:无
Output:无
Return:状态显示屏正常返回(0x00), 状态显示屏故障返回（0xFF）
        命令执行失败返回错误码：
        创建消息队列失败ERROR_GET_MSG_ID_FAIL（-14）
        命令发送失败ERROR_SEND_FAIL (-25)
		命令CRC校验错误ERROR_COM_CS（2）
        命令帧头错误ERROR_COM_FH（3）
        命令码错误ERROR_COM_CMD（4）
        命令执行失败ERROR_COM_EXE_FAIL（5）
**************************************************************************************/
int StatusScreenSelfTest(void);

/*************************************************************************************
Description:蜂鸣器控制
Input:state-输入参数，蜂鸣器状态，0-关闭，1-启动
Output:无
Return: 命令执行正常返回OK_EXECUTE(1)
        命令执行失败返回错误码：
        参数错误 ERROR_PARAMETER（-22）
        创建消息队列失败ERROR_GET_MSG_ID_FAIL（-14）
        命令发送失败ERROR_SEND_FAIL (-25)
        命令CRC校验错误ERROR_COM_CS（2）
        命令帧头错误ERROR_COM_FH（3）
        命令码错误ERROR_COM_CMD（4）
        命令执行失败ERROR_COM_EXE_FAIL（5）
**************************************************************************************/
int BuzzerCtrl(int state);

/*************************************************************************************
Description:风机状态查询,主板向状态控制板发送风机状态查询指令，从收到的串口数据解析风机状态。
Input:无
Output:state-输出参数，风机状态，bit0-bit3 为第一路风机状态 bit4-bit7为第二路风机状态
			第一路风机状态结果为state&0x0f后 0-正常速度，1-低速转动，2-高速转动，3-停止转动
			第二路风机状态结果为(state&0xf0)>>4后 0-正常速度，1-低速转动，2-高速转动，3-停止转动
Return:命令执行正常返回OK_EXECUTE(1)
        命令执行失败返回错误码：
        参数错误 ERROR_PARAMETER（-22）
        创建消息队列失败ERROR_GET_MSG_ID_FAIL（-14）
        命令发送失败ERROR_SEND_FAIL (-25)
        命令CRC校验错误ERROR_COM_CS（2）
        命令帧头错误ERROR_COM_FH（3）
        命令码错误ERROR_COM_CMD（4）
        命令执行失败ERROR_COM_EXE_FAIL（5）
**************************************************************************************/
int FanStateQuery(int *state);
 
/*************************************************************************************
Description:主板环境参数查询
Input:无
Output:envPara -输出参数，主板环境参数结构体
Return: 命令执行正常返回OK_EXECUTE(1)
        命令执行失败返回错误码：
        参数错误 ERROR_PARAMETER（-22）
        创建消息队列失败ERROR_GET_MSG_ID_FAIL（-14）
        命令发送失败ERROR_SEND_FAIL (-25)
        命令CRC校验错误ERROR_COM_CS（2）
        命令帧头错误ERROR_COM_FH（3）
        命令码错误ERROR_COM_CMD（4）
        命令执行失败ERROR_COM_EXE_FAIL（5）
**************************************************************************************/
int MainBoardEnvQuery(MainBoardSensorData  *envPara);

/*************************************************************************************
Description:电源板环境参数查询
Input:无
Output:envPara -输出参数，电源板环境参数结构体
Return:命令执行正常返回OK_EXECUTE(1)
        命令执行失败返回错误码：
        参数错误 ERROR_PARAMETER（-22）
        创建消息队列失败ERROR_GET_MSG_ID_FAIL（-14）
        命令发送失败ERROR_SEND_FAIL (-25)
        命令CRC校验错误ERROR_COM_CS（2）
        命令帧头错误ERROR_COM_FH（3）
        命令码错误ERROR_COM_CMD（4）
        命令执行失败ERROR_COM_EXE_FAIL（5）
**************************************************************************************/
int PowerBoardEnvQuery(PowerBoardSensorData  *envPara);

/*************************************************************************************
Description: 外存板环境参数查询
Input:无
Output:envPara -输出参数，外存板环境参数结构体
Return:命令执行正常返回OK_EXECUTE(1)
        命令执行失败返回错误码：
        参数错误 ERROR_PARAMETER（-22）
        创建消息队列失败ERROR_GET_MSG_ID_FAIL（-14）
        命令发送失败ERROR_SEND_FAIL (-25)
        命令CRC校验错误ERROR_COM_CS（2）
        命令帧头错误ERROR_COM_FH（3）
        命令码错误ERROR_COM_CMD（4）
        命令执行失败ERROR_COM_EXE_FAIL（5）
**************************************************************************************/
int StorBoardEnvQuery(StoreBoardSensorData  *envPara);  

/************************************************************************************
Description: 温度阈值设置
Input:参数： low -输入参数，温度阈值下限；
            high -输入参数，温度阈值上限
Output:无
Return:命令执行正常返回OK_EXECUTE(1)
        命令执行失败返回错误码：
        参数错误 ERROR_PARAMETER（-22）
        创建消息队列失败ERROR_GET_MSG_ID_FAIL（-14）
        命令发送失败ERROR_SEND_FAIL (-25)
        命令CRC校验错误ERROR_COM_CS（2）
        命令帧头错误ERROR_COM_FH（3）
        命令码错误ERROR_COM_CMD（4）
        命令执行失败ERROR_COM_EXE_FAIL（5）
**************************************************************************************/
int TempLimit(int low, int high);

/*************************************************************************************
Description: 加电时序设置
Input:struPowerOnPara-输入参数，加电顺序参数
Output:无
Return:命令执行正常返回OK_EXECUTE(1)
        命令执行失败返回错误码：
        参数错误 ERROR_PARAMETER（-22）
        创建消息队列失败ERROR_GET_MSG_ID_FAIL（-14）
        命令发送失败ERROR_SEND_FAIL (-25)
        命令CRC校验错误ERROR_COM_CS（2）
        命令帧头错误ERROR_COM_FH（3）
        命令码错误ERROR_COM_CMD（4）
        命令执行失败ERROR_COM_EXE_FAIL（5）
**************************************************************************************/
int PowerOnSequence(POWER_ON_PARA struPowerOnPara);

/*************************************************************************************
Description: 显示器亮度调节
Input:chn -输入参数，显示器序号，0表示下显示器，1表示上显示器
      value -输入参数，亮度值，取值范围1-100
Output:无
Return:命令执行正常返回OK_EXECUTE(1)
        命令执行失败返回错误码：
        参数错误 ERROR_PARAMETER（-22）
        创建消息队列失败ERROR_GET_MSG_ID_FAIL（-14）
        命令发送失败ERROR_SEND_FAIL (-25)
        命令CRC校验错误ERROR_COM_CS（2）
        命令帧头错误ERROR_COM_FH（3）
        命令码错误ERROR_COM_CMD（4）
        命令执行失败ERROR_COM_EXE_FAIL（5）
**************************************************************************************/
int DisplayBrightCtrl(int chn, int value);

/*************************************************************************************     
Description: 显示器设置ip
Input:	chn -输入参数，显示器序号，0表示下显示器，1表示上显示器
		IP -输入参数，IP地址,占4个字节，
		如“192.168.0.12”,IP[0]=192， IP[1]=168， IP[2]=0， IP[3]=12

Output:无
Return:命令执行正常返回OK_EXECUTE(1)
        命令执行失败返回错误码：
        参数错误 ERROR_PARAMETER（-22）
        创建消息队列失败ERROR_GET_MSG_ID_FAIL（-14）
        命令发送失败ERROR_SEND_FAIL (-25)
        命令CRC校验错误ERROR_COM_CS（2）
        命令帧头错误ERROR_COM_FH（3）
        命令码错误ERROR_COM_CMD（4）
        命令执行失败ERROR_COM_EXE_FAIL（5）
**************************************************************************************/
int DisplaySetIP(int chn ,char *IP);

/*************************************************************************************
Description: 显示器设置端口号
Input:	chn -输入参数，显示器序号，0表示下显示器，1表示上显示器
		port -输入参数，端口号 取值范围0-65535

Output:无
Return:命令执行正常返回OK_EXECUTE(1)
        命令执行失败返回错误码：
        参数错误 ERROR_PARAMETER（-22）
        创建消息队列失败ERROR_GET_MSG_ID_FAIL（-14）
        命令发送失败ERROR_SEND_FAIL (-25)
        命令CRC校验错误ERROR_COM_CS（2）
        命令帧头错误ERROR_COM_FH（3）
        命令码错误ERROR_COM_CMD（4）
        命令执行失败ERROR_COM_EXE_FAIL（5）
**************************************************************************************/
int DisplaySetPort(int chn , unsigned short port);

/*************************************************************************************
Description: 显示器获取亮度
Input:chn -输入参数，显示器序号，0表示下显示器，1表示上显示器
Output:value -输出参数，亮度值，取值范围1-100
Return:命令执行正常返回OK_EXECUTE(1)
        命令执行失败返回错误码：
        参数错误 ERROR_PARAMETER（-22）
        创建消息队列失败ERROR_GET_MSG_ID_FAIL（-14）
        命令发送失败ERROR_SEND_FAIL (-25)
        命令CRC校验错误ERROR_COM_CS（2）
        命令帧头错误ERROR_COM_FH（3）
        命令码错误ERROR_COM_CMD（4）
        命令执行失败ERROR_COM_EXE_FAIL（5）
**************************************************************************************/
int DisplayGetBrightness(int chn ,int *value);

/************************************************************************************* 
Description: 显示器设置分辨率
Input:chn -输入参数，显示器序号，0表示下显示器，1表示上显示器
		value -输入参数，分辨率，取值范围：1：1080P，2：720P
Output:无
Return:命令执行正常返回OK_EXECUTE(1)
        命令执行失败返回错误码：
        参数错误 ERROR_PARAMETER（-22）
        创建消息队列失败ERROR_GET_MSG_ID_FAIL（-14）
        命令发送失败ERROR_SEND_FAIL (-25)
        命令CRC校验错误ERROR_COM_CS（2）
        命令帧头错误ERROR_COM_FH（3）
        命令码错误ERROR_COM_CMD（4）
        命令执行失败ERROR_COM_EXE_FAIL（5）
**************************************************************************************/
int DisplaySetResolution(int chn ,int value);

/*************************************************************************************
Description: 显示器设置帧率
Input:chn -输入参数，显示器序号，0表示下显示器，1表示上显示器
		value -输入参数，帧率，取值范围：1-30

Output:无
Return:命令执行正常返回OK_EXECUTE(1)
        命令执行失败返回错误码：
        参数错误 ERROR_PARAMETER（-22）
        创建消息队列失败ERROR_GET_MSG_ID_FAIL（-14）
        命令发送失败ERROR_SEND_FAIL (-25)
        命令CRC校验错误ERROR_COM_CS（2）
        命令帧头错误ERROR_COM_FH（3）
        命令码错误ERROR_COM_CMD（4）
        命令执行失败ERROR_COM_EXE_FAIL（5）
**************************************************************************************/
int DisplaySetFrameRate(int chn ,int value);

/*************************************************************************************
Description: 显示器获取信号源 
Input:chn -输入参数，显示器序号，0表示下显示器，1表示上显示器
Output:value -输出参数，1：VGA，2：DVI
Return:命令执行正常返回OK_EXECUTE(1)
        命令执行失败返回错误码：
        参数错误 ERROR_PARAMETER（-22）
        创建消息队列失败ERROR_GET_MSG_ID_FAIL（-14）
        命令发送失败ERROR_SEND_FAIL (-25)
        命令CRC校验错误ERROR_COM_CS（2）
        命令帧头错误ERROR_COM_FH（3）
        命令码错误ERROR_COM_CMD（4）
        命令执行失败ERROR_COM_EXE_FAIL（5）
**************************************************************************************/
int DisplayGetSignalSource(int chn ,int *value);  

/*************************************************************************************
Description: 显示器控制录屏
Input: chn -输入参数，显示器序号，0表示下显示器，1表示上显示器
		value -输入参数，1 启动录屏 0 停止录屏
Output:无
Return:命令执行正常返回OK_EXECUTE(1) 
		命令执行失败返回错误码：
		参数错误 ERROR_PARAMETER（-22）
		创建消息队列失败ERROR_GET_MSG_ID_FAIL（-14）
		命令发送失败ERROR_SEND_FAIL (-25)
		命令CRC校验错误ERROR_COM_CS（2）
		命令帧错误ERROR_COM_FH（3）
		命令码无效ERROR_COM_CMD（4）
		命令执行失败ERROR_COM_EXE_FAIL（5）
**************************************************************************************/
int DisplayCtrlVideoRecord(int chn ,int value);

/*************************************************************************************
Description: 获取磁盘容量
Input: chn -输入参数，显示器序号，0表示下显示器，1表示上显示器                      
Output:diskInfo-输出参数 剩余磁盘容量
Return:命令执行正常返回OK_EXECUTE(1) 
		命令执行失败返回错误码：
		参数错误 ERROR_PARAMETER（-22）
		创建消息队列失败ERROR_GET_MSG_ID_FAIL（-14）
		命令发送失败ERROR_SEND_FAIL (-25)
		命令CRC校验错误ERROR_COM_CS（2）
		命令帧错误ERROR_COM_FH（3）
		命令码无效ERROR_COM_CMD（4）
		命令执行失败ERROR_COM_EXE_FAIL（5）
**************************************************************************************/
int DisplayGetDiskinfo(int chn ,DISPLAY_DISK_INFO *diskInfo);

/*************************************************************************************

Description: 查询显示组合温度
Input: chn -输入参数，显示器序号，0表示下显示器，1表示上显示器
Output:value -输出参数，显示器温度
Return:命令执行正常返回OK_EXECUTE(1) 
		命令执行失败返回错误码：
		参数错误 ERROR_PARAMETER（-22）
		创建消息队列失败ERROR_GET_MSG_ID_FAIL（-14）
		命令发送失败ERROR_SEND_FAIL (-25)
		命令CRC校验错误ERROR_COM_CS（2）
		命令帧错误ERROR_COM_FH（3）
		命令码无效ERROR_COM_CMD（4）
		命令执行失败ERROR_COM_EXE_FAIL（5）

**************************************************************************************/
int DisplayGetTemp(int chn ,float *value);

/*************************************************************************************

Description: 获取显示器5v电压值
Input: chn -输入参数，显示器序号，0表示下显示器，1表示上显示器
Output:value -输出参数，当前显示器5V电压
Return:命令执行正常返回OK_EXECUTE(1) 
		命令执行失败返回错误码：
		参数错误 ERROR_PARAMETER（-22）
		创建消息队列失败ERROR_GET_MSG_ID_FAIL（-14）
		命令发送失败ERROR_SEND_FAIL (-25)
		命令CRC校验错误ERROR_COM_CS（2）
		命令帧错误ERROR_COM_FH（3）
		命令码无效ERROR_COM_CMD（4）
		命令执行失败ERROR_COM_EXE_FAIL（5）

**************************************************************************************/
int DisplayGet5V(int chn ,float *value);

/*************************************************************************************
Description: 设置状态显示屏的亮度
Input: value -输入参数，亮度值 取值范围1-12
Output:无
Return:命令执行正常返回OK_EXECUTE(1) 
		命令执行失败返回错误码：
		参数错误 ERROR_PARAMETER（-22）
		创建消息队列失败ERROR_GET_MSG_ID_FAIL（-14）
		命令发送失败ERROR_SEND_FAIL (-25)
		命令CRC校验错误ERROR_COM_CS（2）
		命令帧错误ERROR_COM_FH（3）
		命令码无效ERROR_COM_CMD（4）
		命令执行失败ERROR_COM_EXE_FAIL（5）

**************************************************************************************/
int StatusScreenBrightCtrl(int value);

/*************************************************************************************

Description: 设置显示模式
Input: value 0,维护模式 1,用户模式
Output:无
Return:命令执行正常返回OK_EXECUTE(1) 
		命令执行失败返回错误码：
		参数错误 ERROR_PARAMETER（-22）
		创建消息队列失败ERROR_GET_MSG_ID_FAIL（-14）
		命令发送失败ERROR_SEND_FAIL (-25)
		命令CRC校验错误ERROR_COM_CS（2）
		命令帧错误ERROR_COM_FH（3）
		命令码无效ERROR_COM_CMD（4）
		命令执行失败ERROR_COM_EXE_FAIL（5）

**************************************************************************************/
int StatusScreenSetMode(int value);

/*************************************************************************************

Description: 加电控制组合的24V电压获取
Input: 无
Output:value -输出参数，24V电压，取值精度0.1v
Return:命令执行正常返回OK_EXECUTE(1) 
		命令执行失败返回错误码：
		参数错误 ERROR_PARAMETER（-22）
		创建消息队列失败ERROR_GET_MSG_ID_FAIL（-14）
		命令发送失败ERROR_SEND_FAIL (-25)
		命令CRC校验错误ERROR_COM_CS（2）
		命令帧错误ERROR_COM_FH（3）
		命令码无效ERROR_COM_CMD（4）
		命令执行失败ERROR_COM_EXE_FAIL（5）

**************************************************************************************/
int PowerCtrlGet24V(float *value);

/*************************************************************************************

Description: 加电控制组合控制断电
Input: value bit0 设备1断电 bit1 设备2断电 bit2 设备3断电 对应位1为断电，0无效
Output:无
Return：命令执行正常返回OK_EXECUTE(1) 
		命令执行失败返回错误码：
		参数错误 ERROR_PARAMETER（-22）
		创建消息队列失败ERROR_GET_MSG_ID_FAIL（-14）
		命令发送失败ERROR_SEND_FAIL (-25)
		命令CRC校验错误ERROR_COM_CS（2）
		命令帧错误ERROR_COM_FH（3）
		命令码无效ERROR_COM_CMD（4）
		命令执行失败ERROR_COM_EXE_FAIL（5）

**************************************************************************************/
int PowerCtrlPowerOff(char value);

/*************************************************************************************

Description:向状态控制板发送主板设备自检结果
Input: chn测试项， 0x01内存 0x02CPU 0x03 电子盘 0x04 外存板 0x05 键盘 0x6 鼠标 0x7 网卡
       value 0x00 正常 0xff 故障
Output:无
Return:	命令执行正常返回OK_EXECUTE(1)
		参数错误 ERROR_PARAMETER（-22）
		创建消息队列失败ERROR_GET_MSG_ID_FAIL（-14）
		命令发送失败ERROR_SEND_FAIL (-25)
**************************************************************************************/
int DevInfoReport(unsigned int chn, unsigned int value);

/*************************************************************************************
Description: 查询模块固件版本
Input: moduleID-模块ID，
            0：状态控制板；
            1：BMC；
            2：加电控制模块；
            3：视频记录模块；
Output:szVer-输出参数，固件版本
Return:命令执行正常返回OK_EXECUTE(1) 
		命令执行失败返回错误码：
		参数错误 ERROR_PARAMETER（-22）
		创建消息队列失败ERROR_GET_MSG_ID_FAIL（-14）
		命令发送失败ERROR_SEND_FAIL (-25)
		命令CRC校验错误ERROR_COM_CS（2）
		命令帧错误ERROR_COM_FH（3）
		命令码无效ERROR_COM_CMD（4）
		命令执行失败ERROR_COM_EXE_FAIL（5）

**************************************************************************************/
int StatusCtrlBoardModuleVer(int moduleID, char *szVer);

/*************************************************************************************
Description: 查询状态控制板驱动软件版本
Input:无
Output:szVer-输出参数，驱动软件版本
Return:命令执行正常返回OK_EXECUTE(1)
        命令执行失败返回错误码：
        参数错误 ERROR_PARAMETER（-22）
**************************************************************************************/
int StatusCtrlBoardSWVer(char *szVer);

/*************************************************************************************
Description: 卸载状态控制板驱动
Input:无
Output:无
Return:	执行正常返回OK_EXECUTE(1) 
		执行失败返回错误码：
		线程取消失败返回ERROR_THREAD_CANCEL_FAIL（-23）
		线程资源释放失败返回ERROR_THREAD_JOIN_FAIL（-24）
**************************************************************************************/
int StatusCtrlBoardUnLoad(void);

#ifdef __cplusplus
};
#endif 
#endif // STATUSCTRLLIB_GLOBAL_H
