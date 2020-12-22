#ifndef BIT_H
#define BIT_H
#include "statusctrllib_global.h"
#include "msg.h"
#define CPU_NUM	4						//cpu数量判定标准
#define CPUS_USAGE_NUM CPU_NUM + 1		//CPU序号
#define NETCARD_NUM	5					//网卡参照数量
#define ERROR_PRAR 				0x0800	//参数错误
#define ERROR_CPUNUM			0x0801	//cpu数量异常
#define ERROR_THREAD			0x0803	//创建离线测试线程失败
#define ERROR_SYSTEMCMD 		0x0804	//system执行系统命令异常
#define ERROR_OPENDIR   		0x0805	//打开文件错误
#define ERROR_SERIAL   			0x0809	//打开串口失败



//返回值类型
typedef int STATUS;

typedef struct {
	char devname[32];           //设备名
	int devState;               //设备状态，1:故障，0:正常
	unsigned int errCode[256];  //设备故障码
	char errInfo[256];          //设备故障描述信息
	char basicInfo[256];        //设备基本属性描述信息
	char perfInfo[256];         //设备性能参数描述信息
} DEVBITINFO;

typedef struct {
	int cpu_frequency;							//在线测试cpu频率
    unsigned int available_cpu_num;				//在线测试可用cpu数量   当available_cpu_num为0xff代表available_cpu_num != CPU_NUM
    double cpus_usage[CPUS_USAGE_NUM];			//在线测试cpu平均利用率
	int cpu_usage_warning;						//1表示占用率异常     0表示占用率正常
    char cpu_test_info[2048];					//离线测试cpu离线测试结果
} CPUInfo;

typedef struct {
    unsigned int mem_total;	//在线测试内存总量					
	unsigned int mem_free;	//在线测试内存剩余空间
	unsigned int mem_used;	//在线测试内存已用空间
    int mem_used_perc;		//内存占用百分比
	int mem_usage_warning;	//1表示占用率异常    0表示占用率正常
	char compare_and[64];	//离线测试在线与操作
	char compare_xor[64];	//离线测试在线异或操作		
	char compare_sub[64];	//离线测试在线减法操作
	char compare_mul[64];	//离线测试在线乘除操作
	char compare_dvi[64];	//离线测试内存除操作
} MEMInfo;

typedef struct {
    //存放网卡名字   例如有四个网卡字符串的格式是 eth0/eth1/eth2/eth3
    char netinfo[128];							//查询到的网卡名字	
	int network_status;							//1表示存在网卡不在位    0表示网卡都在位
    unsigned int speed[NETCARD_NUM];			//speed[0] 对应eth0对应网卡速度  speed[1] 对应eth1对应网卡速度,以此类推
	unsigned int net_stats[NETCARD_NUM];		//同上     0为未连接  1为连接			/*网卡连接状态 4*/
}NETInfo;

typedef struct{
    int flash_size;					//在线测试硬盘总大小
    int flash_free_size;			//在线测试硬盘空闲空间大小
    float disk_usage;				//在线测试硬盘占用率
	int disk_usage_waring;			//1表示磁盘占用率异常    0表示磁盘占用率正常
	unsigned long read_avg_speed;	//离线测试硬盘读速度
	unsigned long write_avg_speed;	//离线测试硬盘读速度
	unsigned long reread_avg_speed;	//离线测试硬盘重复写速度
	unsigned long rewrite_avg_speed;//离线测试硬盘重复读速度
}DISKInfo;

typedef struct
{
    int stats;									//1表示显卡不在位      0表示显卡在位
    int fps;									//显卡测试帧率
} DisPlayInfo;

typedef struct
{
	//在线测试主板BMC数据
	MainBoardSensorData main_board_sensor_data;	
	//在线测试电源板BMC数据	
    PowerBoardSensorData power_board_sensor_data;
	//在线测试存储板BMC数据	
    StoreBoardSensorData store_board_sensor_data;		
	//1代表警告 0代表正常 0-9位分别代表主板12V、5V、3.3V、1.8V、1.5V、1.05V、1.0V、0.95V、0.9V、0.75V;10-12位代表电源板12V、5V、3.3V;13位代表存储板5V电压;14位代表主板温度;15位代表cpu温度;16位代表电源板12V电压的电流;//17位代表电源板温度；18位代表存储板5V电压的电流；19位代表存储板温度。
	unsigned int bmc_warning;    
	//1代表警告 0代表正常 0-9位分别代表主板12V、5V、3.3V、1.8V、1.5V、1.05V、1.0V、0.95V、0.9V、0.75V; //10-12位代表电源板12V、5V、3.3V;13位代表存储板5V电压						   
    unsigned int bmc_error;
    int serial_flags;    //1 表示未收到串口数据   0可以正常收到
}BMC_Info;
typedef struct
{
    float master_tmp;		//下显示器温度 	
    float assistant_tmp;	//上显示器温度
	float master_5v;		//下显示器5V电压 	
    float assistant_5v;	//上显示器5V电压
	int master_stat;	//下显示器状态 0正常 1表示异常  	
    int assistant_stat;  //下显示器状态0正常  1异常 		
}MonitorInfo;


