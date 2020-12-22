#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/sysinfo.h>
#include <string.h>
#include <sys/stat.h>
#include <signal.h>
#include <time.h>
#include <sys/statfs.h>
#include "bit.h"
#include "api_support_interface.h"

#define MESSAGE 0
int mount_message = 0;
//#include "statusctrllib.h"
/* ????	???????:???cpu??????? ????cpu?????
 * 		???????:
            ???????????????????§¹??,??????????????,?????úY??????????????????????,??????????????§Ö?c????:sin,cos,sqrt,exp,log??????????????????????????.
            ??????????? ?????????????,???????N?????,N????CPU?????
            ???????????????????????????????????? ????280??????????????????28??
            ???? 6344.1 MWIPS ????1s??????6344.1 1024*1024??whetsone???
 * ????1 mode 0???????????????????? 1;???????????????
 * ????2 pCPUInfo ???????????
 * ????? ??????
 */
STATUS ML_MTMW_CPUBIT(int mode, CPUInfo* pCPUInfo)
{
    if((mode != 1)&&(mode != 0))
    {
        return ERROR_PRAR;
    }
    if(pCPUInfo == NULL)
    {
        return ERROR_PRAR;
    }
    if(mode == 1)
    {
        unsigned int available_cpu_num = get_nprocs();
        if (available_cpu_num != CPU_NUM)
            pCPUInfo->available_cpu_num = 0xff;
        pCPUInfo->available_cpu_num = available_cpu_num;
        /*if(get_cpufrequency(&(pCPUInfo->cpu_frequency)) != 0)
        {
           return ERROR_SYSTEMCMD;
        }*/
        cpu_occupy_t cpu_stat1[CPUS_USAGE_NUM];
        cpu_occupy_t cpu_stat2[CPUS_USAGE_NUM];
        int i = 0;

        for (i = 0; i < CPUS_USAGE_NUM; ++i)
            get_cpuoccupy(i, (cpu_occupy_t *)&cpu_stat1[i]);
        sleep(1);
        for (i = 0; i < CPUS_USAGE_NUM; ++i)
            get_cpuoccupy(i, (cpu_occupy_t *)&cpu_stat2[i]);
        for (i = 0; i < CPUS_USAGE_NUM; ++i)
        {
            double cpu_usage = cal_cpuoccupy(&cpu_stat1[i], &cpu_stat2[i]);
            if (cpu_usage <= 0)
                cpu_usage = 0;
            else
                pCPUInfo->cpus_usage[i] = cpu_usage;
            printf("dddddddddddddddddddddddddddddddddddddd %d\n",cpu_usage);
        }
#if MESSAGE
printf("get_cpu_usage_avg = %lf\n",pCPUInfo->cpus_usage[0]);
printf("get_cpufrequency = %d\n",pCPUInfo->cpu_frequency);
#endif
        pCPUInfo->cpu_usage_warning = 0;
        if(pCPUInfo->cpus_usage[0] > 80)
        {
            //printf("cpu warning\n");
            pCPUInfo->cpu_usage_warning = 1;
        }

    }
    else
    {
        //???????????????????§¹??,??????????????,?????úY??????????????????????,??????????????§Ö?c????:sin,cos,sqrt,exp,log??????????????????????????.
        //??????????? ?????????????,???????N?????,N????CPU?????
        //???????????????????????????????????? ????280??????????????????28??
        //6344.1 MWIPS ????1s??????6344.1 1024*1024??whetsone???
        int ret = system("./manage_api/whetstone-double > ./manage_api/cputest.txt >./manage_api/cputest.txt");
        if(ret == -1)
        {
            return ERROR_SYSTEMCMD;
        }
        CPUOffline cpuoffline;
        char buf[128];
        char bufcmd[128];
        char bufmessage[128];
        char bufmessageall[2048] ;
        memset(buf,0,sizeof(buf));
        memset(bufcmd,0,sizeof(bufcmd));
        memset(bufmessage,0,sizeof(bufmessage));
        memset(bufmessageall,0,sizeof(bufmessageall));
        struct timeval tv;
        struct tm tm;


        FILE *fp = fopen("./manage_api/cputest.txt","r");
        if(fp == NULL)
        {
            return ERROR_OPENDIR;
        }
        while(fgets(bufmessage,sizeof(bufmessage),fp))
        {
            strcat(bufmessageall,bufmessage);
        }
        strcpy(pCPUInfo->cpu_test_info,bufmessageall);
        //ret = system("rm  ./manage_api/results/*");
#if 1
    printf("%s\n",pCPUInfo->cpu_test_info);
#endif
        if(ret == -1)
        {
            close(fp);
            return ERROR_SYSTEMCMD;
        }
        close(fp);
    }
    return 0;
}


