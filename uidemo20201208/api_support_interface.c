#include <stdio.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <string.h>
#include <stdlib.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include "api_support_interface.h"
#include <time.h>
#include <sys/statfs.h>
#include "bit.h"
/*******************************************************************/
/**************************获取cpu信息begin*************************/
/*******************************************************************/
double cal_cpuoccupy (cpu_occupy_t *o, cpu_occupy_t *n)
{
    double od, nd;
    double id, sd;
    double cpu_use ;

    od = (double) (o->user + o->nice + o->system +o->idle+o->softirq+o->iowait+o->irq);//第一次(用户+优先级+系统+空闲)的时间再赋给od
    nd = (double) (n->user + n->nice + n->system +n->idle+n->softirq+n->iowait+n->irq);//第二次(用户+优先级+系统+空闲)的时间再赋给od

    id = (double) (n->idle);    //用户第一次和第二次的时间之差再赋给id
    sd = (double) (o->idle) ;    //系统第一次和第二次的时间之差再赋给sd
    if((nd-od) != 0)
        cpu_use =100.0 - ((id-sd))/(nd-od)*100.00; //((用户+系统)乖100)除(第一次和第二次的时间差)再赋给g_cpu_used
        //printf("cpu_user - sss333333333333333333333 cpu_use\n",cpu_use);
    else
        cpu_use = 0;
    return cpu_use;
}

void get_cpuoccupy (int cpu_index, cpu_occupy_t *cpust)
{
    FILE *fd;
    char buff[256];
    cpu_occupy_t *cpu_occupy;
    cpu_occupy=cpust;

    fd = fopen ("/proc/stat", "r");
    if(fd == NULL)
    {
            perror("fopen:");
            exit (0);
    }

    int i = 0;
    for(i = 0; i <= cpu_index; ++i)
        fgets(buff, sizeof(buff), fd);

    sscanf (buff, "%s %u %u %u %u %u %u %u", cpu_occupy->name, &cpu_occupy->user, &cpu_occupy->nice,&cpu_occupy->system, &cpu_occupy->idle ,&cpu_occupy->iowait,&cpu_occupy->irq,&cpu_occupy->softirq);

    fclose(fd);
}

int get_cpufrequency(int *frequency)
{

    *frequency =0;
    float i = 0;
    FILE *fp;
    char buf[256] = {};
    char buf1[32] = {0};
    memset(buf,0,sizeof(buf));
    int ret = system("lscpu > ./manage_api/cpufrequency.txt");
    if(ret == -1)
    {

        return -2;
    }
    fp = fopen("./manage_api/cpufrequency.txt","r");
    if(fp == NULL)
    {
        perror("fopen:");
        return -1;
    }
    while(fgets(buf,sizeof(buf),fp))
    {


        if(strstr(buf,"CPU max MHz:") != NULL){
            get_str_num(buf,frequency);
        }
    }
    fclose(fp);
	return 0;
}
//切割字符串
int get_str_num(char *str,int *num1)
{
    int ret = 0;
    int i=0,j=0;
    char ptr_num1[64] = {};
    memset(ptr_num1,0,sizeof(ptr_num1));

    while(str[i] != '\0')
    {
        j=0;
        if((str[i] < 58) && (str[i] > 45))
        {
            if(str[i] == '/'){
                continue;
            }
            while(ptr_num1[j] != '\0')
            {
                j++;
            }
            ptr_num1[j] = str[i];
            ptr_num1[j+1] = '\0';
           if((str[i+1] > 58) || (str[i+1] < 45) || (str[i] == '/'))
           {
               ret = atoi(ptr_num1);
                ptr_num1[0] = '\0';
           }
        }
        i++;
    }
    *num1 = ret;
    return 0;
}
/**************************获取cpu信息end***************************/
/**************************获取内存信息begin************************/
//返回值　－１打开文件失败  返回０测试获取信息成功
int get_meminfo(int mode, mem_occupy_t *pmem_occupy)
{ 
	FILE *fp;
	char buf[128] = {0};
	char temp[64];
	int i = 0;
	unsigned int a;

	fp = fopen("/proc/meminfo","r");
	if(fp == NULL)
	{
		perror("fopen:");
        return -1;
	}
	while(fgets(buf,sizeof(buf),fp))
	{
		if(strstr(buf,"MemTotal:")!= NULL)
		{
			sscanf(buf,"%s %u",temp,&(pmem_occupy->memtotal));
            //printf("pmem_occupy->memtotal=%u\n",pmem_occupy->memtotal);
		}
		if(strstr(buf,"MemFree:")!= NULL)
		{
			sscanf(buf,"%s %u",temp,&(pmem_occupy->memfree));
            //printf("pmem_occupy->memfree=%u\n",pmem_occupy->memfree);
		}
		if(strstr(buf,"Buffers:")!= NULL)
		{
			sscanf(buf,"%s %u",temp,&(pmem_occupy->buffers));
            //printf("pmem_occupy->buffers=%u\n",pmem_occupy->buffers);
		}
		if((strstr(buf,"Cached:")!= NULL) && (strstr(buf,"SwapCached:"))==NULL)
		{
			sscanf(buf,"%s %u",temp,&(pmem_occupy->cached));
            //printf("pmem_occupy->cached=%u\n",pmem_occupy->cached);
		}
#if 0
        //printf("%s\n",temp);
        //printf("%u\n",*(&(pmem_occupy->memtotal)+4*i));
#endif 

	}
	fclose(fp);
    return 0;
}

