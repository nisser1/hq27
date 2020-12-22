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

#include <hi_type.h>
#include "hh_osd_api.h"

//#include "global_config.h"

#include "hh_hz16lib.h"					//包含字库....
//#include "syslog.h"

extern HH_VIDEO_RESOLUTION ipc_conf;	//设计这套OSD代码跟视频源头绑定; 希望是通用的OSD代码
										//配置摄像头视频源分辨率 
										//网一个具体的平台的移植;
										//在osd代码中根据视频源头的大小
										//来对OSD进行调整....
										//视频源头是1090   OSD字体用16/12
//创建OSD
//实例化一个HH_OSD_LOGO结构体变量
HH_OSD_LOGO *HI_Create_Osd(int index, HH_OSD_LOGO **ppLog, int vencGroup,
						   HI_OSD_ORG	osdOrg, HI_OSD_COLOR osdColor,
 							char *pSzTitle, HI_S16 nFontInt, HI_U8 nShow)
{	//创建osdlogo的结构体变量
	if (NULL == (*ppLog = (HH_OSD_LOGO *)malloc(sizeof(HH_OSD_LOGO))))
	{
		COMM_SYSLOG(LOG_ERR,"HI_Create_Osd_Reg malloc osd error\n");
		return NULL;
	}
	/*下面全部都是osdlogo的结构体变量的填充*/
	//成员变量赋值.....
	(*ppLog)->vochn 		= vencGroup;	//osd 区域叠加到的编码组?????  通道号
	(*ppLog)->nStatus   	= nShow;		//好像显示赋值为0
	(*ppLog)->regHdl    	= index;		//区域索引
	(*ppLog)->pBmpBuf   	= NULL;
	(*ppLog)->nFontInt  	= nFontInt;		//字符间间隔多少个像素  1
	(*ppLog)->nSelLine  	= -1;
	
	if (pSzTitle != NULL)
		strncpy((*ppLog)->szOsdTitle, pSzTitle, HH_MAX_TITLE_NUM - 1);		//(*ppLog)->szOsdTitle osd 叠加字幕信息256字节
	
	//继续填充*ppLog...
	memcpy(&((*ppLog)->osdPos)  , &osdOrg  , sizeof(HI_OSD_ORG));			//copy起始坐标 
	memcpy(&((*ppLog)->osdColor), &osdColor, sizeof(HI_OSD_COLOR));			//copy颜色
	
	//剩余的*ppLog填充...
	//创建osd结构体/创建位图/直接显示
	if (HI_OSD_Build(*ppLog) < 0)			//----------->>>>>
		return NULL;

	return *ppLog;
}


//其实就是字符串解析的函数; 
//解析osd的字符,更新字符相关属性
int    HI_OSD_Parse_OsdTitle(HH_OSD_LOGO *pOsdLogo)
{
	int index      = 0;
	int maxLen     = 0;
	int len        = 0;
	int oldLine    = 0;
	int oldMaxLen  = 0;

	if (pOsdLogo == NULL)
	{
		COMM_SYSLOG(LOG_ERR,"HI_OSD_Parse_OsdTitle Param Err\n");
		return 0;
	}

	oldLine    = pOsdLogo->nOsdLine;				//以前的行数记录大这里...
	oldMaxLen  = pOsdLogo->nOsdTitleMaxLen;			//最长的一行的长度....
	pOsdLogo->nOsdTitleFlag[0] = 0;					//第0个元素直接填充为0
	pOsdLogo->nOsdLine = 1;							//第0行的内容的空的...
	
	printf("pOsdLogo->nOsdTitleMaxLen = %d\n",pOsdLogo->nOsdTitleMaxLen);
	memset(&pOsdLogo->nOsdTitleLen[0], 0, sizeof(unsigned char)*HH_MAX_TITLE_LINE);//实际需要可能支持多行		/*pOsdLogo->nOsdTitleMaxLen 每一行的osd的长度*/

	//一个一个字符的解析.  原始的OSD里也是以\0结尾的...  超过11行就不用管了; 不解析了
	//典型的字符串解析函数...
	while (pOsdLogo->szOsdTitle[index] != '\0' && pOsdLogo->nOsdLine <= HH_MAX_TITLE_LINE)//HH_MAX_TITLE_LINE = 11
	{
		if (pOsdLogo->szOsdTitle[index] == '\n')		//原始的OSD里是有换行的....
		{
			pOsdLogo->nOsdTitleLen[pOsdLogo->nOsdLine-1] = index - pOsdLogo->nOsdTitleFlag[pOsdLogo->nOsdLine-1];	//index当前索引减去之前的 得到当前行OSD内容长度
				
			pOsdLogo->nOsdLine    += 1;// '/n' 字符

			if (pOsdLogo->nOsdLine > HH_MAX_TITLE_LINE)
				break; 

			if (pOsdLogo->nOsdLine <= HH_MAX_TITLE_LINE)
			{
				//nOsdTitleFlag[]存的只是索引号....
				pOsdLogo->nOsdTitleFlag[pOsdLogo->nOsdLine-1] = index + 1;
			}
			if (len > maxLen)
				maxLen = len;
			len = 0;
		} 
		//ASCII大于127肯定是中文...
		else if (pOsdLogo->szOsdTitle[index] > 0x7F)			//中文占用两个8位
		{
			if (pOsdLogo->szOsdTitle[index+1] > 0x7F)			//再跳过一个.....
			{
				len   += 1;
				index += 1;
			}
		}
		else
			len += 1;
		
		++index;
	}

	if (pOsdLogo->nOsdLine <= HH_MAX_TITLE_LINE)		//合法继续处理
	{
		pOsdLogo->nOsdTitleLen[pOsdLogo->nOsdLine-1] =
			index - pOsdLogo->nOsdTitleFlag[pOsdLogo->nOsdLine-1];
		if (len > maxLen)
			maxLen = len;
	}

	if (pOsdLogo->nOsdLine > HH_MAX_TITLE_LINE)		//解析完了...后面还有就不要..不解析了...
		pOsdLogo->nOsdLine = HH_MAX_TITLE_LINE;

	pOsdLogo->nOsdTitleMaxLen = maxLen;
	if (oldLine == pOsdLogo->nOsdLine && oldMaxLen == pOsdLogo->nOsdTitleMaxLen)
		return 0;
	return 1;
}

