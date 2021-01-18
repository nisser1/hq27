/**************************************************************************
 * 	FileName:		osd.c
 *	Description:	OSD Process(字幕叠加)
 *	Copyright(C):	2006-2008 HHDigital Inc.
 *	Version:		V 1.0
 *	Author:			ChenYG
 *	Created:		2006-08-06
 *	Updated:
 *
 **************************************************************************/
#include <stdio.h>
#include <stdlib.h>

#include <time.h>
#include <string.h>
#include <sys/time.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>

//#include "global_config.h"
#include "hh_osd.h"
#include "hh_osd_api.h"
//#include "toolbox.h"
//#include "syslog.h"



HH_VIDEO_RESOLUTION ipc_conf;		 //RESOLUTION--分辨洛??
VIDEO_OSD_STATUS *pvideo_osd_status; //osd状态信息; 用来控制OSD的开关


int UART_TEMP_FLAG;

//osd的logo
HH_OSD_LOGO		*pGOsdLog[HH_MAX_OSD_NUM];	//={0}//做多支持多少路OSD  我们的项目里面只有2路....    HH_MAX_OSD_NUM=48


//两种方法是中文和英文的; 模式是英文; 
#if 0
static char *S_WEEK[7]   = {"星期日", "星期一", "星期二", "星期三",
							"星期四", "星期五", "星期六"};
#else
static char *S_WEEK[7]   = {"SUN", "MON", "TUE", "WED",
							"THU", "FRI", "SAT"};
#endif
pthread_mutex_t gOsdMutex   = PTHREAD_MUTEX_INITIALIZER;
unsigned int           nOsdRebuild = 0;

//获取画logo的handle
//算法里面可能有很多的osd; 那个osd对应的hanle是多少.......
HH_OSD_LOGO* HH_OSD_GetLogoHandle(int nCh, int bMin, int nOsdType)
{
	int nIndex      = 0;
	int nEncMaxType = 3;

	if (nCh < 0 ||  bMin < 0 || bMin >= nEncMaxType
		|| nOsdType < 0 || nOsdType >= HH_MAX_OSD_PER_GROUP)
	{
		COMM_SYSLOG(LOG_ERR,"HH_OSD_GetLogoHandle param err\n");
		return NULL;
	}
	//算法里面可能有很多的osd; 那个osd对应的hanle是多少; 根据osd所在chn去
	//计算出handle; 其他地方都用这个算法得出handle不会重复; 
	nIndex = nCh * nEncMaxType *  HH_MAX_OSD_PER_GROUP +
			 bMin * HH_MAX_OSD_PER_GROUP + nOsdType;

	if (nIndex >= HH_MAX_OSD_NUM)
	{
		COMM_SYSLOG(LOG_ERR,"HH_OSD_GetLogoHandle index err %d\n", nIndex);
		return NULL;
	}
	//COMM_SYSLOG(LOG_ERR,"\nHH_OSD_GetLogoHandle nCh=%d,bMin=%d,nOsdType=%d,nIndex=%d,pGOsdLog[%d]=%d\n",nCh,bMin,nOsdType,nIndex,nIndex,pGOsdLog[nIndex]);
	return 	pGOsdLog[nIndex];
}
HH_OSD_LOGO* HH_OSD_GetLogoHandle1(int nCh, int bMin, int nOsdType)
{
	/*int nIndex      = 0;
	int nEncMaxType = 3;

	if (nCh < 0 ||  bMin < 0 || bMin >= nEncMaxType
		|| nOsdType < 0 || nOsdType >= HH_MAX_OSD_PER_GROUP)
	{
		COMM_SYSLOG(LOG_ERR,"HH_OSD_GetLogoHandle param err\n");
		return NULL;
	}
	//算法里面可能有很多的osd; 那个osd对应的hanle是多少; 根据osd所在chn去
	//计算出handle; 其他地方都用这个算法得出handle不会重复; 
	nIndex = nCh * nEncMaxType *  HH_MAX_OSD_PER_GROUP +
			 bMin * HH_MAX_OSD_PER_GROUP + nOsdType;

	if (nIndex >= HH_MAX_OSD_NUM)
	{
		COMM_SYSLOG(LOG_ERR,"HH_OSD_GetLogoHandle index err %d\n", nIndex);
		return NULL;
	}*/
	//COMM_SYSLOG(LOG_ERR,"\nHH_OSD_GetLogoHandle nCh=%d,bMin=%d,nOsdType=%d,nIndex=%d,pGOsdLog[%d]=%d\n",nCh,bMin,nOsdType,nIndex,nIndex,pGOsdLog[nIndex]);
	return 	pGOsdLog[nCh];
}
float HH_OSD_GetOsdRateX(int nCh, int nMinEnc)
{
	int	nEncWidth  = 0;
	int	nEncHeight = 0;
	int nEncType   = 0;

	//HH_VENC_GetVEncSize(nCh, nMinEnc, &nEncWidth, &nEncHeight, &nEncType);

	if(nMinEnc == 0)
	{
		nEncWidth = 1280 ;

	}
	else if(nMinEnc == 1)
	{
		//nEncWidth = 640 ;
		nEncWidth = 960 ;

	}
	//COMM_SYSLOG(LOG_ERR,"nEncWidth = %d" , nEncWidth);
    return (float)nEncWidth / 640;
}
//这个代码是从整体框架精简的; 可能代码不全...
float HH_OSD_GetOsdRateY(int nCh, int nMinEnc)
{
	int	nEncWidth  = 0;
	int	nEncHeight = 0;
	int nEncType   = 0;

	if(nMinEnc == 0)
	{
		 return (float)3;

	}
	else if(nMinEnc == 1)
	{
		return (float)1;

	}
}

