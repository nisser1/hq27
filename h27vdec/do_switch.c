#include "gpio.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <string.h>
#include "sample_comm.h"
#include "do_switch.h"
#include "fifo_buffer.h"

struct ztGpioPrivate ztPrivate;
const unsigned int LED_NUM[4] = {
		LED1_NUM,LED2_NUM,LED3_NUM,LED4_NUM
	};


MESSAGE *msg = NULL;

FILE *INIT_Log(char *fileName)
{
	FILE *fp = NULL;
	fp = fopen(fileName,"a+");
	if(fp == NULL)
	{
		perror("fialed open configure file!!!\n");
	}
	return fp;
}


int do_switch_all(int cmd,FILE *fp)
{
	//PRINTF_PRT("SWITCH BEGIN\n");
	//PRINTF_PRT("previous_mode_all=%d cmd=%d\n",previous_mode_all,cmd);
	HI_S32 i, s32Ret = HI_SUCCESS;	
	char logBuf[128];
	memset(logBuf,0,sizeof(logBuf));
	
	//记录时间
	struct timeval tv;
	struct tm tm;
	gettimeofday(&tv,NULL);
	localtime_r(&tv.tv_sec,&tm);
	fprintf(fp,"%04d-%02d-%02d %02d:%02d:%02d  ",tm.tm_year+1900,tm.tm_mon+1,tm.tm_mday,tm.tm_hour,tm.tm_min,tm.tm_sec);
	sprintf(logBuf,"previous_mode_all = %3d    cmd         =%3d \n",previous_mode_all,cmd);
	fputs(logBuf,fp);
	//记录时间end
	
	
	/**********************************
	*step0切换osd叠加信息
	***********************************/
#if 0
	if(previous_mode_all == 201){
		HH_OSD_Destory1(0,0,0);
	}
#endif	
	if((previous_mode_all >= 190) && (previous_mode_all <= 199))
	{
		for(i = 0 ; i < 10 ; i++ )
		{
			HH_OSD_Destory1(i,0,0);
		}
	}
	if((previous_mode_all >= 140) && (previous_mode_all <= 149))
	{
		if(previous_mode_all == 140)
		{
			for(i = 0 ; i < 4 ; i++ )
			{
				HH_OSD_Destory1(i,0,0);
			}
		}
		if(previous_mode_all == 141)
		{
			for(i = 4 ; i < 8 ; i++ )
			{
				HH_OSD_Destory1(i-4,0,0);
			}
		}
		if(previous_mode_all == 142)
		{
			for(i = 5 ; i < 9; i++ )
			{
				HH_OSD_Destory1(i-5,0,0);
                printf("ppppppppppppppppppppppppppppppppppppddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddf\n");
			}
		}
	}
	if((previous_mode_all >= 100) && (previous_mode_all <= 109))
	{
		HH_OSD_Destory1(0,0,0);
	}
	/*********************************************
	*step1:解绑
	**********************************************/
#if 0
	if((previous_mode_all >= 200) && (previous_mode_all <= 205))
	{
		if(previous_mode_all == 201)
		{
			SAMPLE_COMM_VPSS_UnBind_VO(9 ,0, 0, 0);
		}
		//PRINTF_PRT("100 = %d\n",cmd%10);
	}
#endif
	if((previous_mode_all >= 100) && (previous_mode_all <= 109))
	{
		SAMPLE_COMM_VPSS_UnBind_VO((previous_mode_all % 10),0, 0, 0);
	}
	if((previous_mode_all >= 140) && (previous_mode_all <= 149))
	{	
		for(i = 0; i< 4; i++)
		{
			if((previous_mode_all%10) == 0)//代表140
			{
				SAMPLE_COMM_VPSS_UnBind_VO(i,0, 0, i);
				//PRINTF_PRT("140 = %d\n",i);
				if(s32Ret != HI_SUCCESS)
				{
					SAMPLE_PRT("vpss unbind vo fail for %#x!\n", s32Ret);
				}				
			}
			else if((previous_mode_all%10) == 1)//代表141
			{
				SAMPLE_COMM_VPSS_UnBind_VO(i+4,0, 0, i);
				//PRINTF_PRT("141 = %d\n",i+5);
				if(s32Ret != HI_SUCCESS)
				{
					SAMPLE_PRT("vpss unbind vo fail for %#x!\n", s32Ret);
				}				
			}
			else if((previous_mode_all%10) == 2)//代表142
			{
                if( (i + 5) == 7 ){
                    SAMPLE_COMM_VPSS_UnBind_VO(8, 0, 0, i);
                    continue;
                }
                if( (i+5) == 8 ){
                    SAMPLE_COMM_VPSS_UnBind_VO(7, 0, 0, i);
                    continue;
                }

				SAMPLE_COMM_VPSS_UnBind_VO(i+5,0, 0, i);
				//PRINTF_PRT("142 = %d\n",i+6);
				if(s32Ret != HI_SUCCESS)
				{
					SAMPLE_PRT("vpss unbind vo fail for %#x!\n", s32Ret);
				}				
			}			
		}
	}
	//九屏幕切换处理
	if((previous_mode_all >= 190) && (previous_mode_all <= 199))
	{
		//PRINTF_PRT("previous_mode_all = %d\n",previous_mode_all);
		
		for(i = 0; i< 9; i++){
			if(previous_mode_all%10 == i ){
				SAMPLE_COMM_VPSS_UnBind_VO(i ,0 ,0 ,9 );
				continue;
			}

			SAMPLE_COMM_VPSS_UnBind_VO(i ,0 ,0 , i);
			
		}
		
#if 0	//在存在HDMI信号时交换切换模式
		for(i = 0; i< 9; i++)
		{
			//非9模式的切换  
			/*
				1: 当默认模式下直接切换时 previous_mode_all=199 这时需要将9先解绑for循环走不到9
				2: 当9内有切换时          previous_mode_all<199 这时两种情况1: 9的位置还是9(在此cmd下按了两次确认)
																			2: 9的位置时previous_mode_all%10  这是需要执行解绑两个
			*/		
			//@2			
			if(cmd < 190)
			{	
				if(previous_mode_all%10 == i)
				{
					//@21
					if(who_in_pos9 == 9)
					{
						//SAMPLE_COMM_VPSS_UnBind_VO(i,0, 0, i);
						SAMPLE_COMM_VPSS_UnBind_VO(9 ,0 ,0 , 9);
					}
					else
					{
						//@22
						SAMPLE_COMM_VPSS_UnBind_VO((previous_mode_all % 10) ,0 ,0 ,9 );
						SAMPLE_COMM_VPSS_UnBind_VO(9 ,0 ,0 ,(previous_mode_all % 10));
						continue;
					}
					
				}
				//@1
				if(previous_mode_all%10 == 9)
				{
					SAMPLE_COMM_VPSS_UnBind_VO(9 ,0 ,0 , 9);
				}
				SAMPLE_COMM_VPSS_UnBind_VO(i,0, 0, i);
				PRINTF_PRT("cmd < 190 i=%d\n",i);
				continue;
			}
			
			//9模式内部切换
			/*
				1: 切换命令 且有选择与上次不同的视频位置做切换   previous_mode_all != cmd 说明切换时一定有位置互换 只需要将之前的
				2: 
			*/
			
			//@1
			if((0== i) && (previous_mode_all != cmd))//解绑vo 9位置对应的视频  
			{
				SAMPLE_COMM_VPSS_UnBind_VO((previous_mode_all % 10) ,0 ,0 ,9 );
				SAMPLE_COMM_VPSS_UnBind_VO((cmd % 10) ,0 ,0 ,(cmd % 10));
				if(previous_mode_all != 199)
				{
					SAMPLE_COMM_VPSS_UnBind_VO(9 ,0 ,0 ,(previous_mode_all % 10) );
				}
			}
			//跳过i的位置  上边已经解绑过了 只要命令不一样  上方的if不管时什么都要进这里再跳过i
			if(((cmd%10) == i) && (previous_mode_all != cmd))
			{
				continue;
			}
			
			//@02
			if((previous_mode_all == cmd) && ((cmd % 10) == i))
			{
				if(who_in_pos9 == (cmd%10))
				{
					SAMPLE_COMM_VPSS_UnBind_VO((cmd % 10) ,0 ,0 ,9 );
					SAMPLE_COMM_VPSS_UnBind_VO(9 ,0 ,0 ,(cmd % 10) );
				}
				else
				{
					SAMPLE_COMM_VPSS_UnBind_VO((cmd % 10) ,0 ,0 ,(cmd % 10));
					SAMPLE_COMM_VPSS_UnBind_VO(9 ,0 ,0 ,9 );
				}
				continue;
			}

			SAMPLE_COMM_VPSS_UnBind_VO(i,0, 0, i);
			PRINTF_PRT("i=%d\n",i);
			if(s32Ret != HI_SUCCESS)
			{
				SAMPLE_PRT("vpss unbind vo fail for %#x!\n", s32Ret);
			}			
		}
#endif		
	}
	
	
	/*********************************************
	*step2:停止Vo通道
	**********************************************/
	//PRINTF_PRT("FLAGS1 \n");
	if((previous_mode_all/10) != (cmd/10))
	{
		if((previous_mode_all/10) == 10 )
		{
			//PRINTF_PRT("STOP MODE 1\n");
			SAMPLE_COMM_VO_StopChn(0, VO_MODE_1MUX);
		}
		if((previous_mode_all/10) == 14 )
		{
			//PRINTF_PRT("STOP MODE 4\n");
			SAMPLE_COMM_VO_StopChn(0, VO_MODE_4MUX);
		}
		if((previous_mode_all/10) == 19 )
		{
			//PRINTF_PRT("STOP MODE 9\n");
			SAMPLE_COMM_VO_StopChn(0, VO_MODE_9MUX);
		}
	}
	/*********************************************
	*step3:开启不同Vo通道
	**********************************************/
	//PRINTF_PRT("FLAGS2 \n");
	if((previous_mode_all/10) != (cmd/10))
	{
		if((cmd/10) == 10 )
		{
			//PRINTF_PRT("START MODE 1\n");
			SAMPLE_COMM_VO_StartChn(0, VO_MODE_1MUX);
		}
		if((cmd/10) == 14 )
		{
			//PRINTF_PRT("START MODE 4\n");
			SAMPLE_COMM_VO_StartChn(0, VO_MODE_4CUS);
		}
		if((cmd/10) == 19 )
		{
			//PRINTF_PRT("START MODE 9\n");
			SAMPLE_COMM_VO_StartChn(0, VO_MODE_9CUS);
		}
#if 0		
		if((cmd/10) == 20)
		{
			//测试界面
			SAMPLE_COMM_VO_StartChn(0, VO_MODE_1MUX);
		}
#endif
	}
	/*********************************************
	*step3:vpss 和 vo 建立新的绑定关系
	**********************************************/
	//PRINTF_PRT("FLAGS3 \n");
	if((cmd >= 100) && (cmd <= 109))
	{
		SAMPLE_COMM_VPSS_Bind_VO((cmd % 10),0, 0, 0);
		//PRINTF_PRT("100 = %d\n",cmd%10);
		current_vdeio_display_mode = 1;
	}
#if 0
	if((cmd >= 200) && (cmd <= 205))
	{
		SAMPLE_COMM_VPSS_Bind_VO(9 ,0, 0, 0);
		//PRINTF_PRT("100 = %d\n",cmd%10);
		current_vdeio_display_mode = 20;
	}
#endif
	//PRINTF_PRT("FLAGS4 \n");
	if((cmd >= 140) && (cmd <= 149))
	{	
		for(i = 0; i< 4; i++)
		{
			if(cmd == 140)//代表140
			{
				SAMPLE_COMM_VPSS_Bind_VO(i,0, 0, i);
				//PRINTF_PRT("140 = %d\n",i);
				if(s32Ret != HI_SUCCESS)
				{
					SAMPLE_PRT("vpss unbind vo fail for %#x!\n", s32Ret);
				}				
			}
			else if(cmd == 141)//代表141
			{
				SAMPLE_COMM_VPSS_Bind_VO(i+4,0, 0, i);
				//PRINTF_PRT("141 = %d\n",i+4);
				if(s32Ret != HI_SUCCESS)
				{
					SAMPLE_PRT("vpss unbind vo fail for %#x!\n", s32Ret);
				}				
			}
			else if(cmd == 142)//代表142
			{
                if( (i+5) == 7){
				    SAMPLE_COMM_VPSS_Bind_VO(8, 0, 0, i);	
                    continue;
                }
                if( (i+5) == 8){
				    SAMPLE_COMM_VPSS_Bind_VO(7, 0, 0, i);	
                    continue;
                }
				SAMPLE_COMM_VPSS_Bind_VO(i+5,0, 0, i);	
				//PRINTF_PRT("142 e = %d %d\n",i+6,i);
				if(s32Ret != HI_SUCCESS)
				{
					SAMPLE_PRT("vpss unbind vo fail for %#x!\n", s32Ret);
				}				
			}			
		}
		current_vdeio_display_mode = 4;
	}
	
	//九屏幕初始化
	//PRINTF_PRT("FLAGS5 \n");
	if((cmd >= 190) && (cmd <= 199)){
		for(i = 0; i < 9 ; i++){
			if(cmd % 10 == i){
				SAMPLE_COMM_VPSS_Bind_VO(i ,0 ,0 , 9);
				who_in_pos9 = i;
				continue;
			}
			SAMPLE_COMM_VPSS_Bind_VO(i ,0 ,0 , i);
		}

		
#if 0  //存在HDMI信号时9屏幕的切换方式
		if(cmd == 199)
		{
			SAMPLE_COMM_VPSS_Bind_VO(9 ,0 ,0 ,9);
		}
		PRINTF_PRT("FLAGS6\n");
		for(i = 0; i< 9; i++)
		{
			PRINTF_PRT("FLAGS7\n");
			if(((cmd % 10) == i) && (previous_mode_all != cmd)) 
			{
				SAMPLE_COMM_VPSS_Bind_VO((cmd % 10) ,0 ,0 ,9 );
				PRINTF_PRT("CMD = %d\n",cmd);
				SAMPLE_COMM_VPSS_Bind_VO(9 ,0 ,0 ,(cmd % 10) );
				who_in_pos9 = i; 
				continue;
			}
			
			
			if((i == cmd%10) && (cmd == previous_mode_all))
			{
				if(who_in_pos9 == 9)
				{
					SAMPLE_COMM_VPSS_Bind_VO(cmd%10 ,0 ,0 ,9);
					SAMPLE_COMM_VPSS_Bind_VO(9 ,0 ,0 ,cmd%10);
					who_in_pos9 = i;
				}
				else
				{
					SAMPLE_COMM_VPSS_Bind_VO(9 ,0 ,0 ,9);
					SAMPLE_COMM_VPSS_Bind_VO(cmd%10 ,0 ,0 ,cmd%10);
					who_in_pos9 = 9;
				}
				continue;
			}
			PRINTF_PRT("BIND9 i begin=%d\n",i);
			SAMPLE_COMM_VPSS_Bind_VO(i,0, 0, i);
			PRINTF_PRT("BIND9 i end=%d\n",i);

			if(s32Ret != HI_SUCCESS)
			{
				SAMPLE_PRT("vpss unbind vo fail for %#x!\n", s32Ret);
			}			
		}
	#endif
		current_vdeio_display_mode = 9;
	}

	//osd添加  九屏模式下的视频切换操作
	/*
		1
	*/
	if((cmd >= 190) && (cmd <= 199))
	{
		for(i = 0 ; i < 9 ; i++ )
		{
			if(i == cmd % 10)
			{
				HH_OSD_Init(cmd,10,cmd,0);
				HH_OSD_Init(9,10,cmd,0);
				continue;	
			}
			HH_OSD_Init(i,9,190+i,0);
		}
		
#if 0		
		for(i = 0 ; i < 9 ; i++ )
		{
			if(cmd == 199)
			{
				HH_OSD_Init(9 ,10 ,199 ,0);
			}
			printf("add osd %d\n",i);
			if((i == (cmd %10)) && (previous_mode_all != cmd))
			{
				HH_OSD_Init(9 ,9 ,cmd ,0);
				HH_OSD_Init(i ,10 ,199 ,0);
				continue;				
			}
			if((i == (cmd %10)) && (previous_mode_all == cmd))
			{
				if(who_in_pos9 == 9)
				{
					HH_OSD_Init(9 ,9 ,199 ,9);
					HH_OSD_Init(cmd%10 ,9 ,cmd ,0);
					continue;
				}
				else
				{
					HH_OSD_Init(9 ,9 ,cmd ,0);
					HH_OSD_Init(i ,10 ,199 ,0);
				}
			}
			HH_OSD_Init(i,9,190+i,0);
		}
#endif
	}
	
	if((cmd >= 140) && (cmd <= 149))
	{
		if(cmd == 140)
		{
			for(i = 0 ; i < 4 ; i++ )
			{
				HH_OSD_Init(i,4,i,0);
			}
		}
		if(cmd == 141)
		{
			for(i = 4 ; i < 8 ; i++ )
			{
				HH_OSD_Init(i-4,4,i,0);
			}
		}
		if(cmd == 142)
		{
			for(i = 5 ; i < 9 ; i++ )
			{
                if(i == 7){
				    HH_OSD_Init(3,4,7,0);
                    continue;
                }
                if(i == 8){
				    HH_OSD_Init(2,4,8,0);
                    continue;
                }
				HH_OSD_Init(i-5,4,i,0);
			}
		}
	}
	if((cmd >= 100) && (cmd <= 109))
	{
		HH_OSD_Init(0,1,cmd,0);
	}
#if 0
	if((cmd >= 200) && (cmd <= 205))
	{
		if(cmd == 201){
			HH_OSD_Init(0,1,199,0);
		}
	}
#endif
	previous_tow_mode = previous_mode_all;
	previous_mode_all = cmd;

	PRINTF_PRT("SWITCH EDN\n");
	return 0;
}