//获取图像的大小...
//获取输出的宽高;
//这个值是在ipc_config全局变量中配置出来的
//本身是osd的东西; 跟ipc不完全绑定;
//可以吧这个OSD用在任何的视频源; 
//可以用在任何的视频源
int    HI_OSD_Get_ImaSize(int vencGroup, int *pWidth, int *pHeight)
{
	int nCh = vencGroup / 3;
	int nEnc= vencGroup % 3;

	if (nCh < 0  || nEnc < 0 || nEnc >= 3 ||
		pWidth == NULL || pHeight == NULL)
	{
		COMM_SYSLOG(LOG_ERR,"HH_OSD_Get_ImaSize Para Err\n");
		return -1;
	}
	switch(nEnc)
	{
		case 0:
		    *pWidth = ipc_conf.width;		//读取的是ipc_conf里的宽和高; 
		    *pHeight = ipc_conf.height;
		    break;

		case 1:
			*pWidth = ipc_conf.width;
		    *pHeight = ipc_conf.height;
		    break;

		case 2:
			*pWidth = ipc_conf.width;
		    *pHeight =ipc_conf.height;
		    break;

		default:
			break;
	}

	return 0;
}
//跟显示有关..
//根据传参show为true为false 来关闭开启osd的显示;
int HI_OSD_Set_Show(HH_OSD_LOGO *pOsdLogo, int show)
{
//#if defined(HI3516_ARCH) || defined(HI3518_ARCH)
    MPP_CHN_S stChn;
    RGN_CHN_ATTR_S stChnAttr;
    HI_S32 s32Ret;

    COMM_SYSLOG(LOG_DEBUG,"Enter HI_OSD_Set_Show.\n");

	if (pOsdLogo == NULL || pOsdLogo->regHdl == -1)
	{
		COMM_SYSLOG(LOG_ERR,"HI_OSD_Set_Show Param Err\n");
		return HI_FAILURE;
	}

	//COMM_SYSLOG(LOG_ERR,"HI_OSD_Set_Show == %d %d\n", pOsdLogo->vencGroup, pOsdLogo->regHdl);

    stChn.enModId  = HI_ID_VO;		//
	//#ifdef HI3518_ARCH // s32ChnId always 0 in 3518
    stChn.s32DevId = 0;
    stChn.s32ChnId = pOsdLogo->vochn;

   // PRINTF(LOG_OSD,"pOsdLogo->vencGroup[%d].\n",pOsdLogo->vencGroup);

   //------------------>>>>>
    s32Ret = HI_MPI_RGN_GetDisplayAttr(pOsdLogo->regHdl, &stChn, &stChnAttr);
    if(HI_SUCCESS != s32Ret)
    {
        COMM_SYSLOG(LOG_ERR,"%s: HI_MPI_RGN_GetDisplayAttr (%d)) failed with %#x!\n",  __FUNCTION__, pOsdLogo->regHdl, s32Ret);
        return HI_FAILURE;
    }
	
   //------------------>>>>>
	stChnAttr.bShow = show;		//决定region心事还是不显示; 
    s32Ret = HI_MPI_RGN_SetDisplayAttr(pOsdLogo->regHdl, &stChn, &stChnAttr);
    if(HI_SUCCESS != s32Ret)
    {
        COMM_SYSLOG(LOG_ERR,"%s: HI_MPI_RGN_SetDisplayAttr (%d)) failed with %#x!\n", __FUNCTION__, pOsdLogo->regHdl, s32Ret);
        return HI_FAILURE;
    }
	return HI_SUCCESS;
}
//根据视屏源头的大小配置字模的大小; 
//根据IPC_CONF配置的视频源头的大小来配置pOsdLogo中记录的OSD自摸
//字模的大小比如宽高x y;
int HI_OSD_Get_BmpSize(HH_OSD_LOGO *pOsdLogo)
{
	int	vencWith   = 0;
	int vencHeight = 0;

	HH_OSD_LOGO   tmpLogo;

	if (pOsdLogo == NULL)
	{
		COMM_SYSLOG(LOG_ERR,"HI_OSD_Get_FontSize Param Err\n");
		return 0;
	}

	memcpy(&tmpLogo, pOsdLogo, sizeof(HH_OSD_LOGO));
	
	//获取当前视频源头的宽高...
	HI_OSD_Get_ImaSize(pOsdLogo->vochn, &vencWith, &vencHeight);
	printf("current vedio width hegiht %d %d\n",vencWith,vencHeight);
	pOsdLogo->nVencFeild = 0;		
	
	//视频源头的宽度...
	//视频源头的宽度只支持下面4种; 
	//729和1080都支持..
	if(vencWith == 384)			//视频源头的宽度...
	{
		pOsdLogo->nOsdFontW = 32;		//字模本身的宽高..
		pOsdLogo->nOsdFontH = 32;
		if(pOsdLogo->regHdl%2)
			pOsdLogo->nFontInt = -16;	//nFontInt字符间间隔
										//看大的减去小的; 小得减去大的
		else
			pOsdLogo->nFontInt = -16;
	}
	else if(vencWith == 1536)
	{
		pOsdLogo->nOsdFontW = 32;		//字模本身的宽高..
		pOsdLogo->nOsdFontH = 32;		//字模本身的宽高..
		if(pOsdLogo->regHdl%2)
			pOsdLogo->nFontInt = -16;
		else
			pOsdLogo->nFontInt = -16;
	}
	else if(vencWith == 960)
	{
		pOsdLogo->nOsdFontW = 32;
		pOsdLogo->nOsdFontH = 32;
		if(pOsdLogo->regHdl%2)
			pOsdLogo->nFontInt = -8;
		else
			pOsdLogo->nFontInt = -8;
	}
	else if(vencWith == 1920)
	{
		pOsdLogo->nOsdFontW = 32;
		pOsdLogo->nOsdFontH = 32;
		if(pOsdLogo->regHdl%2)
			pOsdLogo->nFontInt = -8;
		else
			pOsdLogo->nFontInt = -8;
	}

	//COMM_SYSLOG(LOG_ERR,"pOsdLogo->nOsdFontW = %d pOsdLogo->nOsdFontH = %d pOsdLogo->nFontInt = %d pOsdLogo->nOsdTitleMaxLen = %d pOsdLogo->nOsdLine = %d \n",pOsdLogo->nOsdFontW ,pOsdLogo->nOsdFontH ,pOsdLogo->nFontInt ,pOsdLogo->nOsdTitleMaxLen,pOsdLogo->nOsdLine);
	//pOsdLogo中的结构体;在真个程序里面做了完整的信息记录和总开关;
	//osd的宽度加上中间的间隔  nFontInt--间隔
	//乘以osd里面做长的那一行;
	pOsdLogo->nWidth = (pOsdLogo->nOsdFontW + pOsdLogo->nFontInt)* pOsdLogo->nOsdTitleMaxLen - pOsdLogo->nFontInt;
	pOsdLogo->nHeight= pOsdLogo->nOsdFontH * pOsdLogo->nOsdLine;
	//为了让他对齐....
	pOsdLogo->nWidth = (pOsdLogo->nWidth+1) & 0xFFE;
	pOsdLogo->nHeight= (pOsdLogo->nHeight+1)& 0xFFE;
	//COMM_SYSLOG(LOG_ERR,"test---------pOsdLogo->nWidth = %d \n",pOsdLogo->nWidth);


	//title???内容的排布!!!
	if(vencWith == 1536)
	{
		//下面是别人的排布; 没必要一定按照别人的排布方式..
		if(pOsdLogo->regHdl%2)			//基数行....
		{
			pOsdLogo->osdPos.nOrgX = 0;
			//pOsdLogo->osdPos.nOrgY = 652;
			pOsdLogo->osdPos.nOrgY = 0;

		}else
		{
			//pOsdLogo->osdPos.nOrgX = 1280 - pOsdLogo->nWidth;
			pOsdLogo->osdPos.nOrgX = vencWith - 400 ;		//偶数在右边..
			pOsdLogo->osdPos.nOrgY = 0;
		}

	}
	else if(vencWith == 384)
	{
		if(pOsdLogo->regHdl%2)
		{

			pOsdLogo->osdPos.nOrgX = 0 ;
			pOsdLogo->osdPos.nOrgY = pOsdLogo->nHeight + 2;
		}else
		{
			//pOsdLogo->osdPos.nOrgX = 640 - pOsdLogo->nWidth;
			pOsdLogo->osdPos.nOrgX = 0;
			pOsdLogo->osdPos.nOrgY = 0;
		}
	}
	else if(vencWith == 960)
	{
		if(pOsdLogo->regHdl%2)
		{
			pOsdLogo->osdPos.nOrgX = 0 ;
			pOsdLogo->osdPos.nOrgY = 540 - pOsdLogo->nHeight;
		}else
		{
			//pOsdLogo->osdPos.nOrgX = 640 - pOsdLogo->nWidth;
			pOsdLogo->osdPos.nOrgX = 0;
			pOsdLogo->osdPos.nOrgY = 0;
		}
	}
	else if(vencWith == 1920)
	{
		if(pOsdLogo->regHdl%2)
		{
			pOsdLogo->osdPos.nOrgX = 0 ;
			//pOsdLogo->osdPos.nOrgY = 1080 - pOsdLogo->nWidth;
			pOsdLogo->osdPos.nOrgY = 0;
		}else
		{
//			pOsdLogo->osdPos.nOrgX = 0;
			pOsdLogo->osdPos.nOrgX = vencWith - 400;
			pOsdLogo->osdPos.nOrgY = 0;
		}
	}
	//在上面pOsdLogo拷贝到了tmpLogo
	//判断osdlogo的内容有没有在上面被修改掉; 
	//如果没有修改; 就会返回0;
	if (memcmp(&tmpLogo, pOsdLogo, sizeof(HH_OSD_LOGO)) == 0)
		return 0;

	return 1;
}