/**************************获取内存信息end**************************/
/**************************获取网卡信息begin**************************/
int net_detect(const char* net_name)
{
	int skfd = 0;
	struct ifreq ifr;
	skfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (skfd < 0)
	{
		return -1;
	}
	/* 传入网络接口名称，获得标志 */
	strcpy(ifr.ifr_name, net_name);
	if (ioctl(skfd, SIOCGIFFLAGS, &ifr) < 0)
	{
		close(skfd);
		return -1;
	}

	if (ifr.ifr_flags & IFF_RUNNING)
	{
		close(skfd);
		return 0;
	}
	else
	{
		close(skfd);
		return 1;
	}
    return -1;
}
int get_netcard(char *buf_info)
{
	char tmpbuf[32]="";
	strcpy(buf_info,"");
	int fd;
	int interfaceNum = 0;
	struct ifreq buf[16];
	struct ifconf ifc;
	struct ifreq ifrcopy;
	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		perror("socket");
		close(fd);
	 	return -1;
	 }
	ifc.ifc_len = sizeof(buf);
	ifc.ifc_buf = (caddr_t)buf;
	if (!ioctl(fd, SIOCGIFCONF, (char *)&ifc))
	{
		interfaceNum = ifc.ifc_len / sizeof(struct ifreq);
	}
	strcat(buf_info,"-/-");	
	while (interfaceNum-- > 0)
	{
		sprintf(tmpbuf,"%s", buf[interfaceNum].ifr_name);
		strcat(buf_info,tmpbuf);	
		strcat(buf_info,"-/+");	
		
	}
    //printf("%s\n",buf_info);

	return 0;
}

#if 0
//static struct ipmi_intf * ipmi_serial_intf = NULL;

struct cpu_occupt_pth_arg_t{
	char *sh;
	int  occupy;
}cpu_occupy_pth_arg;
#endif

/*
 * 功能: 运行shell命令，返回运行结果
 * 参数: sh,shell命令
 * 参数: p_str 存储运行结果的指针
 * 参数: size,存储区大小
 * 返回: 成功返回0
*/
int runapi_sh(char *sh,char *p_str,int size)
{
    FILE *stream = NULL;
	int len = 0;
    int ret = 0;
    stream = popen(sh,"r");
    if (NULL == stream) {
		ret = -1;
        goto point_return;
    }
    len = fread(p_str,1,size,stream);
    if (0 >= len) {
		ret = -2;
        goto point_release;
    }
point_release:
    pclose(stream);
point_return:
    return ret;
}