/*设置获得OSD坐标原点.....
nCh			---通道数目
nEnc		--VO通道号
osdType		---OSD区域的个数 这里是2个区域(标题和时间  不包含静态的左下角和右下角)  2个osd区域  0 1
*/
int HH_OSD_GetOrg(int nCh, int nEnc, int osdType, HI_OSD_ORG  *pOsdOrg)
{
	int	nEncWidth  = 0;
	int	nEncHeight = 0;

	if (pOsdOrg == NULL)
	{
		COMM_SYSLOG(LOG_ERR,"HH_OSD_GetOrg Param Err\n");
		return -1;
	}
	if(nEnc == 0)							//设置venc编码通道0的时间和标题坐标
	{
		switch(osdType)
		{
			case 0:			//TIME坐标是在左边; 
				pOsdOrg->nOrgX = 0	;
				pOsdOrg->nOrgY = 0 ;
				break;

			case 9:			//左上角标题的坐标
				pOsdOrg->nOrgX = 130;
				pOsdOrg->nOrgY	= 100;
				break;
			case 10:
				pOsdOrg->nOrgX = 700;
				pOsdOrg->nOrgY	= 350;
				break;
			case 4:
				pOsdOrg->nOrgX = 400;
				pOsdOrg->nOrgY	= 150;
				break;
		}

	}else if(nEnc == 1)						//设置venc编码通道1的时间和标题坐标
	{
		switch(osdType)
		{
			case HH_OSD_TYPE_TIME:
				pOsdOrg->nOrgX = 0	;
				pOsdOrg->nOrgY = 0 ;
				break;

			case HH_OSD_TYPE_TITLE:
				pOsdOrg->nOrgX = 0;
				pOsdOrg->nOrgY	= ipc_conf.height - 32;
				break;
		}
	}
	//COMM_SYSLOG(LOG_DEBUG,">>>>>pOsdOrg->nOrgX = %d pOsdOrg->nOrgY = %d \n ",pOsdOrg->nOrgX ,pOsdOrg->nOrgX);
	return 0;
}

//获取颜色
int HH_OSD_GetColor(int nCh, HI_OSD_COLOR *pOsdColor)
{
	if (nCh < 0 || pOsdColor == NULL)
	{
		COMM_SYSLOG(LOG_ERR,"HH_OSD_GetColor Param Err\n");
		return -1;
	}

	pOsdColor->bgAlpha = 0;									//背景设为透明
	pOsdColor->fgAlpha = 0x7F;              				//前景不透明
	//黑色
	if (pvideo_osd_status->nOsdColor == 1)
	{
		pOsdColor->fgColor  = HH_OSD_COLOR_BLACK;
		pOsdColor->edgeColor= HH_OSD_COLOR_WHITE;
		pOsdColor->selColor = HH_OSD_COLOR_WHITE;
	}
	//黄色
	else if (pvideo_osd_status->nOsdColor == 2)
	{
		pOsdColor->fgColor  = HH_OSD_COLOR_YELLOW;			//背景是黄
		pOsdColor->edgeColor= HH_OSD_COLOR_BLACK;			//边沿是黑
		pOsdColor->selColor = HH_OSD_COLOR_WHITE;			//选中是白
	}
	//红色
	else if (pvideo_osd_status->nOsdColor == 3)
	{
		pOsdColor->fgColor  = HH_OSD_COLOR_RED;
		pOsdColor->edgeColor= HH_OSD_COLOR_BLACK;
		pOsdColor->selColor = HH_OSD_COLOR_WHITE;
	}
	//蓝色
	else if (pvideo_osd_status->nOsdColor == 4)
	{
		pOsdColor->fgColor  = HH_OSD_COLOR_BLUE;
		pOsdColor->edgeColor= HH_OSD_COLOR_BLACK;
		pOsdColor->selColor = HH_OSD_COLOR_WHITE;
	}
	else
	{
		pOsdColor->fgColor  = HH_OSD_COLOR_WHITE;
		pOsdColor->edgeColor= HH_OSD_COLOR_BLACK;
		pOsdColor->selColor = HH_OSD_COLOR_BLACK;
		pOsdColor->bgColor = 64;
	}
	pOsdColor->bgColor = pOsdColor->fgColor; //背景设为透明 颜色随便写
	return 0;
}
//获取菜单的颜色..
int HH_OSD_GetMenuColor(int nCh, HI_OSD_COLOR *pOsdColor)
{
	if (nCh != 0 || pOsdColor == NULL)
	{
		COMM_SYSLOG(LOG_ERR,"HH_OSD_GetMenuColor Param Err\n");
		return -1;
	}

	pOsdColor->bgAlpha = 0x7F;					//背景设为透明
	pOsdColor->fgAlpha = 0x7F;					//前景不透明
	
	pOsdColor->fgColor  = HH_OSD_COLOR_WHITE;
	pOsdColor->edgeColor= HH_OSD_COLOR_BLACK;
	pOsdColor->selColor = HH_OSD_COLOR_YELLOW;

	//pOsdColor->bgColor = pOsdColor->fgColor; 		//背景设为透明 颜色随便写
	pOsdColor->bgColor = HH_OSD_COLOR_BLACK;
	return 0;
}