/* ????
 * ???????	??????????????????????????????????¦Ä?????  ??¦ËMB
 * ???????  ????????§»??????????????
 * ????1 mode 0???????????????????? 1;???????????????
 * ????2 pCPUInfo ???????????
 * ????? ??????????0 ??????????????
 */
STATUS ML_MTMW_MEMBIT(int mode, MEMInfo *pMEMInfo)
{
    pthread_t tid;
    unsigned int total = 0,buf = 0,cache = 0,free = 0;
    if((mode != 0) && (mode != 1))
    {
        return ERROR_PRAR;
    }
    if(pMEMInfo == NULL)
    {
        return ERROR_PRAR;
    }
    if(mode ==1 )
    {
        mem_occupy_t mem_occupy;
        int ret = get_meminfo(1,&mem_occupy);
        if(ret == -1)
        {
            return ERROR_OPENDIR;
        }
        if(ret == -2)
        {
            return ERROR_SYSTEMCMD;
        }
        //pMEMInfo->mem_total = mem_occupy.memtotal;

        total = mem_occupy.memtotal;
        free = mem_occupy.memfree;
        cache = mem_occupy.cached;
        buf = mem_occupy.buffers;
        if(total < cache)
        {
            pMEMInfo->mem_free = free/1024;
            pMEMInfo->mem_used = (total - free)/1024;
            pMEMInfo->mem_used_perc = ((double)(pMEMInfo->mem_used)/(double)total) *100;
            pMEMInfo->mem_total = total/1024;

        }
        else
        {
            pMEMInfo->mem_used_perc = 100 * ((double)(total - buf - cache - free)/((double)total));
            pMEMInfo->mem_free = (free + cache + buf)/1024;
            pMEMInfo->mem_used = (total - free - cache - buf)/1024;
            pMEMInfo->mem_total =total/1024;
#if MESSAGE
printf("mem_used=%d  mem_free=%d ",pMEMInfo->mem_used,pMEMInfo->mem_free);
printf("mem_used_perc=%d  mem_total=%d\n",pMEMInfo->mem_used_perc,pMEMInfo->mem_total);
printf("mem_used=%d  mem_free=%d mem_used_perc=%d  mem_total=%d\n",pMEMInfo->mem_used,pMEMInfo->mem_free,pMEMInfo->mem_used_perc,pMEMInfo->mem_total);
#endif
        }
        pMEMInfo->mem_usage_warning = 0;
        if(pMEMInfo->mem_used_perc > 80)
        {
//printf("mem warning\n");
            pMEMInfo->mem_usage_warning = 1;
        }
    }
    else
    {
        char buf[128] ;
        char bufcmd[128] ;
        memset(buf,0,sizeof(buf));
        memset(bufcmd,0,sizeof(bufcmd));
        getcwd(buf,sizeof(buf));
        //strcat(bufcmd,".");
        strcat(bufcmd,buf);
        strcat(bufcmd,"/memtester 10M 1 > ");
        strcat(bufcmd,buf);
        strcat(bufcmd,"/memtester.txt");
        int ret = system("./manage_api/memtester 200M 1 > ./manage_api/memtester.txt");
        printf("sysret = %d\n",ret);
        if(ret == -1)
        {
            return ERROR_SYSTEMCMD;
        }
        //printf("sysret = %d\n",ret);

        char pPath[256] ="";
        strcpy(pPath,buf);
        //strcat(pPath,"/memtester.txt ");
        unsigned long nFileSize = -1;
        struct stat statbuff;
        while(stat(pPath,&statbuff) == 0)
        {

            if(nFileSize == statbuff.st_size)
            {
                break;
            }
            nFileSize = statbuff.st_size;
        }
        MEMOffline offlinemessage;
        if(read_memtest_result(&offlinemessage) == -1)
        {
            return ERROR_OPENDIR;
        }
        strcpy(pMEMInfo->compare_and,offlinemessage.compare_and);
        strcpy(pMEMInfo->compare_sub,offlinemessage.compare_sub);
        strcpy(pMEMInfo->compare_mul,offlinemessage.compare_mul);
        strcpy(pMEMInfo->compare_dvi,offlinemessage.compare_dvi);
        strcpy(pMEMInfo->compare_xor,offlinemessage.compare_xor);
#if MESSAGE
printf("%s,%s,%s,%s,%s\n",pMEMInfo->compare_and,pMEMInfo->compare_sub,pMEMInfo->compare_mul,pMEMInfo->compare_dvi,pMEMInfo->compare_xor);
#endif
    }
    return 0;
}