//
int get_flashapi_size(int *size , int *freesize/*int flash_num,int *bmb*/)
{
    int allsize = 0;
    int allfreesize = 0;
    struct statfs diskInfo;
    statfs("/", &diskInfo);
    unsigned long long blocksize = diskInfo.f_bsize;	//每个block里包含的字节数
    unsigned long long totalsize = blocksize * diskInfo.f_blocks; 	//总的字节数，f_blocks为block的数目
//    printf("Total_size = %llu B = %llu KB = %llu MB = %llu GB\n",
//    totalsize, totalsize>>10, totalsize>>20, totalsize>>30);
    unsigned long long freeDisk = diskInfo.f_bfree * blocksize;	//剩余空间的大小
    unsigned long long availableDisk = diskInfo.f_bavail * blocksize; 	//可用空间大小
//    printf("Disk_free = %llu MB = %llu GB\nDisk_available = %llu MB = %llu GB\n",
//    freeDisk>>20, freeDisk>>30, availableDisk>>20, availableDisk>>30);
    int gensize = totalsize >> 30;
    int genfreesize = freeDisk >>30;
    gensize += 1;
    genfreesize += 1;
   printf("\n aa %d %d\n",gensize,genfreesize);
/*
    statfs("/dev", &diskInfo);
    blocksize = diskInfo.f_bsize;	//每个block里包含的字节数
    totalsize = blocksize * diskInfo.f_blocks; 	//总的字节数，f_blocks为block的数目
//    printf("Total_size = %llu B = %llu KB = %llu MB = %llu GB\n",
//    totalsize, totalsize>>10, totalsize>>20, totalsize>>30);
    freeDisk = diskInfo.f_bfree * blocksize;	//剩余空间的大小
    availableDisk = diskInfo.f_bavail * blocksize; 	//可用空间大小
//    printf("Disk_free = %llu MB = %llu GB\nDisk_available = %llu MB = %llu GB\n",
//    freeDisk>>20, freeDisk>>30, availableDisk>>20, availableDisk>>30);
    int udev = totalsize >> 30;
    int udevfree = freeDisk >>30 ;
    udev += 1;
    udevfree += 1;
  //  printf("%d %d\n",udev,udevfree);



    statfs("/run", &diskInfo);
    blocksize = diskInfo.f_bsize;	//每个block里包含的字节数
    totalsize = blocksize * diskInfo.f_blocks; 	//总的字节数，f_blocks为block的数目
//    printf("Total_size = %llu B = %llu KB = %llu MB = %llu GB\n",
//    totalsize, totalsize>>10, totalsize>>20, totalsize>>30);
    freeDisk = diskInfo.f_bfree * blocksize;	//剩余空间的大小
    availableDisk = diskInfo.f_bavail * blocksize; 	//可用空间大小
//    printf("Disk_free = %llu MB = %llu GB\nDisk_available = %llu MB = %llu GB\n",
//    freeDisk>>20, freeDisk>>30, availableDisk>>20, availableDisk>>30);
    int run = totalsize >> 30;
    int runfree = freeDisk >>30 ;
    run += 1;
    runfree += 1;
   // printf("%d %d\n",run,runfree);


    statfs("/dev/shm", &diskInfo);
    blocksize = diskInfo.f_bsize;	//每个block里包含的字节数
    totalsize = blocksize * diskInfo.f_blocks; 	//总的字节数，f_blocks为block的数目
//    printf("Total_size = %llu B = %llu KB = %llu MB = %llu GB\n",
//    totalsize, totalsize>>10, totalsize>>20, totalsize>>30);
    freeDisk = diskInfo.f_bfree * blocksize;	//剩余空间的大小
    availableDisk = diskInfo.f_bavail * blocksize; 	//可用空间大小
//    printf("Disk_free = %llu MB = %llu GB\nDisk_available = %llu MB = %llu GB\n",
//    freeDisk>>20, freeDisk>>30, availableDisk>>20, availableDisk>>30);
    int shm = totalsize >> 30;
    int shmfree = freeDisk >>30 ;
    shm += 1;
    shmfree += 1;
  // printf("%d %d\n",shm,shmfree);


    statfs("/run/lock", &diskInfo);
    blocksize = diskInfo.f_bsize;	//每个block里包含的字节数
    totalsize = blocksize * diskInfo.f_blocks; 	//总的字节数，f_blocks为block的数目
//    printf("Total_size = %llu B = %llu KB = %llu MB = %llu GB\n",
//    totalsize, totalsize>>10, totalsize>>20, totalsize>>30);
    freeDisk = diskInfo.f_bfree * blocksize;	//剩余空间的大小
    availableDisk = diskInfo.f_bavail * blocksize; 	//可用空间大小
//    printf("Disk_free = %llu MB = %llu GB\nDisk_available = %llu MB = %llu GB\n",
//    freeDisk>>20, freeDisk>>30, availableDisk>>20, availableDisk>>30);
    int lock = totalsize >> 30;
    int lockfree = freeDisk >>30 ;
    lock += 1;
    lockfree += 1;
   // printf("%d %d\n",lock,lockfree);


    statfs("/sys/fs/cgroup", &diskInfo);
    blocksize = diskInfo.f_bsize;	//每个block里包含的字节数
    totalsize = blocksize * diskInfo.f_blocks; 	//总的字节数，f_blocks为block的数目
//    printf("Total_size = %llu B = %llu KB = %llu MB = %llu GB\n",
//    totalsize, totalsize>>10, totalsize>>20, totalsize>>30);
    freeDisk = diskInfo.f_bfree * blocksize;	//剩余空间的大小
    availableDisk = diskInfo.f_bavail * blocksize; 	//可用空间大小
//    printf("Disk_free = %llu MB = %llu GB\nDisk_available = %llu MB = %llu GB\n",
//    freeDisk>>20, freeDisk>>30, availableDisk>>20, availableDisk>>30);
    int cgroup = totalsize >> 30;
    int cgroupfree = freeDisk >>30 ;
    cgroup += 1;
    cgroupfree += 1;
   // printf("%d %d\n",cgroup,cgroupfree);



    statfs("/boot", &diskInfo);
    blocksize = diskInfo.f_bsize;	//每个block里包含的字节数
    totalsize = blocksize * diskInfo.f_blocks; 	//总的字节数，f_blocks为block的数目
//    printf("Total_size = %llu B = %llu KB = %llu MB = %llu GB\n",
//    totalsize, totalsize>>10, totalsize>>20, totalsize>>30);
    freeDisk = diskInfo.f_bfree * blocksize;	//剩余空间的大小
    availableDisk = diskInfo.f_bavail * blocksize; 	//可用空间大小
//    printf("Disk_free = %llu MB = %llu GB\nDisk_available = %llu MB = %llu GB\n",
//    freeDisk>>20, freeDisk>>30, availableDisk>>20, availableDisk>>30);
    int boot = totalsize >> 30;
    int bootfree = freeDisk >>30 ;
    boot += 1;
    bootfree += 1;
   // printf("%d %d\n",boot,bootfree);




    statfs("/run/user/0", &diskInfo);
    blocksize = diskInfo.f_bsize;	//每个block里包含的字节数
    totalsize = blocksize * diskInfo.f_blocks; 	//总的字节数，f_blocks为block的数目
  //  printf("Total_size = %llu B = %llu KB = %llu MB = %llu GB\n",
   // totalsize, totalsize>>10, totalsize>>20, totalsize>>30);
    freeDisk = diskInfo.f_bfree * blocksize;	//剩余空间的大小
    availableDisk = diskInfo.f_bavail * blocksize; 	//可用空间大小
  //  printf("Disk_free = %llu MB = %llu GB\nDisk_available = %llu MB = %llu GB\n",
  //  freeDisk>>20, freeDisk>>30, availableDisk>>20, availableDisk>>30);
    int user = totalsize >> 30;
    int userfree = freeDisk >>30 ;
    user += 1;
    userfree += 1;
   // printf("%d %d\n",user,userfree);

*/
    allsize = gensize;// + udev + run + shm + lock + cgroup + boot + user;
    allfreesize =  genfreesize+1;
    //printf("sdfdfdsfds   %d %d\n",allsize,allfreesize);
    *size = allsize;
   *freesize = allfreesize;
//	char *sh0 = FLASH_SIZE_SH;
//	char *sh1 = FLASH1_SIZE_SH;
//	char sh[32] = {};
//	memset(sh,0,32);
//	if(flash_num == 0){
//		strcpy(sh,sh0);
//	}else{
//		strcpy(sh,sh1);
//	}
//	char buff[200] = "";
//    if (0 != runapi_sh(sh,buff,sizeof(buff))) {
//		return -1;
//	}
//	sscanf(buff,"%*[^:]: %d",bmb);
//	*bmb *= 10;
    return 0;
}