int    HI_OSD_Refresh_BitmapSize(HH_OSD_LOGO *pOsdLogo)
{

	return HI_SUCCESS;
}
// 创建申请了osd的bmp图片内存; 调用海思的region api来初始化overlay_rgn
int    HI_Create_Osd_Reg(HH_OSD_LOGO *pOsdLogo)
{
//#if defined(HI3516_ARCH) || defined(HI3518_ARCH)
	RGN_ATTR_S	  stRgnAttr     = {0};
	HI_U32        s32Ret          =  0 ;
	HI_U32        nBmpSize      =  0 ;
	int	  VencGrp       =  0;
	MPP_CHN_S		stChn;
	RGN_CHN_ATTR_S	stChnAttr = {0};

  //  PRINTF(LOG_OSD,"Enter HI_Create_Osd_Reg.\n");
	if (pOsdLogo == NULL)
	{
		COMM_SYSLOG(LOG_ERR,"HI_Create_Osd_Bitmap Param Err\n");
		return HI_FAILURE;
	}

	if (pOsdLogo->pBmpBuf != NULL)
	{
		free(pOsdLogo->pBmpBuf);
		pOsdLogo->pBmpBuf = NULL;
	}
	COMM_SYSLOG(LOG_ERR,"HI_Create_Osd_Reg width %d height %d\n", pOsdLogo->nWidth, pOsdLogo->nHeight);
	//nWidth--是所有OSD里最长的哪一行
	//nHeight --所有的OSD里卖弄有几行; 
	//*2的原因:填充的是ARGB1555,一个像素占用2个字节; 
	//
	//算出来的是以字节为单位的整个osd区域的占用的
	//bmp图片
	//把所有的OSD信息;不管几行'一行几个字; 弄成一个BMP
	//图片刷; 不是一个一个字的刷
	nBmpSize = pOsdLogo->nWidth * pOsdLogo->nHeight * 2;
	if (pOsdLogo->nWidth <= 0 || pOsdLogo->nHeight <= 0 || nBmpSize == 0 || nBmpSize > 1024*1024)
	{
		COMM_SYSLOG(LOG_ERR,"HI_Create_Osd_Reg width %d height %d Err  [pOsdLogo->nWidth = %d]\n", pOsdLogo->nWidth, pOsdLogo->nHeight,pOsdLogo->nWidth);
		return HI_FAILURE;
	}
	//存储生成的BMP图片; 先给BMP申请内存; 
	pOsdLogo->pBmpBuf = malloc(nBmpSize);
	if (pOsdLogo->pBmpBuf == NULL)
	{
		COMM_SYSLOG(LOG_ERR,"HI_Create_Osd_Bitmap malloc error\n");
		return -1;
	}
	//填充的是0X55--原始的  没有被动过的...; 如果显示; 说明没动过.
	memset(pOsdLogo->pBmpBuf, 0x55, pOsdLogo->nWidth * pOsdLogo->nHeight * 2);
	memset(&stRgnAttr, 0, sizeof(RGN_ATTR_S));
	//之初始化的background 没有初始化frontbackground
	//stRgnAttr.enType								= OVERLAY_RGN;
	stRgnAttr.enType								= OVERLAYEX_RGN;
	stRgnAttr.unAttr.stOverlayEx.enPixelFmt			= PIXEL_FORMAT_ARGB_1555;
	stRgnAttr.unAttr.stOverlayEx.stSize.u32Width  	= pOsdLogo->nWidth;
	stRgnAttr.unAttr.stOverlayEx.stSize.u32Height 	= pOsdLogo->nHeight;
	//stRgnAttr.unAttr.stOverlay.u32BgColor			= pOsdLogo->osdColor.bgColor;
	stRgnAttr.unAttr.stOverlayEx.u32BgColor			= 64 ;			//说明背景色是半透明的....
	stRgnAttr.unAttr.stOverlayEx.u32CanvasNum 		= 2; 			//hsx区域的内存数量
	printf("pOsdLogo->nWidth = %d ,  pOsdLogo->nWidth = %d\n",pOsdLogo->nWidth,pOsdLogo->nHeight);
	COMM_SYSLOG(LOG_DEBUG, "HI_Create_Osd_Reg handle  = %d (%d, %d) with = %d height = %d (group : %d)(unknow : %d)\n",
		pOsdLogo->regHdl,
		pOsdLogo->osdPos.nOrgX, pOsdLogo->osdPos.nOrgY,
		pOsdLogo->nWidth, pOsdLogo->nHeight,
		pOsdLogo->vochn,
		pOsdLogo->regHdl % HH_MAX_OSD_PER_GROUP);					//pOsdLogo->regHdl % HH_MAX_OSD_PER_GROUP == VO通道号 + 区域索引

    //---------->海思的API创建region
	s32Ret = HI_MPI_RGN_Create(pOsdLogo->regHdl, &stRgnAttr);		// 这个地方应该填vo通道号pOsdLogo->regHdl
	if(HI_SUCCESS != s32Ret)
	{
		HI_MPI_RGN_Destroy(pOsdLogo->regHdl);
		printf("failedssss with %#x!\n", s32Ret);
		return HI_FAILURE;
	}
	
    VencGrp = pOsdLogo->vochn;
    stChn.enModId = HI_ID_VO;
    stChn.s32DevId = 0;
    stChn.s32ChnId = VencGrp;

    stChnAttr.bShow		= HI_TRUE;
    stChnAttr.enType	= OVERLAYEX_RGN;
    stChnAttr.unChnAttr.stOverlayExChn.stPoint.s32X = 0;//pOsdLogo->osdPos.nOrgX & (~3);
    stChnAttr.unChnAttr.stOverlayExChn.stPoint.s32Y = 0;//pOsdLogo->osdPos.nOrgY & (~3);
    stChnAttr.unChnAttr.stOverlayExChn.u32BgAlpha   = pOsdLogo->osdColor.bgAlpha;
    stChnAttr.unChnAttr.stOverlayExChn.u32FgAlpha   = pOsdLogo->osdColor.fgAlpha;
	stChnAttr.unChnAttr.stOverlayExChn.u32Layer     = 0;
    //---------->>>将区域叠加到通道上。
	printf("pOsdLogo->regHdl = %d sstChn.enModId=%d stChn.s32DevId=%d stChn.s32ChnId=%d stChnAttr.unChnAttr.stOverlayExChn.stPoint.s32X=%d stChnAttr.unChnAttr.stOverlayExChn.stPoint.s32Y=%d pOsdLogo->vochn=%d\n",pOsdLogo->regHdl,\
			stChn.enModId,stChn.s32DevId,stChn.s32ChnId,stChnAttr.unChnAttr.stOverlayExChn.stPoint.s32X,\
			stChnAttr.unChnAttr.stOverlayExChn.stPoint.s32Y,pOsdLogo->vochn);
    s32Ret = HI_MPI_RGN_AttachToChn(pOsdLogo->regHdl, &stChn, &stChnAttr);
    if(HI_SUCCESS != s32Ret)
    {
        COMM_SYSLOG(LOG_ERR,"%s: HI_MPI_RGN_AttachToChn (%d) failed with %#x!\n", __FUNCTION__, pOsdLogo->regHdl, s32Ret);
        return HI_FAILURE;
    }

	return 0;
}
//根据posdlogo记录的osd要显示的内容来生成bmp图片数据; 
//调用HI_MPI_RGN_SetBitMap 吧BMP显示到OSD; 显示OSD中的一个帧数....