//选择水印切换在这里做
int set_cmd(int sig_cmd,MESSAGE *msg,FILE *fp,int *execute_cmd)
{
	struct timeval tv;
	struct tm tm;
	char logBuf[128];
	int move_complete_cmd;
	//sig异步通知上来的信号     
	//printf("dddddddddd sig_cmd %d\n",sig_cmd);
	if((sig_cmd == 60) || (sig_cmd == 200) || (sig_cmd == 140) || (sig_cmd == 141)\
		|| (sig_cmd == 142) || (sig_cmd == 201) || (sig_cmd ==202) )//200是默认9屏模式
	{
		if(sig_cmd == 200){
			*execute_cmd = 1;
			return 190;
		}else if(sig_cmd == 140){
			*execute_cmd = 1;
			witch_mode_4 = 140;
			return 140;
			
		}else if(sig_cmd == 141){
			*execute_cmd = 1;
			witch_mode_4 = 141;
			return 141;
		}else if(sig_cmd == 142){
			*execute_cmd = 1;
			witch_mode_4 = 142;
			return 142;
		}else if(sig_cmd == 201){
			*execute_cmd = 0;
			//这里做一个条件变量   让另一个位置去点灯
			pthread_mutex_lock(&mutex);
			if(led_ctl == 1){
				pthread_mutex_unlock(&mutex);
				return 0;				
			}
			pthread_mutex_unlock(&mutex);			
			pthread_mutex_lock(&mutex);
			pthread_cond_signal(&cond);
			pthread_mutex_unlock(&mutex);
			return 0;
		}else if(sig_cmd == 202){
			*execute_cmd = 1;
			return previous_tow_mode;
		}else if(sig_cmd == 60){
			*execute_cmd = 1;
			return 0;
		}else{
			*execute_cmd = 0;
		}
		return 0;
	}
	
	
	//PRINTF_PRT("sig_cmd = %d",sig_cmd);
	//9模式下只响应 1切4的命令(3个)  切一的命令 vo9视频切1显示
	/*if((sig_cmd >= 190) && (sig_cmd <= 199))
	{
		return -1;
	}*/
	pthread_mutex_lock(&mutex);
	if(current_vdeio_display_mode == 9)
	{
		sig_cmd += 5; //205 206  9屏幕
		if((msg->pos[0]>9) || (msg->pos[1]>9) || (msg->pos[1] < 0) || (msg->pos[0] < 0)){
			msg->pos[0]=0; 
			msg->pos[1]=0;
		}
	}
	if(current_vdeio_display_mode == 4)
	{
		if((sig_cmd != 203) && (sig_cmd != 204)){
			sig_cmd -= 5;
		}else{
			sig_cmd += 10; //207 208  4屏幕
		}
		if(msg->pos[0]>3)
			msg->pos[0]=0;
			//msg->pos[1]=0;
	}
	pthread_mutex_unlock(&mutex);
	if(current_vdeio_display_mode == 1)
	{
		return 0;
	}
	//msg->pos[1]表示按下上下左右键位置  msg->pos[0]表示按下上下左右键之前位置
	memset(logBuf,0,sizeof(logBuf));

	gettimeofday(&tv,NULL);
	localtime_r(&tv.tv_sec,&tm);
	fprintf(fp,"%04d-%02d-%02d %02d:%02d:%02d  ",tm.tm_year+1900,tm.tm_mon+1,tm.tm_mday,tm.tm_hour,tm.tm_min,tm.tm_sec);
	sprintf(logBuf,"begin msg->pos[0] = %-3d    msg->pos[1] = %-3d \n",msg->pos[0],msg->pos[1]);
	fputs(logBuf,fp);
	printf("bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbsig_cmd=%d\n",sig_cmd);
	pthread_mutex_lock(&mutex);
	switch(sig_cmd)
	{	
		//
		//		208顺时针
		//		209逆时针
		case 208:
			if((msg->pos[0] > 4) && (msg->pos[0] < 9)){
				msg->pos[1] = msg->pos[1]-1;			
				HH_OSD_Destory1(msg->pos[0],0,0);		
				HH_OSD_Init(msg->pos[0],9,msg->pos[0],0);  
				HH_OSD_Destory1(msg->pos[1],0,0);
				HH_OSD_Init(msg->pos[1],9,msg->pos[1],1);  //要变大的	
				move_complete_cmd = msg->pos[1] + 190;
			}
			if((msg->pos[0] < 5) && (msg->pos[0] > 0)){
				msg->pos[1] = msg->pos[1]-1;
				HH_OSD_Destory1(msg->pos[0],0,0);
				HH_OSD_Init(msg->pos[0],9,msg->pos[0],0);  
				HH_OSD_Destory1(msg->pos[1],0,0);
				HH_OSD_Init(msg->pos[1],9,msg->pos[1],1); 
				move_complete_cmd = msg->pos[1] + 190;
			}
			if(msg->pos[0] == 0){
				msg->pos[1] = 9;
				HH_OSD_Destory1(msg->pos[0],0,0);
				HH_OSD_Init(msg->pos[0],9,msg->pos[0],0);  
				HH_OSD_Destory1(msg->pos[1],0,0);
				HH_OSD_Init(msg->pos[1],10,who_in_pos9,1);    
				move_complete_cmd = who_in_pos9 + 100;			//选中9  将要将该通道的数据切成单屏幕

			}		
			if(msg->pos[0] == 9){	
				msg->pos[1] = 8;
				HH_OSD_Destory1(msg->pos[0],0,0);
				HH_OSD_Init(msg->pos[0],10,who_in_pos9,0);  
				HH_OSD_Destory1(msg->pos[1],0,0);
				HH_OSD_Init(msg->pos[1],9,msg->pos[1],1);  //要变大的
				move_complete_cmd = msg->pos[1] + 190;
			}
			break;
		case 209:
			if(msg->pos[0] < 4){
				msg->pos[1] = msg->pos[1]+1;
				HH_OSD_Destory1(msg->pos[0],msg->pos[0],0);
				HH_OSD_Init(msg->pos[0],9,msg->pos[0],0);  
				HH_OSD_Destory1(msg->pos[1],0,0);
				HH_OSD_Init(msg->pos[1],9,msg->pos[1],1); 
				move_complete_cmd = msg->pos[1] + 190;
			}
			if((msg->pos[0] > 3) && (msg->pos[0] < 8)){
				msg->pos[1] = msg->pos[1]+1;
				HH_OSD_Destory1(msg->pos[0],0,0);
				HH_OSD_Init(msg->pos[0],9,msg->pos[0],0);  
				HH_OSD_Destory1(msg->pos[1],0,0);
				HH_OSD_Init(msg->pos[1],9,msg->pos[1],1); //要变颜色的
				move_complete_cmd = msg->pos[1] + 190;
			}
			if( msg->pos[0] == 8){
				msg->pos[1] = 9;
				HH_OSD_Destory1(msg->pos[0],0,0);
				HH_OSD_Init(msg->pos[0],9,msg->pos[0],0);
				HH_OSD_Destory1(msg->pos[1],0,0);
				HH_OSD_Init(msg->pos[1],10,who_in_pos9,1); //要变大的
				move_complete_cmd = who_in_pos9 + 100;
			}
			if(msg->pos[0] == 9){//这里可能会有问题  9下来无法显示在0上
				msg->pos[1] = 0;
				HH_OSD_Destory1(msg->pos[0],0,0);
				HH_OSD_Init(msg->pos[0],10,who_in_pos9,0);  
				HH_OSD_Destory1(msg->pos[1],0,0);
				HH_OSD_Init(msg->pos[1],9,msg->pos[1],1);  //要变大的
				move_complete_cmd = msg->pos[1] + 190;
			}
			break;


		//				上 58
		//		左  57			右 55
		//				下 56
		//
		case 55: //向右  小于4可以向右			9可以向右向右直接跳到8的位置 
			if(msg->pos[0] < 4 ){
				msg->pos[1] = msg->pos[1]+1;
				HH_OSD_Destory1(msg->pos[0],msg->pos[0],0);
				HH_OSD_Init(msg->pos[0],9,msg->pos[0],0);  
				HH_OSD_Destory1(msg->pos[1],0,0);
				HH_OSD_Init(msg->pos[1],9,msg->pos[1],1); 
				move_complete_cmd = msg->pos[1] + 190;
			}
			if(msg->pos[0] == 9){	
				msg->pos[1] = 8;
				HH_OSD_Destory1(msg->pos[0],0,0);
				HH_OSD_Init(msg->pos[0],10,who_in_pos9,0);  
				HH_OSD_Destory1(msg->pos[1],0,0);
				HH_OSD_Init(msg->pos[1],9,msg->pos[1],1);  //要变大的
				move_complete_cmd = msg->pos[1] + 190;
			}
			move_complete_cmd = msg->pos[1] + 190;
			break;
		case 56://向下   大于4可以向下			9可以向右下向下直接跳到0的位置
			if((msg->pos[0] > 4) && (msg->pos[0] < 9)){
				msg->pos[1] = msg->pos[1]-1;			
				HH_OSD_Destory1(msg->pos[0],0,0);		
				HH_OSD_Init(msg->pos[0],9,msg->pos[0],0);  
				HH_OSD_Destory1(msg->pos[1],0,0);
				HH_OSD_Init(msg->pos[1],9,msg->pos[1],1);  //要变大的	
				move_complete_cmd = msg->pos[1] + 190;
			}
			if(msg->pos[0] == 9){//这里可能会有问题  9下来无法显示在0上
				msg->pos[1] = 0;
				HH_OSD_Destory1(msg->pos[0],0,0);
				HH_OSD_Init(msg->pos[0],10,who_in_pos9,0);  
				HH_OSD_Destory1(msg->pos[1],0,0);
				HH_OSD_Init(msg->pos[1],9,msg->pos[1],1);  //要变大的
				move_complete_cmd = msg->pos[1] + 190;
			}	
			move_complete_cmd = msg->pos[1] + 190;			
			break;
		case 57://向左	 1-4可以向左			4-8可以向左跳到9的位置  
			if((msg->pos[0] < 5) && (msg->pos[0] > 0)){
				msg->pos[1] = msg->pos[1]-1;
				HH_OSD_Destory1(msg->pos[0],0,0);
				HH_OSD_Init(msg->pos[0],9,msg->pos[0],0);  
				HH_OSD_Destory1(msg->pos[1],0,0);
				HH_OSD_Init(msg->pos[1],9,msg->pos[1],1); 
				move_complete_cmd = msg->pos[1] + 190;
			}
			if((msg->pos[0] > 4) && (msg->pos[0] < 9)){
				msg->pos[1] = 9;
				HH_OSD_Destory1(msg->pos[0],0,0);
				HH_OSD_Init(msg->pos[0],9,msg->pos[0],0);
				HH_OSD_Destory1(msg->pos[1],0,0);
				HH_OSD_Init(msg->pos[1],10,who_in_pos9,1); //要变大的
				move_complete_cmd = who_in_pos9 + 100;
			}
			break;
		case 58://向上   4-7可以向上			0-3可以向上至9的位置	
			if((msg->pos[0] > 3) && (msg->pos[0] < 8)){
				msg->pos[1] = msg->pos[1]+1;
				HH_OSD_Destory1(msg->pos[0],0,0);
				HH_OSD_Init(msg->pos[0],9,msg->pos[0],0);  
				HH_OSD_Destory1(msg->pos[1],0,0);
				HH_OSD_Init(msg->pos[1],9,msg->pos[1],1); //要变颜色的
				move_complete_cmd = msg->pos[1] + 190;
			}
			if(msg->pos[0] < 4){
				msg->pos[1] = 9;
				HH_OSD_Destory1(msg->pos[0],0,0);
				HH_OSD_Init(msg->pos[0],9,msg->pos[0],0);  
				HH_OSD_Destory1(msg->pos[1],0,0);
				HH_OSD_Init(msg->pos[1],10,who_in_pos9,1);    
				move_complete_cmd = who_in_pos9 + 100;			//选中9  将要将该通道的数据切成单屏幕

			}
			break;	
			
			
		//4屏幕模式
				//
		//		303顺时针
		//		204逆时针
		case 213:
			if(msg->pos[0] == 0 )
				msg->pos[1] = 2;
			if(msg->pos[0] == 2)
				msg->pos[1] = 3;
			if(msg->pos[0] == 3)
				msg->pos[1] = 1;
			if(msg->pos[0] == 1)
				msg->pos[1] = 0;
			if(witch_mode_4 == 140)
			{
				HH_OSD_Destory1(msg->pos[0],0,0);
				HH_OSD_Init(msg->pos[0],4,msg->pos[0],0);  
				HH_OSD_Destory1(msg->pos[1],0,0);
				HH_OSD_Init(msg->pos[1],4,msg->pos[1],1);
				move_complete_cmd = msg->pos[1] + 100;
			}
			if(witch_mode_4 == 141)
			{
				HH_OSD_Destory1(msg->pos[0],0,0);
				HH_OSD_Init(msg->pos[0],4,msg->pos[0]+4,0);  
				HH_OSD_Destory1(msg->pos[1],0,0);
				HH_OSD_Init(msg->pos[1],4,msg->pos[1]+4,1);
				move_complete_cmd = msg->pos[1] + 100 + 4;
			}
			if(witch_mode_4 == 142)
			{
				HH_OSD_Destory1(msg->pos[0],0,0);
				HH_OSD_Init(msg->pos[0],4,msg->pos[0]+5,0);  
				HH_OSD_Destory1(msg->pos[1],0,0);
				HH_OSD_Init(msg->pos[1],4,msg->pos[1]+5,1);
				move_complete_cmd = msg->pos[1] + 100 + 5;
			}
			break;
		case 214:
			if(msg->pos[0] == 0)
				msg->pos[1] = 1;
			if(msg->pos[0] == 2)
				msg->pos[1] = 0;
			if(msg->pos[0] == 3)
				msg->pos[1] = 2;
			if(msg->pos[0] == 1)
				msg->pos[1] = 3;
			if(witch_mode_4 == 140)
			{
				HH_OSD_Destory1(msg->pos[0],0,0);
				HH_OSD_Init(msg->pos[0],4,msg->pos[0],0);  
				HH_OSD_Destory1(msg->pos[1],0,0);
				HH_OSD_Init(msg->pos[1],4,msg->pos[1],1);
				move_complete_cmd = msg->pos[1] + 100;
			}
			if(witch_mode_4 == 141)
			{
				HH_OSD_Destory1(msg->pos[0],0,0);
				HH_OSD_Init(msg->pos[0],4,msg->pos[0]+4,0);  
				HH_OSD_Destory1(msg->pos[1],0,0);
				HH_OSD_Init(msg->pos[1],4,msg->pos[1]+4,1);
				move_complete_cmd = msg->pos[1] + 100 + 4;
			}
			if(witch_mode_4 == 142)
			{
				HH_OSD_Destory1(msg->pos[0],0,0);
				HH_OSD_Init(msg->pos[0],4,msg->pos[0]+5,0);  
				HH_OSD_Destory1(msg->pos[1],0,0);
				HH_OSD_Init(msg->pos[1],4,msg->pos[1]+5,1);
				move_complete_cmd = msg->pos[1] + 100 + 5;
			}
			break;

		//	0	 2  	 	     	4   	6					5      8	  
		//		   
		//	1    3			    	5	    7					6      7
		case 45: //向右  小于2可以向右			 
			if(msg->pos[0] == 0 )
				msg->pos[1] = 2;
			if(msg->pos[0] == 1)
				msg->pos[1]=3;
			if(witch_mode_4 == 140)
			{
				HH_OSD_Destory1(msg->pos[0],0,0);
				HH_OSD_Init(msg->pos[0],4,msg->pos[0],0);  
				HH_OSD_Destory1(msg->pos[1],0,0);
				HH_OSD_Init(msg->pos[1],4,msg->pos[1],1);
				move_complete_cmd = msg->pos[1] + 100;
			}
			if(witch_mode_4 == 141)
			{
				HH_OSD_Destory1(msg->pos[0],0,0);
				HH_OSD_Init(msg->pos[0],4,msg->pos[0]+4,0);  
				HH_OSD_Destory1(msg->pos[1],0,0);
				HH_OSD_Init(msg->pos[1],4,msg->pos[1]+4,1);
				move_complete_cmd = msg->pos[1] + 100 + 4;
			}
			if(witch_mode_4 == 142)
			{
                if(msg->pos[1] == 3){
				    HH_OSD_Destory1(msg->pos[0],0,0);
				    HH_OSD_Init(msg->pos[0],4,msg->pos[0]+5,0);  
				    HH_OSD_Destory1(msg->pos[1],0,0);
				    HH_OSD_Init(msg->pos[1],4,msg->pos[1]+4,1);
				    move_complete_cmd = msg->pos[1] + 100 + 4;
                }
                if(msg->pos[1] == 2){
				    HH_OSD_Destory1(msg->pos[0],0,0);
				    HH_OSD_Init(msg->pos[0],4,msg->pos[0]+5,0);  
				    HH_OSD_Destory1(msg->pos[1],0,0);
				    HH_OSD_Init(msg->pos[1],4,msg->pos[1]+6,1);
				    move_complete_cmd = msg->pos[1] + 100 + 6;
                }
			}
			break;
		case 46://向下   大于2可以向下			9可以向右下向下直接跳到0的位置
			if(msg->pos[0] == 0)
				msg->pos[1] = 1;			
			if(msg->pos[0] == 2)
				msg->pos[1] = 3;	
			if(witch_mode_4 == 140)
			{
				HH_OSD_Destory1(msg->pos[0],0,0);
				HH_OSD_Init(msg->pos[0],4,msg->pos[0],0);  
				HH_OSD_Destory1(msg->pos[1],0,0);
				HH_OSD_Init(msg->pos[1],4,msg->pos[1],1);
				move_complete_cmd = msg->pos[1] + 100;
			}
			if(witch_mode_4 == 141)
			{
				HH_OSD_Destory1(msg->pos[0],0,0);
				HH_OSD_Init(msg->pos[0],4,msg->pos[0]+4,0);  
				HH_OSD_Destory1(msg->pos[1],0,0);
				HH_OSD_Init(msg->pos[1],4,msg->pos[1]+4,1);
				move_complete_cmd = msg->pos[1] + 100 + 4;
			}
			if(witch_mode_4 == 142)
			{
                if(msg->pos[1] == 1){
				    HH_OSD_Destory1(msg->pos[0],0,0);
				    HH_OSD_Init(msg->pos[0],4,msg->pos[0]+5,0);  
				    HH_OSD_Destory1(msg->pos[1],0,0);
				    HH_OSD_Init(msg->pos[1],4,msg->pos[1]+5,1);
				    move_complete_cmd = msg->pos[1] + 100 + 5;
                }
                if(msg->pos[1] == 3){
				    HH_OSD_Destory1(msg->pos[0],0,0);
				    HH_OSD_Init(msg->pos[0],4,msg->pos[0]+6,0);  
				    HH_OSD_Destory1(msg->pos[1],0,0);
				    HH_OSD_Init(msg->pos[1],4,msg->pos[1]+4,1);
				    move_complete_cmd = msg->pos[1] + 100 + 4; 
                }
			}
			break;
		case 47://向左	 1-4可以向左			4-8可以向左跳到9的位置  
			if(msg->pos[0] == 2)
				msg->pos[1] = 0;
			if(msg->pos[0] == 3)
				msg->pos[1] = 1;
			if(witch_mode_4 == 140)
			{
				HH_OSD_Destory1(msg->pos[0],0,0);
				HH_OSD_Init(msg->pos[0],4,msg->pos[0],0);  
				HH_OSD_Destory1(msg->pos[1],0,0);
				HH_OSD_Init(msg->pos[1],4,msg->pos[1],1);
				move_complete_cmd = msg->pos[1] + 100;
			}
			if(witch_mode_4 == 141)
			{
				HH_OSD_Destory1(msg->pos[0],0,0);
				HH_OSD_Init(msg->pos[0],4,msg->pos[0]+4,0);  
				HH_OSD_Destory1(msg->pos[1],0,0);
				HH_OSD_Init(msg->pos[1],4,msg->pos[1]+4,1);
				move_complete_cmd = msg->pos[1] + 100 + 4;
			}
			if(witch_mode_4 == 142)
			{
                if(msg->pos[1] == 1){
				    HH_OSD_Destory1(msg->pos[0],0,0);
				    HH_OSD_Init(msg->pos[0],4,msg->pos[0]+4,0);  
				    HH_OSD_Destory1(msg->pos[1],0,0);
				    HH_OSD_Init(msg->pos[1],4,msg->pos[1]+5,1);
				    move_complete_cmd = msg->pos[1] + 100 + 5;
                }
                if(msg->pos[1] == 0){
				    HH_OSD_Destory1(msg->pos[0],0,0);
				    HH_OSD_Init(msg->pos[0],4,msg->pos[0]+6,0);  
				    HH_OSD_Destory1(msg->pos[1],0,0);
				    HH_OSD_Init(msg->pos[1],4,msg->pos[1]+5,1);
				    move_complete_cmd = msg->pos[1] + 100 + 5;
                }
			}
			break;
		case 48://向上   4-7可以向上			0-3可以向上至9的位置	
			if(msg->pos[0] == 1)
				msg->pos[1] = 0;
			if(msg->pos[0] == 3)
				msg->pos[1] = 2;
			if(witch_mode_4 == 140)
			{
				HH_OSD_Destory1(msg->pos[0],0,0);
				HH_OSD_Init(msg->pos[0],4,msg->pos[0],0);  
				HH_OSD_Destory1(msg->pos[1],0,0);
				HH_OSD_Init(msg->pos[1],4,msg->pos[1],1);
				move_complete_cmd = msg->pos[1] + 100;
			}
			if(witch_mode_4 == 141)
			{
				HH_OSD_Destory1(msg->pos[0],0,0);
				HH_OSD_Init(msg->pos[0],4,msg->pos[0]+4,0);  
				HH_OSD_Destory1(msg->pos[1],0,0);
				HH_OSD_Init(msg->pos[1],4,msg->pos[1]+4,1);
				move_complete_cmd = msg->pos[1] + 100 + 4;
			}
			if(witch_mode_4 == 142)
			{
                if(msg->pos[1] == 0){
				    HH_OSD_Destory1(msg->pos[0],0,0);
				    HH_OSD_Init(msg->pos[0],4,msg->pos[0]+5,0);  
				    HH_OSD_Destory1(msg->pos[1],0,0);
				    HH_OSD_Init(msg->pos[1],4,msg->pos[1]+5,1);
				    move_complete_cmd = msg->pos[1] + 100 + 5;
                }
                if(msg->pos[1] == 2){
				    HH_OSD_Destory1(msg->pos[0],0,0);
				    HH_OSD_Init(msg->pos[0],4,msg->pos[0]+4,0);  
				    HH_OSD_Destory1(msg->pos[1],0,0);
				    HH_OSD_Init(msg->pos[1],4,msg->pos[1]+6,1);
				    move_complete_cmd = msg->pos[1] + 100 + 6;
                }
			}
			break;
		default:
			break;
	}
	memset(logBuf,0,sizeof(logBuf));
	localtime_r(&tv.tv_sec,&tm);
	fprintf(fp,"%04d-%02d-%02d %02d:%02d:%02d  ",tm.tm_year+1900,tm.tm_mon+1,tm.tm_mday,tm.tm_hour,tm.tm_min,tm.tm_sec);
	sprintf(logBuf,"begin msg->pos[0] = %-3d    msg->pos[1] = %-3d \n",msg->pos[0],msg->pos[1]);
	fputs(logBuf,fp);
	fflush(fp);
	msg->pos[0] = msg->pos[1];
	*execute_cmd = 0;
	pthread_mutex_unlock(&mutex);
	return move_complete_cmd;
}