/* ????
    ?????1:????????§Ò??????????????????eth0-3 ???lo???????§Ò?? eth0/eth1/eth2/eth3/lo
          2:??????????????
          3???????????????????
          4?????eth0-eth4???????????????????????????????????????????????????????????????
    ??????????????????????
 * ????1 mode 0???????????????????? 1;???????????????
 * ????2 pMEMInfo ?????????????????
 * ????? ??????????0 ??????????????
 */
STATUS ML_MTMW_NETCARDBIT(int mode, NETInfo *pNETInfo)
{


    return 0;
}

/* ????	???DISK???????
 * ????1 mode 0???????????????????? 1;???????????????
 * ????2 pDISKInfo ???????????
 * ????? ??????????0 ??????????????
 */
STATUS ML_MTMW_DISKBIT(int mode,DISKInfo *pDISKInfo)
{
    pthread_t tid;
    int disk1_size = 0;
    int disk1_free_size = 0;
    if((mode != 0) && (mode != 1))
    {
        return ERROR_PRAR;
    }
    if(pDISKInfo == NULL)
    {
        return ERROR_PRAR;
    }
    if(mode == 1){
        int ret = get_flashapi_size(&disk1_size,&disk1_free_size);
        if(ret == -1)
        {
            return ERROR_SYSTEMCMD;
        }
        disk1_size = (float)(disk1_size) * (1.07374);
        disk1_free_size = (float)(disk1_free_size) * (1.07374);
        pDISKInfo->flash_size = disk1_size;
        pDISKInfo->flash_free_size = disk1_free_size;
        pDISKInfo->disk_usage = (float)(disk1_size - disk1_free_size)*(float)100 / (float)(disk1_size);
        //printf("s %f\n",(float)(disk1_size - disk1_free_size)*(float)100 / (float)(disk1_size));
        //pDISKInfo->disk_usage =pDISKInfo->disk_usage * 100;
#if MESSAGE
        printf("mainboard disk %d %d %f\n",pDISKInfo->flash_size, pDISKInfo->flash_free_size,pDISKInfo->disk_usage);
#endif
        pDISKInfo->disk_usage_waring = 0;
        if(pDISKInfo->disk_usage > 80)
        {
            pDISKInfo->disk_usage_waring = 1;
        }
    }
    else{
        //./iozone -a -n 512m -g 4g -i 0 -i 1 -i 5 -f /mnt/iozone
//printf("111111\n");
        int ret = system("./manage_api/iozone -a -n 16m -g 32m -i 0 -i 1 -i -f > ./manage_api/disktest.txt");
//printf("111112\n");
        if(ret == -1)
        {
            return ERROR_SYSTEMCMD;
        }
        char buf[256] = {};
        memset(buf,0,sizeof(buf));
        getcwd(buf,sizeof(buf));
        char pPath[128] ="";
        strcpy(pPath,buf);
        strcat(pPath,"./manage_api/disktest.txt");
        unsigned long nFileSize = -1;
        struct stat statbuff;
//		while(stat(pPath,&statbuff) == 0)
//		{
//			if(nFileSize == statbuff.st_size)
//			{
//				break;
//			}
//			nFileSize = statbuff.st_size;
//			sleep(3);
//		}
        DISKOffline diskoffline;
        ret = get_diskoffline(&diskoffline);
        if(ret == -1)
        {
            return ERROR_OPENDIR;
        }

        pDISKInfo->read_avg_speed= diskoffline.read_avg_speed/1024/8;

        pDISKInfo->write_avg_speed =diskoffline.write_avg_speed/1024/8;

        pDISKInfo->reread_avg_speed =diskoffline.reread_avg_speed/1024/8;

        pDISKInfo->rewrite_avg_speed =diskoffline.rewrite_avg_speed/1024/8;

        printf("diskouttest %d %d %d %d\n",pDISKInfo->read_avg_speed,pDISKInfo->write_avg_speed,pDISKInfo->reread_avg_speed,pDISKInfo->rewrite_avg_speed);
    }
    return 0;
}
/*?????
 * 1????????? ???????????????§³???????????????? ????????
 * 2????????? ???
 * ????1?? ????????
 * ????2?? ???????
 * ?????????????????????????0
*/
STATUS ML_MTMW_DISKBOARDBIT(int mode,DISKInfo *pDISKInfo)
{
    if((mode != 0) && (mode != 1))
    {
        return ERROR_PRAR;
    }
    if(pDISKInfo == NULL)
    {
        return ERROR_PRAR;
    }
    //printf("mount-message %d\n",mount_message);
  /*  if(mount_message == 0)
    {
        system("umount /dev/sdb5");
        mount_message += 1;
        system("mount /dev/sdb5 /media/root/ht706");
    }*/
    if(mode == 1)
    {
        char bufname[64];
        char buftmp[64];
        int allsize = 0;
        int allfreesize = 0;
        struct statfs diskInfo;

        memset(bufname,0,sizeof(bufname));
        memset(buftmp,0,sizeof(buftmp));
        system("ls /media/ht706 > ./manage_api/diskboard.txt");
        FILE *fp = fopen("./manage_api/diskboard.txt","r");
        if(fp == NULL)
        {
            return ERROR_OPENDIR;
        }
        int bytes = 0;
        while(fgets(buftmp,sizeof(buftmp),fp))
        {

        }
        while(buftmp[bytes] != '\n')
        {bytes++;}
        buftmp[bytes] = '\0';
        strcpy(bufname,"/media/ht706/");
        strcat(bufname,buftmp);
        fclose(fp);
        //printf("%s\n",bufname);
        //printf("/media/ht706/bad43121-1a3d-45c3-8aaa-e24593add88f\n");
        statfs(bufname,&diskInfo);
        unsigned long long blocksize = diskInfo.f_bsize;	//???block?????????????
        unsigned long long totalsize = blocksize * diskInfo.f_blocks; 	//??????????f_blocks?block?????
      //  printf("Total_size = %llu B = %llu KB = %llu MB = %llu GB\n",
      //  totalsize, totalsize>>10, totalsize>>20, totalsize>>30);
        unsigned long long freeDisk = diskInfo.f_bfree * blocksize;	//?????????§³
        unsigned long long availableDisk = diskInfo.f_bavail * blocksize; 	//????????§³
       // printf("Disk_free = %llu MB = %llu GB\nDisk_available = %llu MB = %llu GB\n",
       //freeDisk>>20, freeDisk>>30, availableDisk>>20, availableDisk>>30);
        unsigned int gensize = totalsize >> 30;
        unsigned int genfreesize = freeDisk >>30;
        //gensize += 1;
        //genfreesize += 1;
        gensize = gensize * 1.07374;//B?????GB 1024??1000????????
        genfreesize = genfreesize * 1.07374;
        printf("all: %d free: %d usage:%d\n",gensize,genfreesize,(gensize-genfreesize)/gensize);
        pDISKInfo->disk_usage = (gensize-genfreesize)/gensize;
        pDISKInfo->flash_size = gensize;
        pDISKInfo->flash_free_size = genfreesize;
        printf("----------------------------------------------------------%f %d %d \n", pDISKInfo->disk_usage,pDISKInfo->flash_size, pDISKInfo->flash_free_size);
        pDISKInfo->disk_usage_waring = 0;
        if(pDISKInfo->disk_usage > 80)
        {
            pDISKInfo->disk_usage_waring = 1;
        }
    }
    else
    {
        int ret = system("./manage_api/iozone -a -n 16m -g 32m -i 0 -i 1 -i -f > ./manage_api/disktest.txt");
        if(ret == -1)
        {
            return ERROR_SYSTEMCMD;
        }
        char buf[256] = {};
        memset(buf,0,sizeof(buf));
        getcwd(buf,sizeof(buf));
        char pPath[128] ="";
        strcpy(pPath,buf);
        strcat(pPath,"./manage_api/disktest.txt");
        unsigned long nFileSize = -1;
        struct stat statbuff;
//		while(stat(pPath,&statbuff) == 0)
//		{
//			if(nFileSize == statbuff.st_size)
//			{
//				break;
//			}
//			nFileSize = statbuff.st_size;
//			sleep(3);
//		}
        DISKOffline diskoffline;
        ret = get_diskoffline(&diskoffline);
        if(ret == -1)
        {
            return ERROR_OPENDIR;
        }

        pDISKInfo->read_avg_speed= diskoffline.read_avg_speed/1024/8;

        pDISKInfo->write_avg_speed =diskoffline.write_avg_speed/1024/8;

        pDISKInfo->reread_avg_speed =diskoffline.reread_avg_speed/1024/8;

        pDISKInfo->rewrite_avg_speed =diskoffline.rewrite_avg_speed/1024/8;

        printf("diskouttest %d %d %d %d\n",pDISKInfo->read_avg_speed,pDISKInfo->write_avg_speed,pDISKInfo->reread_avg_speed,pDISKInfo->rewrite_avg_speed);
    }

    return 0;
}