//这个bmp由于是rgb的 没有保存在本地...直接保存在了内存....
int HI_OSD_Create_Bitmap(HH_OSD_LOGO *pOsdLogo)
{
	int             line    		 =  0 ;
	char            szTitle[128]= {0};
	int             begPos     	 =  0 ;
	HI_U32          s32Ret    =  0 ;
	
	BITMAP_S	stBitmap;					//用来创建bmp图片的数据填充的信息结构体

	if (pOsdLogo == NULL || pOsdLogo->pBmpBuf == NULL || pOsdLogo->regHdl  == -1)
	{
		COMM_SYSLOG(LOG_ERR,"HI_OSD_Create_Bitmap Param Err\n");
		return HI_FAILURE;
	}

	memset(pOsdLogo->pBmpBuf, 0x55, pOsdLogo->nWidth * pOsdLogo->nHeight * 2);
	
	for(line = 0; line < pOsdLogo->nOsdLine; line++)
	{
		begPos = pOsdLogo->nOsdTitleFlag[line];		//第一行开始位置的索引
		memset(szTitle, 0, sizeof(szTitle));			//定义了一个局部的变量; 
		//上面清零; 这里开始放东西
		if (pOsdLogo->nOsdTitleLen[line] <= 0 ||
			pOsdLogo->nOsdTitleLen[line] >= sizeof(szTitle))
		{
			continue ;
		}
		strncpy(szTitle, &(pOsdLogo->szOsdTitle[begPos]), pOsdLogo->nOsdTitleLen[line]);
		//创建BMP图片....
		//实际上没创建一个真实的BMP;放在了内存中...
		//没有读/ 写/解析/ 因为BMP是自己生成的...

		//由字符串转换生成bmp()的一下数据; 下面开始填充...
		HI_OSD_CreateBMP(pOsdLogo->nVencFeild, pOsdLogo->nOsdFontW,
			pOsdLogo->nWidth, pOsdLogo->nFontInt,
			pOsdLogo->pBmpBuf + line * pOsdLogo->nWidth * pOsdLogo->nOsdFontH*2,
			szTitle,
			((line == pOsdLogo->nSelLine)?pOsdLogo->osdColor.selColor:pOsdLogo->osdColor.fgColor),
			pOsdLogo->osdColor.bgColor,
			pOsdLogo->osdColor.edgeColor);
	}

	//由于上面获取的bmp的信息 进行bmp数据的填充....
	
	//这个bmp由于是rgb的 没有保存在本地...直接保存在了内存....
	//stBitmap.用来记录上面创建的bmp图像里面的属性; 
	stBitmap.u32Width		= pOsdLogo->nWidth; 		//把HI_OSD_CreateBMP得到的BMP的属性放到 这里
    stBitmap.u32Height		= pOsdLogo->nHeight;	//把HI_OSD_CreateBMP得到的BMP的属性放到 这里
    stBitmap.enPixelFormat	= PIXEL_FORMAT_ARGB_1555;
	stBitmap.pData          = pOsdLogo->pBmpBuf;			//把HI_OSD_CreateBMP得到的BMP的实际内容放到 这里
 
    //真正的显示...设置区域位图，即对区域进行位图填充。
    s32Ret = HI_MPI_RGN_SetBitMap(pOsdLogo->regHdl, &stBitmap);
    if(s32Ret != HI_SUCCESS)
    {
        COMM_SYSLOG(LOG_ERR,"%s: HI_MPI_RGN_SetBitMap failed with %#x(%d)!\n", __FUNCTION__, s32Ret, pOsdLogo->regHdl);
        //return HI_FAILURE;
    }
	return 0;
}
//解析posdlogo解析的内容; 并刷新显示title...
int HI_OSD_Refresh_Title(HH_OSD_LOGO *pOsdLogo)
{
	int ret = 0;

	if (pOsdLogo == NULL)
	{
		COMM_SYSLOG(LOG_ERR,"HI_OSD_Refresh_Title Param Err\n");
		return HI_FAILURE;
	}
	//解析....用逻辑或
	if (HI_OSD_Parse_OsdTitle(pOsdLogo) || HI_OSD_Get_BmpSize(pOsdLogo))
	{
		COMM_SYSLOG(LOG_ERR,"ERR_POINT\n");
		if ((ret = HI_Create_Osd_Reg(pOsdLogo)) != 0)
			return ret;
	}
	//显示....
	return HI_OSD_Create_Bitmap(pOsdLogo);
}
//在显示.........等效于HI_OSD_Create_Bitmap
int HI_OSD_Refresh_Color(HH_OSD_LOGO *pOsdLogo)
{
	unsigned int  				sRet   = 0;

	if (pOsdLogo == NULL || pOsdLogo->regHdl == 0)
	{
		COMM_SYSLOG(LOG_ERR,"HI_OSD_Refresh_Color Param Err\n");
		return HI_FAILURE;
	}

	if ((sRet = HI_OSD_Create_Bitmap(pOsdLogo)) != 0)
	{
		COMM_SYSLOG(LOG_ERR,"HH_ChName_Refresh Create Bitmap Err %x\n", sRet);
		return sRet;
	}

	return HI_SUCCESS;

}
//刷新osd显示的左上角坐标的xy'刷新后信息没有变; 但是坐标变了; 
//调用改xy; 指针传递作为参数; 显示的内容没变;但是坐标变了.
int HI_OSD_Refresh_Org(HH_OSD_LOGO *pOsdLogo)
{
    MPP_CHN_S stChn;
    RGN_CHN_ATTR_S stChnAttr;
    HI_S32 s32Ret;

	if (pOsdLogo == NULL || pOsdLogo->regHdl == -1)
	{
		COMM_SYSLOG(LOG_ERR, "HI_OSD_Refresh_Org Param Err\n");
		return HI_FAILURE;
	}

    stChn.enModId  = HI_ID_VO;

    stChn.s32DevId = pOsdLogo->vochn;
    stChn.s32ChnId = 0;


    s32Ret = HI_MPI_RGN_GetDisplayAttr(pOsdLogo->regHdl, &stChn, &stChnAttr);
    if(HI_SUCCESS != s32Ret)
    {
        COMM_SYSLOG(LOG_ERR,"%s: HI_MPI_RGN_GetDisplayAttr (%d)) failed with %#x!\n",  __FUNCTION__, pOsdLogo->regHdl, s32Ret);
        return HI_FAILURE;
    }

	/* longjian 2012-3-29 修改过 */
	//刷新xy
    stChnAttr.unChnAttr.stOverlayExChn.stPoint.s32X = pOsdLogo->osdPos.nOrgX & 0xffc;
    stChnAttr.unChnAttr.stOverlayExChn.stPoint.s32Y = pOsdLogo->osdPos.nOrgY & 0xffc;

	//COMM_SYSLOG(LOG_ERR,"\nvencGroup= %d,regHdl= %d\n", pOsdLogo->vencGroup, pOsdLogo->regHdl);
	//COMM_SYSLOG(LOG_ERR,"s32X= %d, s32Y= %d\n",  stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X, stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y );

    s32Ret = HI_MPI_RGN_SetDisplayAttr(pOsdLogo->regHdl, &stChn, &stChnAttr);
    if(HI_SUCCESS != s32Ret)
    {
        COMM_SYSLOG(LOG_ERR,"%s: HI_MPI_RGN_SetDisplayAttr (%d)) failed with %#x!\n", __FUNCTION__, pOsdLogo->regHdl, s32Ret);
        return HI_FAILURE;
    }

	return HI_SUCCESS;
}
//检查img的大小是否改变
//检查从ipc_conf中读取的视频流的大小和posdlogo中记录的大小是够一样;
//不一样就讲posdloogo的大小跟新为新的大小; 
//自动码流来调整清晰度..自适应码流....
//网络怒不好继续退; 降低清晰度为大小换区视频的不卡对; 
//下面的目的是让osd显示的大小随着码流的大小输出而改变; 
//让用户看到的osd是米有大小差异的变化的; 不然输出低码流; osd会变大.....