/*
						上 53
	水印切换	左  52			右 50
						下 51
		
	四屏切换	140 指挥车		141 战车 	142 camer						
	
	切到9		190		0-9 VO
		
*/





int response_command(int cmd,MESSAGE *msg)
{
	//MESSAGE *msg = (MESSAGE *)arg;
	char recv_cmd[32];
	int ret = 0,i=0;
	int cmd_tmp = cmd;
	int execute_cmd = 0;	
	FILE *fp = INIT_Log("/home/video_switch.log");
	if(fp == NULL)
	{
		printf("fp null\n");
		return ;
	}

	switch(cmd_tmp){
	//遥控器//char * Key_Str_Name[12] = {"多屏 ","四屏 ","上 ","左 ","确定 ","右 ","下 ","测试 ","返回 ","本车 ","战车 ","摄像头 "};
	case 0x00: cmd = 200; 	break;		//多屏	
	case 0x01: cmd = 140; 	break;		//四屏
	case 0x02: cmd = 53; 	break;		//上
	case 0x03: cmd = 52; 	break;		//左
	case 0x04: cmd = 60;	break;		//确定 
	case 0x05: cmd = 50;	break;		//右
	case 0x06: cmd = 51;	break;		//下
	case 0x07: cmd = 201;	break;		//测试 
	case 0x08: cmd = 202;	break;		//返回 
	case 0x09: cmd = 140;	break;		//本车 
	case 0x0a: cmd = 141;	break;		//战车 
	case 0x0b: cmd = 142;	break;		//摄像头
	
	//按键	
	case 0x80: cmd = 202;	break;		//返回
	case 0x81: cmd = 60;	break;		//确认	
	case 0x82: cmd = 204;	break;		//逆时针
	case 0x83: cmd = 203;	break;		//顺时针
	case 0x84: cmd = 142;	break;		//摄像头
	case 0x85: cmd = 200;	break;		//多屏 
	case 0x86: cmd = 141;	break;		//战车 
	case 0x87: cmd = 140;	break;		//本车 
	default:
		printf("cmd error!\n");
		break;
	}
	
	
	//set_cmd返回0表示按下的是确定按钮  如果不是确定按钮就一定要有一个合理的返回值 190-205 140-142 100-109
	ret = set_cmd(cmd,msg,fp,&execute_cmd);
    printf("rbbbbbbbbbbbbbbbbbbbbddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddd ret=%d execute_cmd=%d cmd_confirm=%d previous_mode_all=%d previous_tow_mode=%d\n",ret,execute_cmd,cmd_confirm,previous_mode_all,previous_tow_mode);
	
	
	if(ret != 0)
		cmd_confirm = ret;
	
	if(((cmd_confirm >= 140) && (cmd_confirm <= 149))\
		|| ((cmd_confirm >= 100) && (cmd_confirm <= 109))\
		||((cmd_confirm >= 190) && (cmd_confirm <= 205))){}
	else{
		return -1;		
	} 
	
	//execute_cmd响应命令
	if(execute_cmd == 1)	
	{
		//VO是10个通道   码流是9路
		pthread_mutex_lock(&mutex);
		for(i=0;i<MAX_VOCHN;i++) 	
		{			
			msg->bck_not_show[i] = -1;		
		}
		do_switch_all(cmd_confirm,fp);
		msg->pos[0] = 0;
		msg->pos[1] = 0;	
		cmd_response = 0;	

		/**********************************
		*清空缓冲区标志停止数据发送标志
		***********************************/
		//PRINTF_PRT("bck_not_show cmd_confirm = %d\n",cmd_confirm);
		if(cmd_confirm == 140)
		{
			for(i=4;i<MAX_VOCHN-1;i++) 	//VO是10个通道   码流是9路
			{
				msg->bck_not_show[i] = 1;	//4 5 6 7 8 为1
			}
		}
		if(cmd_confirm == 141)
		{
			for(i=0;i<MAX_VOCHN-6;i++)		//VO是十个通道 MAX_VOCHN-4是正常显示的4-7   MAX_VOCHN-4-1再减去摄像头一路  MAX_VOCHN-4-1-1再减去VO多的一路
			{
				msg->bck_not_show[i] = 1;	// 0 1 2 3  为1
			}
			msg->bck_not_show[8] = 1;	 	//8=1
		}
		if(cmd_confirm == 142)
		{
			for(i=0;i<5;i++)				// 0 1 2 3 4 为1
			{
				msg->bck_not_show[i] = 1;
			}
			//msg->bck_not_show[8] == 1;	 
		}
		
		//命令为切换至单屏
		if((cmd_confirm >= 100)&& (cmd_confirm <= 109) )
		{
			for(i=0;i<MAX_VOCHN-1;i++)
			{
				if(cmd_confirm%10 == i)
					continue;
				msg->bck_not_show[i] = 1;					
			}
		}
		if(cmd_confirm == 201)
		{
			for(i=0;i<MAX_VOCHN-1;i++)
				msg->bck_not_show[i] = 1;
		}
		pthread_mutex_unlock(&mutex);
		for(i=0; i< MAX_VOCHN-1; i++ )
		{
			PRINTF_PRT(" not show %d\n",msg->bck_not_show[i]);
		}	
	}
	return 0;
}