//获取时间显示的内容..
int HH_OSD_GetTimeTitle(int nCh, int bMin, char *pTitle)
{
	//准备开始系统调用时间函数结构体
	time_t timep;
	struct tm *pLocalTime;
	//返回1970-1-1, 00:00:00以来经过的秒数?
	time(&timep);
	//将时间数值变换成本地时间，考虑到本地时区和夏令时标志;
	pLocalTime = localtime(&timep);					//当亲的时间

////////////////////////////////////////////////////////////////////////////////////////////
	
	//追远源码--->>>tstDateOSD.
	if(pvideo_osd_status->stDateOSD.Show ||		//自己之前设置的日期开关
	   pvideo_osd_status->stTimeOSD.Show ||		//自己之前设置的时间开关
	   pvideo_osd_status->stWeekOSD.Show )		//自己之前设置的星期开关
	{
		if (pvideo_osd_status->stDateOSD.Show )	//日期是必须要显示的...
		{
		      //nOsdPrefer--代表不同国家的year-month-day 显示方式..
			if (pvideo_osd_status->nOsdPrefer == 0){    //1900 
			//写到 pTitle
                		sprintf(pTitle,"%04d-%02d-%02d",1900+pLocalTime->tm_year,1+pLocalTime->tm_mon, pLocalTime->tm_mday) ;
			}else if (pvideo_osd_status->nOsdPrefer == 1){		//不同的国家/年月日 日月年
				sprintf(pTitle,"%02d-%02d-%04d",1+pLocalTime->tm_mon,pLocalTime->tm_mday, 1900+pLocalTime->tm_year) ;
			}else{
				sprintf(pTitle,"%02d-%02d-%04d",pLocalTime->tm_mday,1+pLocalTime->tm_mon, 1900+pLocalTime->tm_year) ;
			}
		}

		if(pvideo_osd_status->stTimeOSD.Show )		//秒????
		{
			sprintf(pTitle,"%s %02d:%02d:%02d %s",pTitle, pLocalTime->tm_hour, pLocalTime->tm_min, pLocalTime->tm_sec, S_WEEK[pLocalTime->tm_wday % 7] ) ;
		}
		//week是不工作的; 显示的是没有week的
		//if(pvideo_osd_status->stWeekOSD.Show )
		//{
		//	sprintf(pTitle,"%s ",pTitle, S_WEEK[curTime.week % 7]);
		//}
	}
	return 0;
}

int HH_OSD_GetFrameTitle(int nCh, int bMin, char *pTitle)
{
    //extern VideoEnc_Info _VideoEncInfo[MAX_VIDEO_NUM];
	bMin = bMin % 2;

	if(pvideo_osd_status->stBitrateOSD.Show)
	{
	//
	}
	return 0;

}


int HH_OSD_GetPTZTitle(int nCh, int bMin, char *pTitle)
{

	return 0;
}
int HH_OSD_GetChNameTitle(int nCh, int bMin,char *pTitle)
{
    if(pvideo_osd_status->stTitleOSD.Show)
    {
		sprintf(pTitle,"%s",pvideo_osd_status->Title);
		//printf("$$$$$$$$$######pTitle[%s]\n",pTitle);

    }
	return 0;
}