int cal_cpuoccupy1(CPU_OCCUPY *o, CPU_OCCUPY *n)
{
    unsigned long od, nd;
    unsigned long id, sd;
    int cpu_use = 0;

    od = (unsigned long)(o->user + o->nice + o->system + o->idle);
    nd = (unsigned long)(n->user + n->nice + n->system + n->idle);

    id = (unsigned long)(n->user - o->user);
    sd = (unsigned long)(n->system - o->system);

    if(nd-od !=0)
    {
        cpu_use = (int)((sd+id)*100)/(nd-od);

    }
    else
    {
        cpu_use = 0;
    }
    return cpu_use;
}


void get_cpuoccupy1(CPU_OCCUPY *cpust)
{
    FILE *fd;
    int n;
    char buf[256];
    CPU_OCCUPY *cpu_occupy;
    cpu_occupy = cpust;
    fd = fopen("/proc/stat", "r");
    fgets(buf, sizeof(buf), fd);
    //memset(cpu_occupy->name,0,sizeof(cpu_occupy->name));
    //cpu_occupy->user=0;
    //cpu_occupy->nice=0;
    //cpu_occupy->system=0;
    //cpu_occupy->idle=0;
    sscanf(buf, "%s %u %u %u %u", cpu_occupy->name, &cpu_occupy->user, &cpu_occupy->nice, &cpu_occupy->system, &cpu_occupy->idle);
    fclose(fd);


}