/* 键值：按下时，0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07 */
/* 键值：松开时，0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87 */

struct pin_desc {
	unsigned int key_pin;
	unsigned char key_fall_val;
	unsigned char key_raise_val;
};
struct pin_desc pins_desc[8] = {
    {0x0,0x00,0x80},
    {0x1,0x01,0x81},
    {0x2,0x02,0x82},
    {0x3,0x03,0x83},
    {0x4,0x04,0x84},
    {0x5,0x05,0x85},
    {0x6,0x06,0x86},
    {0x7,0x07,0x87},
};

int Slot_Key_Map(unsigned char key_val)
{
	int i = 0;
	int index = 0;
	for(i = 0; i < 8; i++) {
		if(pins_desc[i].key_fall_val == key_val) return index;	
		index++;	
	}
	for(i = 0; i < 8; i++) {
		if(pins_desc[i].key_raise_val == key_val) return index;
		index++;
	}
	return index;
}


#define Fake_MAGIC 'g'
#define Fake_IOC_RENC _IOR(Fake_MAGIC,0,int)

unsigned char Key_Value[12] = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B};
unsigned char Key_Opp_Value[12] = {0xff,0xfe,0xfd,0xfc,0xfb,0xfa,0xf9,0xf8,0xf7,0xf6,0xf5,0xf4};
char * Key_Str_Name[12] = {"多屏 ","四屏 ","上 ","左 ","确定 ","右 ","下 ","测试 ","返回 ","本车 ","战车 ","摄像头 "};