int HH_OSD_GetTitle(int nCh, int bMin, int nOsdType, char *pTitle)
{
	if (nOsdType == HH_OSD_TYPE_TIME)
	{
	       //获取时间显示的内容..
		HH_OSD_GetTimeTitle(nCh, bMin, pTitle);// time
	}
	else if (nOsdType == HH_OSD_TYPE_TITLE)
	{
		//获取标题显示的内容..  之前自己设置的ZIOTLAB
		HH_OSD_GetChNameTitle(nCh, bMin, pTitle);//字符
	}

	return 0;
}
//获得字符间隔
int HH_OSD_GetFontInt(int nCh, int bMin, int nOsdType)
{
	if (nOsdType == HH_OSD_TYPE_TITLE)
	{
		return 5;
	}
	return 5;
}
//Show
int HH_OSD_GetShow(int nCh, int bMin, int nOsdType)
{
	if (nOsdType == HH_OSD_TYPE_TIME)
	{
		return (pvideo_osd_status->stTimeOSD.Show ||
				pvideo_osd_status->stDateOSD.Show ||
				pvideo_osd_status->stWeekOSD.Show ||
				pvideo_osd_status->stBitrateOSD.Show);
	}
	else if (nOsdType == HH_OSD_TYPE_TITLE)
	{
		return pvideo_osd_status->stTitleOSD.Show;
	}

	return 0;
}
//show刷新
int HH_OSD_Show_Refresh(int nCh, int nMinEnc, int osdType)
{
	HH_OSD_LOGO *pOsdLogo    = NULL;
	int          showsStatus = 0;
	if (nCh < 0  ||  nMinEnc < 0 || nMinEnc > 2
		|| osdType < 0 || osdType >= HH_MAX_OSD_PER_GROUP)
	{
		COMM_SYSLOG(LOG_ERR,"HH_Check_Show_Refresh param err\n");
		return -1;
	}
	//----------获取handle
	if ((pOsdLogo = HH_OSD_GetLogoHandle(nCh, nMinEnc, osdType)) == NULL)
	{
		COMM_SYSLOG(LOG_ERR,"HH_Check_Show_Refresh Get Handle Err\n");
		return -2;
	}
	
	showsStatus = HH_OSD_GetShow(nCh, nMinEnc, osdType);
	if (showsStatus != pOsdLogo->nStatus)
	{
		//根据传参show为true或者fllse来开启或者关闭osd显示; 
		HI_OSD_Set_Show(pOsdLogo, showsStatus);
		pOsdLogo->nStatus = showsStatus; //2010-07-12 修改不能隐藏的问题
	}
	return 0;
}
 
//osdType = HH_OSD_TYPE_TIME   时间刷新....
//osdType = HH_OSD_TYPE_TITLE  标题刷新....
int HH_OSD_ChName_Refresh(int nCh, int nMinEnc, int osdType)
{
	HH_OSD_LOGO *pOsdLogo                 = NULL;
	char   szTitle[HH_MAX_TITLE_NUM]= {0};
	int          nNeedReCreate            = 0;
	int          ret                      = 0;

	if (nCh < 0  ||  nMinEnc < 0 || nMinEnc > 2)
	{
		COMM_SYSLOG(LOG_ERR,"HH_OSD_ChName_Refresh param err\n");
		return -1;
	}
      //来获取要操作的vergion的handle....
      //算法里面可能有很多的osd; 那个osd对应的hanle是多少
	if ((pOsdLogo = HH_OSD_GetLogoHandle(nCh, nMinEnc, osdType)) == NULL)
	{
		//COMM_SYSLOG(HH_LOG_ERR,"HH_OSD_ChName_Refresh Get Handle Err nCh=%d,osdType=%d\n", nCh, osdType);
		return -2;
	}

	memset(szTitle, 0, sizeof(szTitle));

	//获取要显示的时间时间内容或者标题内容
	//osdType = HH_OSD_TYPE_TIME   时间刷新....
	//osdType = HH_OSD_TYPE_TITLE  标题刷新....
	HH_OSD_GetTitle(nCh, nMinEnc, osdType, szTitle);
	//对比现在显示的和刚才显示的是否一样..

	if (strncmp(szTitle, pOsdLogo->szOsdTitle, HH_MAX_TITLE_NUM) != 0 ||
		//ipconfig里卖弄的大小和posdlogo里面记录的大小是不是一样的
		HI_OSD_Check_ImaSizeChange(pOsdLogo))
	{
		memcpy(pOsdLogo->szOsdTitle, szTitle, HH_MAX_TITLE_NUM - 1);
		nNeedReCreate |= HI_OSD_Parse_OsdTitle(pOsdLogo);
		nNeedReCreate |= HI_OSD_Get_BmpSize(pOsdLogo);//上节课调用过..

		if (nNeedReCreate)		//图片发生了跟改..
		{
			HI_OSD_Destroy(pOsdLogo);
			//COMM_SYSLOG(LOG_ERR,"111111111111111111111111111111111111111111111111\n");
			if ((ret = HI_Create_Osd_Reg(pOsdLogo)) != 0)
			{
				COMM_SYSLOG(LOG_ERR,"HH_ChName_Refresh Create Reg Err %x\n", ret);
				return ret;
			}
			//刷新osd显示的左上角坐标的xy刷新后信息没有变; 但是坐标变了; 
			HI_OSD_Refresh_Org(pOsdLogo);
			//HI_OSD_Refresh_Color(pOsdLogo);
		}
		// 1--根据posdlogo中的osd的内容,在内存中生成bmp图片数据(不是真正的BMP图片); 
		// 2--把内存中生成的bmp位图拷贝至内部画布显示
		if ((ret = HI_OSD_Create_Bitmap(pOsdLogo)) != 0)
		{
			COMM_SYSLOG(LOG_ERR,"HH_ChName_Refresh Create Bitmap Err %x\n", ret);
			return ret;
		}
		//真正的开始显示的.....
		HH_OSD_Show_Refresh(nCh, nMinEnc, osdType);
	}


	return 0;
}
//用来显示标题左上角
int HH_OSD_ChnAllName_Refresh(int nCh)	
{
	int nEncType = 0;

	if (nCh < 0 )
	{
		COMM_SYSLOG(LOG_ERR,"HH_OSD_ChnAllName_Refresh param err\n");
		return -1;
	}

	for (nEncType = 0; nEncType < VIDEO_NUM; ++nEncType)
	{
	      //可以通过HH_OSD_TYPE_TITLE--显示时间-------这里是title
	      //可以通过HH_OSD_TYPE_TIME---显示标题
		HH_OSD_ChName_Refresh(nCh, nEncType, HH_OSD_TYPE_TITLE);
	}
	return 0;
}
//用来显示标题
int HH_OSD_AllName_Refresh()
{
	int nCh      = 0;

	for (nCh = 0; nCh < VIDEO_NUM; ++nCh)
	{
		//用来显示标题
		HH_OSD_ChnAllName_Refresh(nCh);
		//COMM_SYSLOG(LOG_ERR,"111111\n");
	}
	return 0;
}
//显示的是时间...
int HH_OSD_ChnTime_Refresh(int nCh)
{
	int nEncType = 0;

	for (nEncType = 0; nEncType < VIDEO_NUM; ++nEncType)
	{
		//HH_OSD_TYPE_TIME   时间刷新....------这里是time
		//HH_OSD_TYPE_TITLE  标题刷新....
		HH_OSD_ChName_Refresh(nCh, nEncType, HH_OSD_TYPE_TIME);
	}
	return 0;
}