int HI_OSD_Check_ImaSizeChange(HH_OSD_LOGO *pOsdLogo)
{
	int nWidth = 0;
	int nHeight= 0;

	if (pOsdLogo == NULL)
		return 0;
	//原始图像的大小....
	HI_OSD_Get_ImaSize(pOsdLogo->vochn, &nWidth, &nHeight);
	
	if (pOsdLogo->nImageWidth != nWidth || pOsdLogo->nImageHeight != nHeight)
	{
		pOsdLogo->nImageWidth = nWidth;
		pOsdLogo->nImageHeight= nHeight;
		return 1;
	}
	return 0;
}
//刷新被现实的马一行; 
//这里没有实现菜单;
//原始的里是有菜单的; 这里把他去掉了...
//这个函数在这里没有任何作用; 
int HI_OSD_Refresh_SelLine(HH_OSD_LOGO *pOsdLogo, int nLine)
{
	if (pOsdLogo == NULL || pOsdLogo->regHdl == 0)
	{
		COMM_SYSLOG(LOG_ERR,"HI_OSD_Refresh_SelLine Param Err\n");
		return HI_FAILURE;
	}

	pOsdLogo->nSelLine = nLine;
	return nLine;
}
//申请的内存什么的都去掉..
int HI_OSD_Destroy(HH_OSD_LOGO *pOsdLogo)
{
	HI_S32 s32Ret = 0;

    COMM_SYSLOG(LOG_DEBUG,"Enten HI_OSD_Destroy pOsdLogo->regHdl[%d].\n",pOsdLogo->regHdl);
	if (pOsdLogo == NULL)
	{
		COMM_SYSLOG(LOG_ERR,"HI_OSD_Destroy Param Err\n");
		return 0;
	}

	pOsdLogo->nStatus = 0xFF;		//准备马上销毁; 设置为0xFF
	if (pOsdLogo->regHdl != -1)
	{
		//rgn内部模块销毁; 
		if ((s32Ret = HI_MPI_RGN_Destroy(pOsdLogo->regHdl)) != HI_SUCCESS)
		{
		    COMM_SYSLOG(LOG_ERR,"HI_OSD_Destroy failed %x.\n",s32Ret);
		//	HH_SYSLOG(HH_LOG_ERR,"HI_MPI_VPP_DestroyRegion Err %x %d\n", s32Ret, pOsdLogo->regHdl);
		}

		//pOsdLogo->regHdl = -1;
	}
	//防止野指针的方法; 
	if (pOsdLogo->pBmpBuf != NULL)
	{
		free(pOsdLogo->pBmpBuf);
		pOsdLogo->pBmpBuf = NULL;
	}

	return 0;
}