unsigned char NecPrivate[4] = {0};

int KeyValueSlot(unsigned char value)
{
   int i = 0;
   int index = 0;
   for(i = 0; i < 12; i++) {
	if(value == Key_Value[i])
	{
		index = i;
		break;
	}
   }
   if( index >= 0 || index <= 11) return index;
   else return 0xff;
}

int KeyOppSlot(unsigned char value)
{
   int i = 0;
   int index = 0;
   for(i = 0; i < 12; i++) {
	if(value == Key_Opp_Value[i])
	{
		index = i;
		break;
	}
   }
   if( index >= 0 || index <= 11) return index;
   else return 0xff;
}



void signal_iic_handler(int signum)
{
	int index = 0; 
	unsigned char KeyValue = 0;
	read(msg->fd_iic,&KeyValue,1);
	//printf("KeyValue = %x\r\n",KeyValue);
	if(KeyValue != 0xff)
	{
		index = KeyValueSlot(KeyValue);
		printf("%s -------> key[%02x]\r\n",Key_Str_Name[index],Key_Value[index]);
		if(response_command(index ,msg) < 0)
			printf("commond exec error!!\n");
	}
}

void signal_key_handler(int signum)
{
	unsigned char key_val;
	int index = 0;
	read(msg->fd_key,&key_val,1);
	index = Slot_Key_Map(key_val);
	
	if( index >= 8 ) 
    {
	    printf("key%d ----> 弹起: 0x%x\r\n",pins_desc[index-8].key_pin,key_val);
		//逻辑代码
		if(response_command(key_val ,msg) < 0)
			printf("commond exec error!!\n");
	}
    else 
    {
	    //printf("key%d ----> 按下: 0x%x\r\n",pins_desc[index].key_pin,key_val);
	}
}