//用来显示时间
int HH_OSD_AllTime_Refresh()
{
	int nCh      = 0;

	for (nCh = 0; nCh < VIDEO_NUM; ++nCh)
	{
		//用来显示时间
		HH_OSD_ChnTime_Refresh(nCh);
	}
	return 0;
}

int HH_OSD_Org_Refresh(int nCh, int nEnc, int osdType)
{
	HH_OSD_LOGO *pOsdLogo = NULL;

	if (nCh < 0 ||  nEnc < 0 || nEnc > 2
		|| osdType < 0 || osdType >= HH_MAX_OSD_PER_GROUP)
	{
		COMM_SYSLOG(LOG_ERR,"HH_OSD_Org_Refresh param err\n");
		return -1;
	}

	if ((pOsdLogo = HH_OSD_GetLogoHandle(nCh, nEnc, osdType)) == NULL)
	{
		COMM_SYSLOG(LOG_ERR,"HH_OSD_Org_Refresh Get Handle Err\n");
		return -2;
	}

	HH_OSD_GetOrg(nCh, nEnc, osdType, &pOsdLogo->osdPos);
	return HI_OSD_Refresh_Org(pOsdLogo);
}

int HH_OSD_Color_Refresh(int nCh, int nEnc, int osdType)
{
	HH_OSD_LOGO *pOsdLogo                 = NULL;

	if (nCh < 0   ||  nEnc < 0 || nEnc > 2
		|| osdType < 0 || osdType >= HH_MAX_OSD_PER_GROUP)
	{
		COMM_SYSLOG(LOG_ERR,"HH_OSD_Color_Refresh param err\n");
		return -1;
	}

	if ((pOsdLogo = HH_OSD_GetLogoHandle(nCh, nEnc, osdType)) == NULL)
	{
		COMM_SYSLOG(LOG_ERR,"HH_OSD_Org_Refresh Get Handle Err\n");
		return -2;
	}

	HH_OSD_GetColor(nCh, &pOsdLogo->osdColor);

	return HI_OSD_Refresh_Color(pOsdLogo);
}

int HH_OSD_ChnColor_Refresh(int nCh)
{
	int nEncType = 0;

	if (nCh < 0 )
	{
		COMM_SYSLOG(LOG_ERR,"HH_OSD_ChnColor_Refresh param err\n");
		return -1;
	}

	for (nEncType = 0; nEncType < 3; ++nEncType)
	{
		HH_OSD_Color_Refresh(nCh, nEncType, HH_OSD_TYPE_TIME);
		HH_OSD_Color_Refresh(nCh, nEncType, HH_OSD_TYPE_TITLE);
	}
	return 0;
}