/*
 * 功能: 获取存储器占用比率
*/
int get_flashapi_occupy(int *occupy)
{
	char *sh = FLASH_OCCUPY_SH;
	char buff[1500] = "";
	char temp[1500] = "";
	char *p_str = NULL;
	int sum_used = 0,sum_block = 0;
    if (0 != runapi_sh(sh,buff,sizeof(buff))) {
		return -1;
	}
	while(NULL != (p_str = strchr(buff,'\n'))){
		if (1 >= strlen(p_str))
			break;
		strcpy(temp,p_str+1);
		strtok(p_str+1," ");
		p_str =strtok(NULL," ");
		sum_block += atoi(p_str);	
		p_str =strtok(NULL," ");
		sum_used += atoi(p_str);	
		strcpy(buff,temp);
	}
	*occupy = (int)((float)sum_used/sum_block*100+0.5);

	return 0;
}

//int get_kernel_version(char *p_str)
//{
//	char *sh = KERNEL_SH;
//	char buff[300] = "";

//	if (0 != run_sh(sh,buff,sizeof(buff))) {
//		return -1;
//	}
//	sscanf(buff,"Linux version %[^-]",p_str);
//	return 0;
//}

/*
 * 功能: 获取系统版本
*/
//int get_os_version(char *p_str)
//{
//	char *sh = OS_SH;
//	char buff[300] = "";
//	if (0 != run_sh(sh,buff,sizeof(buff))) {
//		return -1;
//	}
//	sscanf(buff,"%[^\\]",p_str);
//	return 0;
//}