void *Media_Common_Receive_thread(void *arg)
{
	msg = (MESSAGE *)arg;
	int i = 0,j = 0;
	
	//struct ztGpioPrivate ztPrivate;
	//const unsigned int LED_NUM[4] = {
	//	LED1_NUM,LED2_NUM,LED3_NUM,LED4_NUM
	//};
	while(1)
	{
		
		pthread_mutex_lock(&mutex);
		pthread_cond_wait(&cond,&mutex);
		pthread_mutex_unlock(&mutex);
		
		pthread_mutex_lock(&mutex);
		led_ctl = 1;
		pthread_mutex_unlock(&mutex);
		//memset(&ztPrivate,0,sizeof(struct ztGpioPrivate));
		//ztInitGpioMap(&ztPrivate);
		//for(i=0 ; i< 4 ; i++){
		//	ztGpioExport(LED_NUM[i]);
		//	ztGpioSetDir(LED_NUM[i],GPIO_OUT);
		//}
	
		for(j=0; j < 4; j++){
			for(i = 0; i < 4;i++){
				if(j==0)
					ztGpioSetLevel(LED_NUM[i],1);
				if(j==1)
					ztGpioSetLevel(LED_NUM[i],0);
				if(j==2)
					ztGpioSetLevel(LED_NUM[i],1);
				if(j==3){
					if((i == 0) || (i==2))
					{
						ztGpioSetLevel(LED_NUM[i],1);
					}
					if((i == 1) || (i == 3 ))
					{
						ztGpioSetLevel(LED_NUM[i],0);
					}
				}					
			}
			sleep(1);
		}
		
		//for(i = 0; i < 4;i++){
		//	ztGpioUnexport(LED_NUM[i]);
		//}
		//ztReleaseGpioMap(&ztPrivate);
		pthread_mutex_lock(&mutex);
		led_ctl = 0;
		pthread_mutex_unlock(&mutex);

	}
    return ;
}