int HH_OSD_ChnOrg_Refresh(int nCh)
{
	int nEncType = 0;

	if (nCh < 0)
	{
		COMM_SYSLOG(LOG_ERR,"HH_OSD_ChnAllName_Refresh param err\n");
		return -1;
	}

	for (nEncType = 0; nEncType < 3; ++nEncType)
	{
		HH_OSD_Org_Refresh(nCh, nEncType, HH_OSD_TYPE_TIME);
		HH_OSD_Org_Refresh(nCh, nEncType, HH_OSD_TYPE_TITLE);
	}
	return 0;
}

int HH_OSD_ChnShow_Refresh(int nCh)
{
	int nEncType = 0;

	if (nCh < 0 )
	{
		COMM_SYSLOG(LOG_ERR,"HH_OSD_ChnAllName_Refresh param err\n");
		return -1;
	}

	for (nEncType = 0; nEncType < 2; ++nEncType)
	{
		HH_OSD_Show_Refresh(nCh, nEncType, HH_OSD_TYPE_TIME);
		HH_OSD_Show_Refresh(nCh, nEncType, HH_OSD_TYPE_TITLE);
	}
	return 0;
}
int HH_OSD_Destory(int nCh, int nEnc, int osdType)
{
	HH_OSD_LOGO *pOsdLogo = NULL;

	if (nCh < 0  ||  nEnc < 0 || nEnc > 2
		|| osdType < 0 || osdType >= HH_MAX_OSD_PER_GROUP)
	{
		COMM_SYSLOG(LOG_ERR,"HH_OSD_Org_Refresh param err\n");
		return -1;
	}

	if ((pOsdLogo = HH_OSD_GetLogoHandle(nCh, nEnc, osdType)) == NULL)
	{
		COMM_SYSLOG(LOG_ERR,"HH_OSD_Org_Refresh Get Handle Err\n");
		return -2;
	}
	
	memset(pOsdLogo->szOsdTitle, 0, HH_MAX_TITLE_NUM);			//??????
	return HI_OSD_Destroy(pOsdLogo);
}
int HH_OSD_Destory1(int nCh, int nEnc, int osdType)
{
	nEnc = 0;
	osdType =0;
	HH_OSD_LOGO *pOsdLogo = NULL;

	/*if (nCh < 0  ||  nEnc < 0 || nEnc > 2
		|| osdType < 0 || osdType >= HH_MAX_OSD_PER_GROUP)
	{
		COMM_SYSLOG(LOG_ERR,"HH_OSD_Org_Refresh param err\n");
		return -1;
	}*/

	if ((pOsdLogo = HH_OSD_GetLogoHandle1(nCh, nEnc, osdType)) == NULL)
	{
		COMM_SYSLOG(LOG_ERR,"HH_OSD_Org_Refresh Get Handle Err\n");
		return -2;
	}
	
	memset(pOsdLogo->szOsdTitle, 0, HH_MAX_TITLE_NUM);			//??????
	return HI_OSD_Destroy(pOsdLogo);
}
int HH_OSD_Rebuild(int nCh, int nEnc, int osdType)
{
	HH_OSD_LOGO *pOsdLogo = NULL;

	if (nCh < 0  ||  nEnc < 0 || nEnc > 2
		|| osdType < 0 || osdType >= HH_MAX_OSD_PER_GROUP)
	{
		COMM_SYSLOG(LOG_ERR,"HH_OSD_Org_Refresh param err\n");
		return -1;
	}

	if ((pOsdLogo = HH_OSD_GetLogoHandle(nCh, nEnc, osdType)) == NULL)
	{
		COMM_SYSLOG(LOG_ERR,"HH_OSD_Org_Refresh Get Handle Err\n");
		return -2;
	}

	HH_OSD_GetOrg(nCh, nEnc, osdType, &pOsdLogo->osdPos);
	HH_OSD_GetColor(nCh, &pOsdLogo->osdColor);
	HH_OSD_GetTitle(nCh, nEnc, osdType, pOsdLogo->szOsdTitle);
	pOsdLogo->nStatus = HH_OSD_GetShow(nCh, nEnc, osdType);

	return HI_OSD_Build(pOsdLogo);
}

int HH_OSD_ChnReBuild(int nCh)
{
	int nEncType = 0;

	if (nCh < 0 )
	{
		COMM_SYSLOG(LOG_ERR,"HH_OSD_ChnAllName_Refresh param err\n");
		return -1;
	}
	nOsdRebuild = 1;
	for (nEncType = 0; nEncType < 3; ++nEncType)
	{
		HH_OSD_Destory(nCh, nEncType, HH_OSD_TYPE_TIME);
		HH_OSD_Destory(nCh, nEncType, HH_OSD_TYPE_TITLE);
	}
	for (nEncType = 0; nEncType < 3; ++nEncType)
	{
		HH_OSD_Rebuild(nCh, nEncType, HH_OSD_TYPE_TIME);
		HH_OSD_Rebuild(nCh, nEncType, HH_OSD_TYPE_TITLE);
	}
	nOsdRebuild = 0;

	return 0;
}