/*CPU测试维护
 * 功能	在线测试:获取cpu个数 、获取CPU频率、计算cpu使用率，如果CPU占用率过高cpu_usage_warning会被置为 1
 * 离线测试: 测试浮点数操作的速度和效率,测试包扩几个模块,每个模块都包括一组用于科学计算的操作,
 *       	 测试共分两次 一次是单进程测试,另一次是N份测试,N等于CPU的个数,
 *			 测试结果与一个基线系统测试结果比较得到的指数值，比如280则代表是基线处理速度的28倍
 *      	 MWIPS表示运算能力例如 6344.1 MWIPS 代表1s钟运行6344.1 1024*1024条whetsone指令（浮点运算指令）
 * 参数1 mode 0：离线测试并返回测试结果 
 *			  1：在线测试并返回结果
 * 参数2 pCPUInfo 返回的测试结果（在线和离线共用一个结构，详细描述见结构详细注释）
 * 返回值 函数正常结束返回0 函数异常返回异常码
 */
STATUS ML_MTMW_CPUBIT(int mode, CPUInfo* pCPUInfo);												

//内存测试维护	
/* 功能
 * 在线测试	：获取内存占有率、内存容量、内存已占用空间、内存未占用空间  单位MB
 *				如果内存占用率过高（>80%）mem_usage_warning会被置为1
 * 离线测试  在内存做一些加减乘除异或操作
 * 参数1 mode 0：离线测试并返回测试结果 
 *			  1: 在线测试并返回结果
 * 参数2 pCPUInfo 返回的测试结果（在线和离线共用一个结构，详细描述见结构详细注释）
 * 返回值 获取成功返回0 获取失败返回错误码 
 */			
STATUS ML_MTMW_MEMBIT(int mode, MEMInfo* pMEMInfo);												

//网卡测试维护				
/* 功能	
 * 在线：1:获取网卡列表，例如四个网卡分别为eth0-3 再加lo则网卡列表为 eth0/eth1/eth2/eth3/lo	
 *		  2:返回网卡链接状态
 *		  3：返回网卡当前匹配速度						
 * 		  4：注意eth0-eth4对应了四个网卡加一个本地网卡，链接状态和匹配速度则是按照获取到的网卡列表字符串顺序一一对应的	
 * 离线：调用网络测试程序。											
 * 参数1 mode 0：   离线测试并返回测试结果    1：在线测试并返回结果
 * 参数2 pMEMInfo： 在线测试返回的测试结果
 * 返回值 获取成功返回0 获取失败返回错误码 
 */
STATUS ML_MTMW_NETCARDBIT(int mode, NETInfo *pNETInfo);											

/*显示接口维护测试接口 	
 * 功能：在线测试： 测试显卡在位状态，在位stats为1否则为0
 *	  	 离线测试： 此功能为耗时操作，测试调用测试界面，并在界面退出时返回平均帧率
 * 参数：mode 测试类型 0 离线  1在线        pDisPlayInfo测试结果
 * 返回值正常返回 0 异常返回异常码	
*/			
STATUS ML_MTMW_DISPLAYBIT(int mode,DisPlayInfo *pDisPlayInfo);								

//电子盘维护测试接口
/* 功能	获取DISK相关信息
 *	在线测试 获取磁盘总大小、剩余空间、已用空间以及使用率，如果使用率大于80% disk_usage_waring会被置为1
 *  离线测试 对磁盘进行读写操作
 * 参数1 mode 0离线测试  1在线测试
 * 参数2 pDISKInfo 返回的测试结果
 * 返回值 获取成功返回0 获取失败返回错误码 
 */			
STATUS ML_MTMW_DISKBIT(int mode,DISKInfo *pDISKInfo);										

//外存储板容量检测
/*功能：
 * 在线测试 获取磁盘总大小、剩余空间、已用空间以及使用率，如果使用率大于80% disk_usage_waring会被置为1
 * 离线测试 对磁盘进行读写操作
 * 参数1 mode 0离线测试  1在线测试
 * 参数2 pDISKInfo 返回的测试结果
 * 返回值 获取成功返回0 获取失败返回错误码 
*/
STATUS ML_MTMW_DISKBOARDBIT(int mode,DISKInfo *pDISKInfo);										



int cal_cpuoccupy1(CPU_OCCUPY *o, CPU_OCCUPY *n);
void get_cpuoccupy1(CPU_OCCUPY *cpust);



#endif