void *set_light_thread(void *arg)
{
#if 1
	msg = (MESSAGE *)arg;
	int i = 0;
	int vedio_status = 0, display = 0;

	char buf[8];
	FILE *fp;
			memset(&ztPrivate,0,sizeof(struct ztGpioPrivate));
			ztInitGpioMap(&ztPrivate);
			for(i=0 ; i< 4 ; i++){
				ztGpioExport(LED_NUM[i]);
				ztGpioSetDir(LED_NUM[i],GPIO_OUT);
			}
	
	while(1){
		if(led_ctl == 0){
			fp = fopen("/led_ctl.txt","ro");
			if(!fp)
			{
				sleep(100);
				continue;
			}
			
			ztGpioSetLevel(LED_NUM[2],1);	
			ztGpioSetLevel(LED_NUM[1],0);
			
			fgets(buf,sizeof(buf),fp);
			if(strstr(buf,"ok") != NULL){
				ztGpioSetLevel(LED_NUM[0],1);
				ztGpioSetLevel(LED_NUM[3],0);
			}
			if(strstr(buf,"no") != NULL){
				ztGpioSetLevel(LED_NUM[0],0);
				ztGpioSetLevel(LED_NUM[3],1);
			}
		}	
		sleep(3);
	}
				for(i = 0; i < 4;i++){
				ztGpioUnexport(LED_NUM[i]);
			}
			ztReleaseGpioMap(&ztPrivate);
#endif
}


