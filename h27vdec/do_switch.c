#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sample_comm.h"
#include "do_switch.h"

void do_switch_4()
{
	
}

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
	PRINTF_PRT("SWITCH BEGIN\n");
	PRINTF_PRT("previous_mode_all=%d cmd=%d\n",previous_mode_all,cmd);
	HI_S32 i, s32Ret = HI_SUCCESS;	
	char logBuf[128];
	memset(logBuf,0,sizeof(logBuf));
	
	
	struct timeval tv;
	struct tm tm;
	gettimeofday(&tv,NULL);
	localtime_r(&tv.tv_sec,&tm);
	fprintf(fp,"%04d-%02d-%02d %02d:%02d:%02d  ",tm.tm_year+1900,tm.tm_mon+1,tm.tm_mday,tm.tm_hour,tm.tm_min,tm.tm_sec);
	
	sprintf(logBuf,"previous_mode_all = %3d    cmd         =%3d \n",previous_mode_all,cmd);
	
	fputs(logBuf,fp);
	/**********************************
	*step0切换osd叠加信息
	***********************************/
	if((previous_mode_all >= 190) && (previous_mode_all <= 199))
	{
		for(i = 0 ; i < 10 ; i++ )
		{
			printf("switch osd %d\n",i);
			HH_OSD_Destory1(i,0,0);
		}
	}
	if((previous_mode_all >= 140) && (previous_mode_all <= 149))
	{
		if(previous_mode_all == 140)
		{
			for(i = 1 ; i < 5 ; i++ )
			{
				HH_OSD_Destory1(i-1,0,0);
			}
		}
		if(previous_mode_all == 141)
		{
			for(i = 5 ; i < 9 ; i++ )
			{
				HH_OSD_Destory1(i-5,0,0);
			}
		}
		if(previous_mode_all == 142)
		{
			for(i = 6 ; i < 10 ; i++ )
			{
				HH_OSD_Destory1(i-6,0,0);
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
				SAMPLE_COMM_VPSS_UnBind_VO(i+1,0, 0, i);
				PRINTF_PRT("140 = %d\n",i+1);
				if(s32Ret != HI_SUCCESS)
				{
					SAMPLE_PRT("vpss unbind vo fail for %#x!\n", s32Ret);
				}				
			}
			else if((previous_mode_all%10) == 1)//代表141
			{
				SAMPLE_COMM_VPSS_UnBind_VO(i+5,0, 0, i);
				PRINTF_PRT("141 = %d\n",i+5);
				if(s32Ret != HI_SUCCESS)
				{
					SAMPLE_PRT("vpss unbind vo fail for %#x!\n", s32Ret);
				}				
			}
			else if((previous_mode_all%10) == 2)//代表142
			{
				SAMPLE_COMM_VPSS_UnBind_VO(i+6,0, 0, i);
				PRINTF_PRT("142 = %d\n",i+6);
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
		PRINTF_PRT("previous_mode_all = %d\n",previous_mode_all);
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
	}
	
	
	/*********************************************
	*step2:停止Vo通道
	**********************************************/
	PRINTF_PRT("FLAGS1 \n");
	if((previous_mode_all/10) != (cmd/10))
	{
		if((previous_mode_all/10) == 10 )
		{
			PRINTF_PRT("STOP MODE 1\n");
			SAMPLE_COMM_VO_StopChn(0, VO_MODE_1MUX);
		}
		if((previous_mode_all/10) == 14 )
		{
			PRINTF_PRT("STOP MODE 4\n");
			SAMPLE_COMM_VO_StopChn(0, VO_MODE_4MUX);
		}
		if((previous_mode_all/10) == 19 )
		{
			PRINTF_PRT("STOP MODE 9\n");
			SAMPLE_COMM_VO_StopChn(0, VO_MODE_9MUX);
		}
	}
	/*********************************************
	*step3:开启不同Vo通道
	**********************************************/
	PRINTF_PRT("FLAGS2 \n");
	if((previous_mode_all/10) != (cmd/10))
	{
		if((cmd/10) == 10 )
		{
			PRINTF_PRT("START MODE 1\n");
			SAMPLE_COMM_VO_StartChn(0, VO_MODE_1MUX);
		}
		if((cmd/10) == 14 )
		{
			PRINTF_PRT("START MODE 4\n");
			SAMPLE_COMM_VO_StartChn(0, VO_MODE_4CUS);
		}
		if((cmd/10) == 19 )
		{
			PRINTF_PRT("START MODE 9\n");
			SAMPLE_COMM_VO_StartChn(0, VO_MODE_9CUS);
		}
	}
	/*********************************************
	*step3:vpss 和 vo 建立新的绑定关系
	**********************************************/
	PRINTF_PRT("FLAGS3 \n");
	if((cmd >= 100) && (cmd <= 109))
	{
		SAMPLE_COMM_VPSS_Bind_VO((cmd % 10),0, 0, 0);
		PRINTF_PRT("100 = %d\n",cmd%10);
		current_vdeio_display_mode = 1;
	}
	PRINTF_PRT("FLAGS4 \n");
	if((cmd >= 140) && (cmd <= 149))
	{	
		for(i = 0; i< 4; i++)
		{
			if((cmd%10) == 0)//代表140
			{
				SAMPLE_COMM_VPSS_Bind_VO(i+1,0, 0, i);
				PRINTF_PRT("140 = %d\n",i+1);
				if(s32Ret != HI_SUCCESS)
				{
					SAMPLE_PRT("vpss unbind vo fail for %#x!\n", s32Ret);
				}				
			}
			else if((cmd%10) == 1)//代表141
			{
				SAMPLE_COMM_VPSS_Bind_VO(i+5,0, 0, i);
				PRINTF_PRT("141 = %d\n",i+5);
				if(s32Ret != HI_SUCCESS)
				{
					SAMPLE_PRT("vpss unbind vo fail for %#x!\n", s32Ret);
				}				
			}
			else if((cmd%10) == 2)//代表142
			{
				PRINTF_PRT("142 b = %d %d\n",i+6,i);
				SAMPLE_COMM_VPSS_Bind_VO(i+6,0, 0, i);	
				PRINTF_PRT("142 e = %d %d\n",i+6,i);
				if(s32Ret != HI_SUCCESS)
				{
					SAMPLE_PRT("vpss unbind vo fail for %#x!\n", s32Ret);
				}				
			}			
		}
		current_vdeio_display_mode = 4;
	}
	
	//九屏幕初始化
	PRINTF_PRT("FLAGS5 \n");
	if((cmd >= 190) && (cmd <= 199))
	{
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
	}
	if((cmd >= 140) && (cmd <= 149))
	{
		if(cmd == 140)
		{
			for(i = 1 ; i < 5 ; i++ )
			{
				HH_OSD_Init(i-1,4,i,0);
			}
		}
		if(cmd == 141)
		{
			for(i = 5 ; i < 9 ; i++ )
			{
				HH_OSD_Init(i-5,4,i,0);
			}
		}
		if(cmd == 142)
		{
			for(i = 6 ; i < 10 ; i++ )
			{
				HH_OSD_Init(i-6,4,i,0);
			}
		}
	}
	if((cmd >= 100) && (cmd <= 109))
	{
		HH_OSD_Init(0,1,cmd,0);
	}

	previous_mode_all = cmd;
	if(who_in_pos9 == 9 )
	{
		previous_mode_all = 199;
	}
	if(cmd < 190)
	{
		previous_mode_all = cmd;
	}
	PRINTF_PRT("SWITCH EDN\n");
	return 0;
}





//选择水印切换在这里做
int set_cmd(char *recv_cmd,MESSAGE *msg,FILE *fp)
{
	struct timeval tv;
	struct tm tm;
	int in_cmd;
	char logBuf[128];
	if((recv_cmd[0]==1) && (recv_cmd[1]==1) && (recv_cmd[2]==1) &&\ 
				(recv_cmd[3]==1) && (recv_cmd[4]==1) && (recv_cmd[5]==1) && (recv_cmd[6]==1))
	{
		in_cmd = recv_cmd[7];
	}
	PRINTF_PRT("in_cmd = %d",in_cmd);
	//9模式下只响应 1切4的命令(3个)  切一的命令 vo9视频切1显示
	if((in_cmd >= 190) && (in_cmd <= 199))
	{
		return -1;
	}

	if(current_vdeio_display_mode == 9)
	{
		in_cmd += 5;
		if((msg->pos[0]>9) || (msg->pos[1]>9) || (msg->pos[1] < 0) || (msg->pos[0] < 0))
		{
			msg->pos[0]=0; 
			msg->pos[1]=0;
		}
	}
	if(current_vdeio_display_mode == 4)
	{
		in_cmd -= 5;
		if(msg->pos[0]>3)
			msg->pos[0]=0;
			//msg->pos[1]=0;
	}
	if(current_vdeio_display_mode == 1)
	{
		return -2;
	}
	//msg->pos[1]表示按下上下左右键位置  msg->pos[0]表示按下上下左右键之前位置
	memset(logBuf,0,sizeof(logBuf));

	gettimeofday(&tv,NULL);
	localtime_r(&tv.tv_sec,&tm);
	fprintf(fp,"%04d-%02d-%02d %02d:%02d:%02d  ",tm.tm_year+1900,tm.tm_mon+1,tm.tm_mday,tm.tm_hour,tm.tm_min,tm.tm_sec);
	sprintf(logBuf,"begin msg->pos[0] = %-3d    msg->pos[1] = %-3d \n",msg->pos[0],msg->pos[1]);
	fputs(logBuf,fp);
	switch(in_cmd)
	{
		
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
				cmd_confirm = msg->pos[1] + 190;
			}
			if(msg->pos[0] == 9){	
				msg->pos[1] = 8;
				HH_OSD_Destory1(msg->pos[0],0,0);
				HH_OSD_Init(msg->pos[0],10,who_in_pos9,0);  
				HH_OSD_Destory1(msg->pos[1],0,0);
				HH_OSD_Init(msg->pos[1],9,msg->pos[1],1);  //要变大的
				cmd_confirm = msg->pos[1] + 190;
			}
			cmd_confirm = msg->pos[1] + 190;
			break;
		case 56://向下   大于4可以向下			9可以向右下向下直接跳到0的位置
			if((msg->pos[0] > 4) && (msg->pos[0] < 9)){
				msg->pos[1] = msg->pos[1]-1;			
				HH_OSD_Destory1(msg->pos[0],0,0);		
				HH_OSD_Init(msg->pos[0],9,msg->pos[0],0);  
				HH_OSD_Destory1(msg->pos[1],0,0);
				HH_OSD_Init(msg->pos[1],9,msg->pos[1],1);  //要变大的	
				cmd_confirm = msg->pos[1] + 190;
			}
			if(msg->pos[0] == 9){//这里可能会有问题  9下来无法显示在0上
				msg->pos[1] = 0;
				HH_OSD_Destory1(msg->pos[0],0,0);
				HH_OSD_Init(msg->pos[0],10,who_in_pos9,0);  
				HH_OSD_Destory1(msg->pos[1],0,0);
				HH_OSD_Init(msg->pos[1],9,msg->pos[1],1);  //要变大的
				cmd_confirm = msg->pos[1] + 190;
			}	
			cmd_confirm = msg->pos[1] + 190;			
			break;
		case 57://向左	 1-4可以向左			4-8可以向左跳到9的位置  
			if((msg->pos[0] < 5) && (msg->pos[0] > 0)){
				msg->pos[1] = msg->pos[1]-1;
				HH_OSD_Destory1(msg->pos[0],0,0);
				HH_OSD_Init(msg->pos[0],9,msg->pos[0],0);  
				HH_OSD_Destory1(msg->pos[1],0,0);
				HH_OSD_Init(msg->pos[1],9,msg->pos[1],1); 
				cmd_confirm = msg->pos[1] + 190;
			}
			if((msg->pos[0] > 4) && (msg->pos[0] < 9)){
				msg->pos[1] = 9;
				HH_OSD_Destory1(msg->pos[0],0,0);
				HH_OSD_Init(msg->pos[0],9,msg->pos[0],0);
				HH_OSD_Destory1(msg->pos[1],0,0);
				HH_OSD_Init(msg->pos[1],10,who_in_pos9,1); //要变大的
				cmd_confirm = who_in_pos9 + 100;
			}
			break;
		case 58://向上   4-7可以向上			0-3可以向上至9的位置	
			if((msg->pos[0] > 3) && (msg->pos[0] < 8)){
				msg->pos[1] = msg->pos[1]+1;
				HH_OSD_Destory1(msg->pos[0],0,0);
				HH_OSD_Init(msg->pos[0],9,msg->pos[0],0);  
				HH_OSD_Destory1(msg->pos[1],0,0);
				HH_OSD_Init(msg->pos[1],9,msg->pos[1],1); //要变颜色的
				cmd_confirm = msg->pos[1] + 190;
			}
			if(msg->pos[0] < 4){
				msg->pos[1] = 9;
				HH_OSD_Destory1(msg->pos[0],0,0);
				HH_OSD_Init(msg->pos[0],9,msg->pos[0],0);  
				HH_OSD_Destory1(msg->pos[1],0,0);
				HH_OSD_Init(msg->pos[1],10,who_in_pos9,1);    
				cmd_confirm = who_in_pos9 + 100;			//选中9  将要将该通道的数据切成单屏幕

			}
			break;	
		//4屏幕模式
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
				cmd_confirm = msg->pos[1] + 100 +1;
			}
			if(witch_mode_4 == 141)
			{
				HH_OSD_Destory1(msg->pos[0],0,0);
				HH_OSD_Init(msg->pos[0],4,msg->pos[0]+4,0);  
				HH_OSD_Destory1(msg->pos[1],0,0);
				HH_OSD_Init(msg->pos[1],4,msg->pos[1]+4,1);
				cmd_confirm = msg->pos[1] + 100 + 4 +1;
			}
			if(witch_mode_4 == 142)
			{
				HH_OSD_Destory1(msg->pos[0],0,0);
				HH_OSD_Init(msg->pos[0],4,msg->pos[0]+5,0);  
				HH_OSD_Destory1(msg->pos[1],0,0);
				HH_OSD_Init(msg->pos[1],4,msg->pos[1]+5,1);
				cmd_confirm = msg->pos[1] + 100 + 5 +1;
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
				cmd_confirm = msg->pos[1] + 100 +1;
			}
			if(witch_mode_4 == 141)
			{
				HH_OSD_Destory1(msg->pos[0],0,0);
				HH_OSD_Init(msg->pos[0],4,msg->pos[0]+4,0);  
				HH_OSD_Destory1(msg->pos[1],0,0);
				HH_OSD_Init(msg->pos[1],4,msg->pos[1]+4,1);
				cmd_confirm = msg->pos[1] + 100 + 4 +1;
			}
			if(witch_mode_4 == 142)
			{
				HH_OSD_Destory1(msg->pos[0],0,0);
				HH_OSD_Init(msg->pos[0],4,msg->pos[0]+5,0);  
				HH_OSD_Destory1(msg->pos[1],0,0);
				HH_OSD_Init(msg->pos[1],4,msg->pos[1]+5,1);
				cmd_confirm = msg->pos[1] + 100 + 5 +1;
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
				cmd_confirm = msg->pos[1] + 100 +1;
			}
			if(witch_mode_4 == 141)
			{
				HH_OSD_Destory1(msg->pos[0],0,0);
				HH_OSD_Init(msg->pos[0],4,msg->pos[0]+4,0);  
				HH_OSD_Destory1(msg->pos[1],0,0);
				HH_OSD_Init(msg->pos[1],4,msg->pos[1]+4,1);
				cmd_confirm = msg->pos[1] + 100 + 4 +1;
			}
			if(witch_mode_4 == 142)
			{
				HH_OSD_Destory1(msg->pos[0],0,0);
				HH_OSD_Init(msg->pos[0],4,msg->pos[0]+5,0);  
				HH_OSD_Destory1(msg->pos[1],0,0);
				HH_OSD_Init(msg->pos[1],4,msg->pos[1]+5,1);
				cmd_confirm = msg->pos[1] + 100 + 5 +1;
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
				cmd_confirm = msg->pos[1] + 100 +1;
			}
			if(witch_mode_4 == 141)
			{
				HH_OSD_Destory1(msg->pos[0],0,0);
				HH_OSD_Init(msg->pos[0],4,msg->pos[0]+4,0);  
				HH_OSD_Destory1(msg->pos[1],0,0);
				HH_OSD_Init(msg->pos[1],4,msg->pos[1]+4,1);
				cmd_confirm = msg->pos[1] + 100 + 4 +1;
			}
			if(witch_mode_4 == 142)
			{
				HH_OSD_Destory1(msg->pos[0],0,0);
				HH_OSD_Init(msg->pos[0],4,msg->pos[0]+5,0);  
				HH_OSD_Destory1(msg->pos[1],0,0);
				HH_OSD_Init(msg->pos[1],4,msg->pos[1]+5,1);
				cmd_confirm = msg->pos[1] + 100 + 5 +1;
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
	return cmd_confirm;
}



/*
						上 53
	水印切换	左  52			右 50
						下 51
		
	四屏切换	140 指挥车		141 战车 	142 camer						
	
	切到9		190		0-9 VO
		
*/

void *Media_Common_Receive_thread(void *arg)
{
	MESSAGE *msg = (MESSAGE *)arg;
	char recv_cmd[32];
	int ret = 0,i=0;
	int cmd;
	FILE *fp = INIT_Log("/home/video_switch.log");
	if(fp == NULL)
	{
		printf("fp null\n");
		return ;
	}
	while(1)
	{	
		//接收解码命令  现在是从fifo接收
		ret = read(msg->fd[0],recv_cmd,RECV_CMD_BUFFSIZE);
		printf("write cmd_buf recv_cmd[7] = %d\n",recv_cmd[7]);
		if(ret < 0)
		{
			printf("read fifo cmd error\n");
			continue;
		}
		
		if((recv_cmd[7] != 60) && (recv_cmd[7] != 200) && (recv_cmd[7] != 140) && (recv_cmd[7] != 141)  && (recv_cmd[7] != 142)  && (recv_cmd[7] != 50)\
			 && (recv_cmd[7] != 51)  && (recv_cmd[7] != 52)  && (recv_cmd[7] != 53) )
		{
			continue;	 
		}
		
		pthread_mutex_lock(&mutex);
		//收到立即响应的命令 
		if((recv_cmd[7] == 60) || (recv_cmd[7] == 200) || (recv_cmd[7] == 140) || (recv_cmd[7] == 141) || (recv_cmd[7] == 142))//200是默认9屏模式
		{
			cmd_response = 1;
			if(recv_cmd[7] == 200)
				cmd = 199;
			if(recv_cmd[7] == 140){
				cmd = 140;
				witch_mode_4 = 140;
			}
			if(recv_cmd[7] == 141){
				cmd = 141;
				witch_mode_4 = 141;
			}
			if(recv_cmd[7] == 142){
				cmd = 142;
				witch_mode_4 = 142;
			}
		}
		else
		{
			//获取cmd
			cmd = set_cmd(recv_cmd,msg,fp);
			PRINTF_PRT("cmd vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv = %d\n",cmd);
		}
		if(cmd == 0)
		{
			continue;
		}
		//VO是10个通道   码流是9路
		for(i=0;i<MAX_VOCHN;i++) 	
		{			
			msg->bck_not_show[i] = -1;		
		}
		
		printf("cmd_response = %d   cmd = %d\n",cmd_response,cmd);
		//如果进过响应命令分支进入切换命令  否则不执行切换
		if(cmd_response == 1)	
		{

			do_switch_all(cmd,fp);
			msg->pos[0] = 0;
			msg->pos[1] = 0;	
			cmd_response = 0;	

			/**********************************
			*清空缓冲区标志停止数据发送标志
			***********************************/
			PRINTF_PRT("bck_not_show cmd = %d\n",cmd);
			if(cmd == 140)
			{
				for(i=4;i<MAX_VOCHN-1;i++) 	//VO是10个通道   码流是9路
				{
					msg->bck_not_show[i] = 1;	//4 5 6 7 8 为1
				}
			}
			if(cmd == 141)
			{
				for(i=0;i<MAX_VOCHN-6;i++)		//VO是十个通道 MAX_VOCHN-4是正常显示的4-7   MAX_VOCHN-4-1再减去摄像头一路  MAX_VOCHN-4-1-1再减去VO多的一路
				{
					msg->bck_not_show[i] = 1;	// 0 1 2 3  为1
				}
				msg->bck_not_show[8] = 1;	 	//8=1
			}
			if(cmd == 142)
			{
				for(i=0;i<5;i++)				// 0 1 2 3 4 为1
				{
					msg->bck_not_show[i] = 1;
				}
				//msg->bck_not_show[8] == 1;	 
			}

			//命令为切换至单屏
			if((cmd >= 100)&& (cmd <= 109) )
			{
				for(i=0;i<MAX_VOCHN;i++)
				{
					if(cmd%10 == i)
						continue;
					msg->bck_not_show[i-1] = 1;			//i-1的原因是有一个HDMI占用了0的位置		
				}
			}
			for(i=0; i< MAX_VOCHN-1; i++ )
			{
				PRINTF_PRT(" not show %d\n",msg->bck_not_show[i]);
			}
			
		}
		pthread_mutex_unlock(&mutex);
	}
	return ;
}