/*
 * 功能: 获取进程数量
 * 参数: total,存放结果
*/
//int get_process_total(int *total)
//{
//	char *sh = PROCESS_SH;
//	char buff[100] = "";
//	if (0 != run_sh(sh,buff,sizeof(buff))) {
//		return -1;
//	}
//	*total = atoi(buff);
//	return 0;
//}
int read_memtest_result(MEMOffline *pMEMOffline)
{
	FILE *fp;
	char buf[128] = {0};
	char bufmessage[256] = {};
	memset(buf,0,sizeof(buf));
	getcwd(buf,sizeof(buf));
	strcat(buf,"./manage_api/memtester.txt");
	fp = fopen("./manage_api/memtester.txt","r");
	if(fp == NULL)
	{
		perror("fopen:");
        return -1;
	}
	while(fgets(bufmessage,sizeof(bufmessage),fp))
	{
        if(strstr(bufmessage,"Compare AND") != NULL){
			if(strstr(bufmessage,"ok") != NULL)	{
				strcpy(pMEMOffline->compare_and,"Compare AND ok");
				
			}
			else if(strstr(bufmessage,"error") != NULL){
				strcpy(pMEMOffline->compare_and,"Compare AND error");
				
			}
		}
		if(strstr(bufmessage,"Compare XOR") != NULL){
			if(strstr(bufmessage,"ok") != NULL)	{
				strcpy(pMEMOffline->compare_xor,"Compare XOR ok");
				
			}
			else if(strstr(bufmessage,"error") != NULL){
				strcpy(pMEMOffline->compare_xor,"Compare XOR error");
				
			}
		}
		if(strstr(bufmessage,"Compare SUB") != NULL){
			if(strstr(bufmessage,"ok") != NULL)	{
				strcpy(pMEMOffline->compare_sub,"Compare SUB ok");
				
			}
			else if(strstr(bufmessage,"error") != NULL){
				strcpy(pMEMOffline->compare_sub,"Compare SUB error");
				
			}
		}

		if(strstr(bufmessage,"Compare MUL") != NULL){
			if(strstr(bufmessage,"ok") != NULL)	{
				strcpy(pMEMOffline->compare_mul,"Compare MUL ok");
				
			}
			else if(strstr(bufmessage,"error") != NULL){
				strcpy(pMEMOffline->compare_mul,"Compare MUL error");
				
			}
		}

		if(strstr(bufmessage,"Compare DIV") != NULL){
			if(strstr(bufmessage,"ok") != NULL)	{
				strcpy(pMEMOffline->compare_dvi,"Compare DIV ok");
				
			}
			else if(strstr(bufmessage,"error") != NULL){
				strcpy(pMEMOffline->compare_dvi,"Compare DIV error");
				
			}
		}


	}
    fclose(fp);
    return 0;
}

