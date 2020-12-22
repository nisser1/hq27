#ifndef API_SUPPORT_INTERFACE
#define API_SUPPORT_INTERFACE

#define FLASH_SIZE_SH		"fdisk -l | grep \"Disk /dev/sda\""
#define FLASH1_SIZE_SH		"fdisk -l | grep \"Disk /dev/sdb\""
#define FLASH_OCCUPY_SH		"df -m"
#define CPU_SPEED_SH		"dmidecode | grep \"Max Speed\""
#define KERNEL_SH			"cat /proc/version"
#define OS_SH				"cat /etc/issue"
#define PROCESS_SH			"ps -ef | wc -l"

typedef struct cpu_occupy_          //定义一个cpu occupy的结构体
{
    char name[20];                  //定义一个char类型的数组名name有20个元素
    unsigned int user;              //定义一个无符号的int类型的user
    unsigned int nice;              //定义一个无符号的int类型的nice
    unsigned int system;            //定义一个无符号的int类型的system
    unsigned int idle;              //定义一个无符号的int类型的idle
    unsigned int iowait;
    unsigned int irq;
    unsigned int softirq;
}cpu_occupy_t;


typedef struct mem_occupy_
{
	unsigned int memtotal;
	unsigned int memfree;
	unsigned int buffers;
	unsigned int cached;
}mem_occupy_t;

typedef struct {
	char compare_and[64];
	char compare_xor[64];
	char compare_sub[64];
	char compare_mul[64];
	char compare_dvi[64];

}MEMOffline;

typedef struct {
	char cpu0_info[128];
	char cpu1_info[128];
	char cpu2_info[128];
	char cpu3_info[128];
}CPUOffline;
typedef struct {
	unsigned long read_avg_speed;
	unsigned long write_avg_speed;
	unsigned long reread_avg_speed;
	unsigned long rewrite_avg_speed;
}DISKOffline;
//cpu信息获取
double cal_cpuoccupy (cpu_occupy_t *o, cpu_occupy_t *n);
void get_cpuoccupy (int cpu_index, cpu_occupy_t *cpust);
int get_cpufrequency(int *frequency);
//int cpu_load(cpu_occupy_t *t);
//int get_cpunum(int cpunum);
//内存信息获取
int get_meminfo(int mode, mem_occupy_t *pmem_occupy);
//网卡信息获取
int net_detect(const char* net_name);
int get_netcard(char *buf_info);
//执行脚本命令
int runapi_sh(char *sh,char *p_str,int size);
//获取存储器大小
int get_flashapi_size(int *size,int *freesize);
//获取存储器占用比率
int get_flashapi_occupy(int *occupy);
//获取内核版本
//int get_kernel_version(char *p_str);
////获取操作系统版本
//int get_os_version(char *p_str);
////获取进程总数
//int get_process_total(int *total);
//获取离线测试信息
int read_memtest_result(MEMOffline *pMEMOffline);
//获取离线cpu测试信息
int read_cpuinfo(CPUOffline *pMEMOffline);
//获取硬盘离线测试数据
int get_diskoffline(DISKOffline *pDISKOffline);
//将字符串中的数字分离出来
int get_str_num(char *str,int *num1);
#endif // CPU_USAGE_H