//创建osd结构体/创建位图/直接显示
//这个函数重要; 之前的函数都直接的或者间接的被下面的调用; 
int HI_OSD_Build(HH_OSD_LOGO *pOsdLogo)
{
	//printf("**********************************************************************************************\n");
	HI_S32 ret = 0;

    //对osd的字符进行字符解析,区分中文英文,更新字符相关属性
	//解析OSD title  解析szOsdTitle解析成一段一段
	//放到nOsdTileLen[]/nOsdTileFlag[]
	HI_OSD_Parse_OsdTitle(pOsdLogo);
	   
	//获取bmp文件的大小....
	//获取配置的大小...
	//根据宽高等  设置bmp 字模尺寸;    字库转bmp
	HI_OSD_Get_BmpSize(pOsdLogo);//这里设置pOsdLogo->nOsdFontW 为32(原本是0)

	//创建区域
	// 创建申请了osd的bmp图片内存; 调用海思的region api来初始化overlay_rgn
	if ((ret = HI_Create_Osd_Reg(pOsdLogo)) != 0)
	{
		COMM_SYSLOG(LOG_ERR,"HI_OSD_Build Creat e Reg Err\n");
		return -1;
	}

	//创建bmp  位图
	if ((ret = HI_OSD_Create_Bitmap(pOsdLogo)) != 0)
	{
		COMM_SYSLOG(LOG_ERR,"HI_OSD_Build Create Reg Err\n");
		return -1;
	}

	//显示OSD  显示位图
	if ((ret = HI_OSD_Set_Show(pOsdLogo, pOsdLogo->nStatus)) != 0)
	{
		COMM_SYSLOG(LOG_ERR,"HI_OSD_Build Create Reg Err\n");
		return -1;
	}
	//printf("**********************************************************************************************1\n");

	return 0;
}