int read_cpuinfo(CPUOffline *pMEMOffline)
{/*
	char buf[256];
	char bufcmd[256];
	char bufmessage[256] = "";
	memset(buf,0,sizeof(buf));
	memset(bufcmd,0,sizeof(bufcmd));
	memset(bufmessage,0,sizeof(bufmessage));
	struct timeval tv;
	struct tm tm;
	gettimeofday(&tv,NULL);
	localtime_r(&tv.tv_sec,&tm);
    strcpy(bufcmd,"/home/ht706/health-manage/cpu_monitor/");
	sprintf(buf,"%04d-%02d-%02d",tm.tm_year+1900,tm.tm_mon+1,tm.tm_mday);
	strcpy(bufcmd,"node2-");
	strcat(bufcmd,buf);
	strcat(bufcmd,"-01");
	printf("bufcmd = %s\n",bufcmd);
	
	FILE *fp = fopen(bufcmd,"r");
	if(fp == NULL)
	{
		perror("fopen:");
		exit(0);
	}	

	while(fgets(bufmessage,sizeof(bufmessage),fp))
	{
		if(strstr(bufmessage,"CPU 0") != NULL)
		{
			strcpy(pMEMOffline->cpu0_info,bufmessage);
		}
		if(strstr(bufmessage,"CPU 1") != NULL)
		{
			memset(pMEMOffline->cpu1_info,0,sizeof(pMEMOffline->cpu1_info));
			//strcpy(pMEMOffline->cpu1_info,bufmessage);
			strcpy(pMEMOffline->cpu1_info,"   CPU 1: phytium FT1500a (3594.2 bogomips)\n\0");
		}
		if(strstr(bufmessage,"CPU 2") != NULL)
		{
			strcpy(pMEMOffline->cpu2_info,bufmessage);
		}
		if(strstr(bufmessage,"CPU 3") != NULL)
		{
			strcpy(pMEMOffline->cpu3_info,bufmessage);
		}

	}
    fclose(fp);*/
	return 0;
}

int get_diskoffline(DISKOffline *pDISKOffline){
	
	unsigned long  a,b;
	int i = 0;
	unsigned long long tmpread = 0,tmpreread = 0;
	unsigned long read = 0,reread = 0;
	unsigned long long  tmpwrite = 0,tmprewrite = 0;
	unsigned long  write = 0,rewrite = 0;	
	FILE *fp;
	char buf[128] = {0};
	char bufmessage[256] = {};
	memset(buf,0,sizeof(buf));
	getcwd(buf,sizeof(buf));
	strcat(buf,"./manage_api/disktest.txt");
	fp = fopen("./manage_api/disktest.txt","r");
	if(fp == NULL)
	{
		perror("fopen:");
        return -1;
	}
	while(fgets(bufmessage,sizeof(bufmessage),fp)){	
		if(strstr(bufmessage,"reclen") && strstr(bufmessage,"rewrite")&&strstr(bufmessage,"reread")&&strstr(bufmessage,"frewrite")){
			break;	
		}
	}
	while(fgets(bufmessage,sizeof(bufmessage),fp)){		
			sscanf (bufmessage, "%lu %lu %lu %lu %lu %lu", &a,&b,&read,&reread,&write,&rewrite);	
			i++ ;
			tmpread = tmpread +read;	
			tmpreread = tmpreread + reread;
			tmpwrite = tmpwrite + write;
			tmprewrite = tmprewrite + rewrite;	
			printf("*      dd %lu %lu %lu %lu %lu %lu\n", a,b,read,reread,write,rewrite);
//			printf("\n");
//		//}
	}
    i--;
    //printf("i = %d\n",i);
    printf("%lu  %lu  %lu  %lu \n", (tmpread/i),(tmpreread/i),(tmpwrite/i),(tmprewrite/i));
    pDISKOffline->read_avg_speed = tmpread/i;
    pDISKOffline->reread_avg_speed = tmpreread/i;
    pDISKOffline->write_avg_speed = tmpwrite/i;
    pDISKOffline->rewrite_avg_speed = tmprewrite/i;
    fclose(fp);
    return 0;
	
}
//int cpu_load(cpu_occupy_t *t)
//{
//	float a;
//	int i = 0;
//	FILE *fp;
//	char buf[256] = {};
//	char tmpbuf[32] = "";
//	memset(buf,0,sizeof(buf));
//	int ret = system("uptime > cpu_load.txt");
//    if(ret == -1)
//	{
//        return -2;
//	}
//	fp = fopen("cpu_load.txt","r");
//	if(fp == NULL)
//	{
//		perror("fopen:");
//        return -1;
//	}
//	char str[128] = "";
//	fgets(str,sizeof(str),fp);
//	char ss[32] = "";
//	char ss1[32] = "";
//	char ss2[32] = "";
//	char ss3[32] = "";
//		char *ptr;
//		char *p;
//		ptr = strtok(str, ":");
//		while (ptr != NULL) {
//			strcpy(ss,ptr);
//			ptr = strtok(NULL, ":");
//		}
//	//printf("ptr=%s\n", ss);
//	ptr = strtok(ss, ",");
//	t->min1_load = atof(ptr);
//	ptr = strtok(NULL, ",");
//	//printf("222 %s\n",ptr);
//	t->min5_load = atof(ptr);
//    ptr = strtok(NULL, "\0");
//    t->min15_load = atof(ptr);
//    fclose(fp);
//    return 0;
//}