int HH_OSD_All_Refresh()
{
	if (nOsdRebuild == 0)
	{
		//显示时间...
		HH_OSD_AllTime_Refresh();			//右上角....
		
		//显示标题...
		HH_OSD_AllName_Refresh();			//左上角
	}
	return 0;
}


/**************************************************************
* 函数名：	 HH_OSD_SetOsdPosDefault
* 函数介绍:   设置OSD默认坐标，当OSD重叠时调用
* 输入参数:
* 输出参数:
* 返回值:
* 作者:	 by yj 2012-09-17
**************************************************************/
//osd状态信息的设置与控制;
int HH_OSD_SetOsdPosDefault(int chn,int mode,int bigshow)//chn和cmd对应
{
#if 1
	int nCh      = 0;
	pvideo_osd_status->stDateOSD.X		= 0;    
	pvideo_osd_status->stDateOSD.Y		= 32;
	pvideo_osd_status->stDateOSD.Show   = 1;	 		// 0:不叠加日期,   1:叠加日期
	pvideo_osd_status->stTitleOSD.X		= 0;
	pvideo_osd_status->stTitleOSD.Y		= 32;
	pvideo_osd_status->stTitleOSD.Show  = 1 ;	 		// 0:不叠加标题,   1:叠加标题
	pvideo_osd_status->nOsdColor        = 5;			//osd 字体颜色
	if(bigshow == 1)
		pvideo_osd_status->nOsdColor        = 3;			//osd 字体颜色
	
	pvideo_osd_status->stTimeOSD.Show   = 1;	 		//0:不叠加日期,   1:叠加日期
	
	pvideo_osd_status->stWeekOSD.Show   = 1 ;	 		// 0:不叠加星期,   1:叠加星期
	pvideo_osd_status->nOsdPrefer       = 0 ;		 	//OSD时间显示 0 为 year-month-day
										 			    //            1 为   month-day-year
										 			    //            1 为   month-day-year
	pvideo_osd_status->stBitrateOSD.Show = 0;   		// 0:不叠加码率,   1:叠加码率
	if(strlen(pvideo_osd_status->Title) != 0)
	{
		memset(pvideo_osd_status->Title,0,CAM_TITLE_LEN_NEW);
	}	
	if((chn >= 190) && (chn <= 199 ))
	{
		chn = chn%10;
	}
	if((chn >= 100) && (chn <= 109 ))
	{
		chn = chn%10;
	}
	if(strlen(pvideo_osd_status->Title) == 0)
	{
		if(chn == 9)
		{
			sprintf(pvideo_osd_status->Title,"%s", "HDMI");	//ZIOTLAB就是要要显示的内容
		}
		else if(chn == 0)
		{
			sprintf(pvideo_osd_status->Title,"%s", "ZHC-1-1");	//ZIOTLAB就是要要显示的内容
		}
		else if(chn == 1)
		{
			sprintf(pvideo_osd_status->Title,"%s", "ZHC-1-2");	//ZIOTLAB就是要要显示的内容
		}
		else if(chn == 2)
		{
			sprintf(pvideo_osd_status->Title,"%s", "ZHC-2-1");	//ZIOTLAB就是要要显示的内容
		}
		else if(chn == 3)
		{
			sprintf(pvideo_osd_status->Title,"%s", "ZHC-2-2");	//ZIOTLAB就是要要显示的内容
		}
		else if(chn == 4)
		{
			sprintf(pvideo_osd_status->Title,"%s", "ZC-1-1");	//ZIOTLAB就是要要显示的内容
		}
		else if(chn == 5)
		{
			sprintf(pvideo_osd_status->Title,"%s", "ZC-1-2");	//ZIOTLAB就是要要显示的内容
		}
		else if(chn == 6)
		{
			sprintf(pvideo_osd_status->Title,"%s", "ZC-2-1");	//ZIOTLAB就是要要显示的内容
		}
		else if(chn == 7)
		{
			sprintf(pvideo_osd_status->Title,"%s", "ZC-2-2");	//ZIOTLAB就是要要显示的内容
		}
		else if(chn == 8)
		{
			sprintf(pvideo_osd_status->Title,"%s", "CAMER");	//ZIOTLAB就是要要显示的内容
		}
		else
		{
			printf("title error\n");
		}
	}
	if(mode == 9)
	{
		ipc_conf.width = 384;								//分辨率
		ipc_conf.height = 216;
	}
	else if(mode == 4)
	{
		ipc_conf.width = 960;								//分辨率
		ipc_conf.height = 540;		
	}
	else if(mode == 10)
	{
		ipc_conf.width = 1536;								//分辨率
		ipc_conf.height = 864;		
	}
	else if(mode == 1)
	{
		ipc_conf.width = 1920;								//分辨率
		ipc_conf.height = 1080;		
	}
#endif
	return 0;
}