/******************************************************************************
* 函数名称:	HH_OSD_GetCharBit(unsigned char *in, short qu, short wei, short fontSize)
* 功能描述: 取得汉字的点阵
* 其他说明:从字库查询获得汉子的内容; 
short qu-------区码
short wei------位码
short fontSize--字体大小
用传入的quhewei来擦讯子墨表; 将查到的自摸输出到in字符串中 
//编译器知道; 编译器里写了汉子; 编译器会自动的查...quwei
*******************************************************************************/
int HI_OSD_GetCharBit(unsigned char *in, short qu, short wei, short fontSize)
{
	int pos,k;
	unsigned char *pData;
	unsigned char *pReal_HZ = g_real_HZ16p;   //字体大小
	
	//COMM_SYSLOG(LOG_INFO,"HH_OSD_GetCharBit qu = %d wei = %d  fontSize= %d \n", qu, wei,fontSize);

	if (fontSize <= 8)
		pReal_HZ = g_real_HZ8p;		//字体大小
	else if (fontSize <= 12 || fontSize == 24)
		pReal_HZ = g_real_HZ12p;		//字体大小

	if (qu > 94 || wei > 94 || qu < 1 || wei < 1)
	{
		COMM_SYSLOG(LOG_ERR,"HH_OSD_GetCharBit qu = %d wei = %d\n", qu, wei);
		return 1;
	}

	pos = (qu - 1) * 94 + wei - 1;			//查询数组的下表; 从0开始

	if (pos > REAL_HZ16_COUNT)
	{
		COMM_SYSLOG(LOG_ERR,"HH_OSD_GetCharBit pos = %d\n", pos);
		return 2;
	}
	if (fontSize <= 8)
	{
 		pData = pReal_HZ + pos * 8 ;//(pos << 3);		//*32
		memcpy(in, pData, 8);						//穿的8
	}
	else if (fontSize <= 12 || fontSize == 24)
	{
 		pData = pReal_HZ + pos * 24 ;//(pos << 3);		//*32
		memcpy(in, pData, 24);
	}
	else if (fontSize <= 16 || fontSize == 32)
	{
		pData = pReal_HZ + (pos << 5);		//*32
		memcpy(in, pData, 32);
		for(k=0;k<32;k++)
		{
			//printf("*pData=%d \r\n,",*(pData+k));
		}
	}


	return 0;
};

/******************************************************************************
* 函数名称:	HH_OSD_DrawBitmapBit(hzPos, bitmap, fColor, bColor, bmpBuffer, fontSize)
* 功能描述: 将汉字的点阵转换为bmp写入缓冲区
* 其他说明:HI_U8* bitmap也有可能是输入
///HI_U8* bitmap是输出
//imagetolcd   吧位图转换成字磨;

//这里是相反的; 吧取来来的模转成一副图片;
*******************************************************************************/
int HI_OSD_DrawBitmapBit(int x_pos, HI_U8* bitmap, HI_U16 fColor,
		HI_U16 bColor, HI_U16 edgeColor, HI_U8* bmpBuffer, HI_U16 bmpWidth,
		HI_U16 fontSize)

{
	int i,  j;
	int w,  h;
	int startTop = 0;
	unsigned char  *pBuf = bmpBuffer;				//只想穿过来同样指向同样的
	unsigned short *pData = NULL;
	unsigned char   index;
	int charwidth = 2;								//argb1555的问题
	int lineWidth = bmpWidth << 1;
	int hzSize    = 16;
	int bZoom     = 0;
	startTop = 0;

	if (fontSize <= 8)			//汉子的小小..
	{
		hzSize = 8;
		charwidth = 1;		//只是设置了汉子size
	}
	else if (fontSize <= 12 || fontSize == 24)
	{
		hzSize = 12;
	}
	if (fontSize == 24 || fontSize == 32)
	{
		bZoom = 1;
	}

	for (i = 0; i < hzSize; i++)
	{
		w = (i + x_pos) << 1;

		h = 0;

		for (j = 0; j < hzSize; j++, startTop += charwidth)
		{
//字磨要先找到
			index = (*(bitmap + startTop + (i>>3))) & (0x80>>(i&0x07)) ? 1: 0;		
			//index 有0 有1
			//printf("\r\n === %d %d ===\r\n", h,w);
//bmp图片放的位置要找到
			pData = (unsigned short *)((unsigned char  *)pBuf + h + w);			
			
			if(index)
			{
				if (bZoom)
				{
					*pData       = fColor;
					*(pData + 1) = fColor;
					*(pData + 2) = edgeColor;
				//	*(pData + 3) = edgeColor;
					*(pData + bmpWidth) = fColor;
					*(pData + bmpWidth + 1) = fColor;
					*(pData + bmpWidth + 2) = edgeColor;
				//	*(pData + bmpWidth + 1) = edgeColor;
				}
				else
				{
					*pData       = fColor;
					*(pData + 1) = edgeColor;

				}
			}
			h += lineWidth * (bZoom + 1);
		}

		startTop  = 0;
	}

	return 0;
}