int HH_OSD_SetOsdParams(int Osd_type,int Osd_Show)
{
	pvideo_osd_status->stDateOSD.Show = Osd_Show;
	pvideo_osd_status->stTimeOSD.Show = Osd_Show;
	pvideo_osd_status->stWeekOSD.Show = Osd_Show;
	pvideo_osd_status->nOsdPrefer = Osd_type;
	return 0 ;

}

int HH_OSD_unInit()
{
	free(pvideo_osd_status);
	return 0 ;
}

int HH_OSD_Init(int chn,int ischn9,int cmd,int bigshow)
{
	int 		 nCh      = 0;			//通道0
	int 		 nEncType = 0;
	int 		 osdType  = 0;
	int 		 index    = 0;
	HI_OSD_ORG	 osdOrg;
	HI_OSD_COLOR osdColor;
	char         szTitle[HH_MAX_TITLE_NUM] = {0};
	COMM_SYSLOG(LOG_DEBUG,"HH_OSD_Init Start... %d\n",VIDEO_NUM);
	//VIDEO_NUM---venc的chn，目前只有1路视频，可直接写1. 这里是chanel 0			//视频数量
	nEncType = chn;
	osdType = 0;
	//HH_MAX_OSD_PER_GROUP---OSD 区域个数,这里是2 个区域

	memset(&osdOrg,   0, sizeof(HI_OSD_ORG));
	memset(&osdColor, 0, sizeof(HI_OSD_COLOR));
	memset(szTitle,   0, sizeof(szTitle));
	
	/*
	osd状态显示的信息(标题内容 尺寸,字体颜色和时间格式)内容设置与控制:
		设置OSD 中左上角显示的标题内容"ZIOTLAB"  "ht706" 
		设置OSD 中宽高尺寸颜色
	实际项目中用来控制OSD的开关的结构体是	pvideo_osd_status = (VIDEO_OSD_STATUS *)malloc(sizeof(VIDEO_OSD_STATUS));
	*/
	pvideo_osd_status = (VIDEO_OSD_STATUS *)malloc(sizeof(VIDEO_OSD_STATUS));
	if(ischn9 == 10)
	{
		osdType = 10;
		HH_OSD_SetOsdPosDefault(cmd,10,bigshow);  //10代表1536
	}
	else if(ischn9 == 9) //9代表九屏的小屏幕
	{
		osdType = 9;
		HH_OSD_SetOsdPosDefault(cmd,9,bigshow);
	}
	else if(ischn9 == 4)
	{
		osdType = 4;
		HH_OSD_SetOsdPosDefault(cmd,4,bigshow);
	}
	else if(ischn9 == 1)
	{
		HH_OSD_SetOsdPosDefault(cmd,1,bigshow);
	}
	//获得坐标原点osdOrg--应该是获得并使之OSD中<左上角标题>和<右上角时间>的坐标原点..
	//nEncType-编码通道的数量;osdType-OSD_REGION的数量
	
	if(bigshow == 1)
	{
		HH_OSD_GetOrg(nCh, 0, osdType, &osdOrg);					//获取显示的坐标
	}
	else
	{
		HH_OSD_GetOrg(nCh, 0, 0, &osdOrg);					//获取显示的坐标
	}
		

	//设置背景<>前景<>边沿的颜色,获取颜色--osdColor
	//通过HH_OSD_SetOsdPosDefault设置的全局变量pvideo_osd_status设置获取颜色--osdColor---背景边沿的颜色>
	HH_OSD_GetColor(nCh, &osdColor);
	//获取显示的水印信息的内容
	//获得上面HH_OSD_SetOsdPosDefault()中全局变量中的水印信息..
	osdType = 0;
	HH_OSD_GetTitle(nCh, nEncType, osdType, szTitle);		
	//获得OSD转句柄index  ，转为为 0 和 1，实际使用直接 等于 osdType即可
	//得出一个index句柄,这里没有什么讲究, 只要使得每个nEncType osdType 中设置的index句柄不相同就行; 
	index = nEncType;//nEncType * HH_MAX_OSD_PER_GROUP + osdType;				//osdmenu			HH_MAX_OSD_PER_GROUP 2
	//printf("index=========%d",index);
	//printf("szTitle=========%s",szTitle);
	
	//通过上面的获取的信息:标题,颜色,水印信息,句柄,等,进行osd的创建和实例化
	//pGOsdLog[index]--代表具体多少路的osd
	//填充HH_OSD_LOGO
	//index操作标识
	if (NULL == HI_Create_Osd(index,&pGOsdLog[index], (int)nEncType, osdOrg, osdColor, szTitle, HH_OSD_GetFontInt(nCh, nEncType, osdType), HH_OSD_GetShow(nCh, nEncType, osdType)))
		COMM_SYSLOG(LOG_ERR,"HI_Create_Osd %d %d %d failed !\n", nCh,nEncType, osdType);
	
	COMM_SYSLOG(LOG_DEBUG,"HH_OSD_Init End...\n");
	printf("**********************************************************************************************\n");
	return 0;
}