/******************************************************************************
* 函数名称:	HH_OSD_DrawBitmapBit(hzPos, bitmap, fColor, bColor, bmpBuffer, fontSize)
* 功能描述: 将汉字的点阵转换为bmp写入缓冲区
* 其他说明: 写两行相同数据(D1 两场时使用)
//32填充的更粗..
*******************************************************************************/
int HI_OSD_DrawBitmapBit32(int x_pos, HI_U8 *bitmap, HI_U16 fColor,
				HI_U16 bColor, HI_U16 edgeColor, HI_U8 *bmpBuffer,
				HI_U16 bmpWidth, HI_U16 fontSize)
{
	int i,  j;
	int w,  h;
	int startTop = 0;
	unsigned char  *pBuf = bmpBuffer;
	unsigned short *pData = NULL;
	unsigned char   index;
	int charwidth = 2;
	int lineWidth = bmpWidth << 1;
	int hzSize    = 16;
	int bZoom     = 0;

	startTop = 0;
	edgeColor = 0;

	if (fontSize <= 8)
	{
		hzSize = 8;
		charwidth = 1;
	}
	else if (fontSize <= 12 || fontSize == 24)
		hzSize = 12;
	else if (fontSize <= 16 || fontSize == 32)
		hzSize = 16;
	if (fontSize == 24 || fontSize == 32)
	{
		bZoom = 1;
	}
	for (i = 0; i < hzSize; i++)
	{
		w = (i + x_pos) << 1;

		h = 0;
		for (j = 0; j < hzSize; j++, startTop += charwidth)
		{	
//下面运算除了田字格里面的那个像素点是有汉子轮廓在的像素点位置
			index = (*(bitmap + startTop + (i>>3))) & (0x80>>(i&0x07)) ? 1: 0;
			pData = (unsigned short *)((unsigned char *)pBuf + h + w);
			if(index)
			{
				if (bZoom)
				{									//前两行计算出了...
					*pData       = fColor;				//写字的部分有字的颜色
					//有字的点自由一个像素点; 那么字体会很细....
					//pData /pData + 1都填充让字体变粗; 具体看笔记中额图
					*(pData + 1) = fColor; //edgeColor				//边缘让字体美观; 
					*(pData + 2) = edgeColor;
					*(pData + 3) = edgeColor;
					//图片一行对应的字节的宽度; 
					*(pData + bmpWidth)     = fColor;		//注意：这儿是(unsigned short *)
					*(pData + bmpWidth + 1) = fColor;
					*(pData + bmpWidth + 2) = edgeColor;
					*(pData + bmpWidth + 3) = edgeColor;
					//宽度
					*(pData + 2*bmpWidth)     = fColor;		//注意：这儿是(unsigned short *)
					*(pData + 2*bmpWidth + 1) = fColor;
					*(pData + 2*bmpWidth + 2) = edgeColor;
					*(pData + 2*bmpWidth + 3) = edgeColor;
					*(pData + 3*bmpWidth)     = fColor;		//注意：这儿是(unsigned short *)
					*(pData + 3*bmpWidth + 1) = fColor;
					*(pData + 3*bmpWidth + 2) = edgeColor;
					*(pData + 3*bmpWidth + 3) = edgeColor;
				}
				else
				{
					*pData       = fColor;
					*(pData + 1) = edgeColor;
					*(pData + bmpWidth)     = fColor;		//注意：这儿是(unsigned short *)
					*(pData + bmpWidth + 1) = edgeColor;
				}
			}
			h += (lineWidth << 1) * (bZoom + 1);
		}
		startTop  = 0;
	}
	return 0;
}

/******************************************************************************
* 函数名称:	HH_OSD_CreateBMP()
* 功能描述: 由字符串生成bmp()
* 其他说明:一次性传递进来一个字符串丢进来挨个的查找每一个字符
//里的每一个字符; 吧每一个字符做成BMP;把bmp拼接成一个总的BMP
*******************************************************************************/
void HI_OSD_CreateBMP(int bTwoFeild, int nFontSize, int bmpWidth,
					  int space, char *bmpBuffer, char *string,
					  HI_U16 fColor, HI_U16 bColor, HI_U16 edgeColor)
{
    int qu  = 0;
	int wei = 0;
	int i   = 0;
	int hzCount    = 0;
	int hzPos      = 0;
	int realSizeX  = nFontSize - nFontSize % 2;
	unsigned char  bitmap[128];
	unsigned char *pstr = (unsigned char *)string;

    hzPos = 0;
	i     = 0;
	while (*(pstr+i) != '\0')
	{
		if (*(pstr+i) >= 128)
		{
			qu = *(pstr+i) - 160;			//QU/
			i++;
			if (*(pstr+i) >= 128)
			{
				hzCount++;
				wei = *(pstr+i) - 160;		//WEI
				//传递qu, wei, nFontSize得到 bitmap
				HI_OSD_GetCharBit(bitmap, qu, wei, nFontSize);
						
				if( bTwoFeild )			//分2场
					HI_OSD_DrawBitmapBit32(hzPos, bitmap, fColor, bColor,
								edgeColor, bmpBuffer, bmpWidth, nFontSize);
				else						//分1场
					HI_OSD_DrawBitmapBit(hzPos, bitmap, fColor, bColor,
								edgeColor, bmpBuffer, bmpWidth, nFontSize);
				i++;
				hzPos += (realSizeX + space);
			}
			else if (*(pstr+i) != '\0')	//汉子区位码....比较冷僻...工作不涉及就不重要; 
			{						//结合笔记中的图..
				hzCount++;
				qu  = 3;
				wei = *(pstr+i) - 32;
				if (wei > 0 && wei < 95)
				{
					HI_OSD_GetCharBit(bitmap, qu, wei, nFontSize);
					if( bTwoFeild )
						HI_OSD_DrawBitmapBit32(hzPos, bitmap, fColor,
						 bColor, edgeColor, bmpBuffer, bmpWidth, nFontSize);
					else
						HI_OSD_DrawBitmapBit(hzPos, bitmap, fColor,
						bColor, edgeColor, bmpBuffer, bmpWidth, nFontSize);
				}
				i++;
				hzPos += (realSizeX + space);
			}
			else
			{
				break;
			}
		}
		else
		{
			hzCount++;
			qu  = 3;
			wei = *(pstr + i) - 32;
			//COMM_SYSLOG(LOG_INFO,"test*****************\n");
			if (wei > 0 && wei < 95)
			{
			    //位码
			    //用传入的quhewei来擦讯子墨表; 将查到的自摸输出到in字符串中 
				HI_OSD_GetCharBit(bitmap, qu, wei, nFontSize);
				if( bTwoFeild )
					//将汉字的点阵转换为bmp写入缓冲
					HI_OSD_DrawBitmapBit32(hzPos, bitmap, fColor, bColor,
						edgeColor, bmpBuffer, bmpWidth, nFontSize);
				else
					HI_OSD_DrawBitmapBit(hzPos, bitmap, fColor, bColor,
						edgeColor, bmpBuffer, bmpWidth, nFontSize);
			}
			i++;
			hzPos += (realSizeX + space);
		}
	}
	return ;
}


