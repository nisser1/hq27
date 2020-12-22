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

#include "hh_hz16lib.h"					//�����ֿ�....
//#include "syslog.h"

extern HH_VIDEO_RESOLUTION ipc_conf;	//�������OSD�������ƵԴͷ��; ϣ����ͨ�õ�OSD����
										//��������ͷ��ƵԴ�ֱ��� 
										//��һ�������ƽ̨����ֲ;
										//��osd�����и�����ƵԴͷ�Ĵ�С
										//����OSD���е���....
										//��ƵԴͷ��1090   OSD������16/12
//����OSD
//ʵ����һ��HH_OSD_LOGO�ṹ�����
HH_OSD_LOGO *HI_Create_Osd(int index, HH_OSD_LOGO **ppLog, int vencGroup,
						   HI_OSD_ORG	osdOrg, HI_OSD_COLOR osdColor,
 							char *pSzTitle, HI_S16 nFontInt, HI_U8 nShow)
{	//����osdlogo�Ľṹ�����
	if (NULL == (*ppLog = (HH_OSD_LOGO *)malloc(sizeof(HH_OSD_LOGO))))
	{
		COMM_SYSLOG(LOG_ERR,"HI_Create_Osd_Reg malloc osd error\n");
		return NULL;
	}
	/*����ȫ������osdlogo�Ľṹ����������*/
	//��Ա������ֵ.....
	(*ppLog)->vochn 		= vencGroup;	//osd ������ӵ��ı�����?????  ͨ����
	(*ppLog)->nStatus   	= nShow;		//������ʾ��ֵΪ0
	(*ppLog)->regHdl    	= index;		//��������
	(*ppLog)->pBmpBuf   	= NULL;
	(*ppLog)->nFontInt  	= nFontInt;		//�ַ��������ٸ�����  1
	(*ppLog)->nSelLine  	= -1;
	
	if (pSzTitle != NULL)
		strncpy((*ppLog)->szOsdTitle, pSzTitle, HH_MAX_TITLE_NUM - 1);		//(*ppLog)->szOsdTitle osd ������Ļ��Ϣ256�ֽ�
	
	//�������*ppLog...
	memcpy(&((*ppLog)->osdPos)  , &osdOrg  , sizeof(HI_OSD_ORG));			//copy��ʼ���� 
	memcpy(&((*ppLog)->osdColor), &osdColor, sizeof(HI_OSD_COLOR));			//copy��ɫ
	
	//ʣ���*ppLog���...
	//����osd�ṹ��/����λͼ/ֱ����ʾ
	if (HI_OSD_Build(*ppLog) < 0)			//----------->>>>>
		return NULL;

	return *ppLog;
}


//��ʵ�����ַ��������ĺ���; 
//����osd���ַ�,�����ַ��������
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

	oldLine    = pOsdLogo->nOsdLine;				//��ǰ��������¼������...
	oldMaxLen  = pOsdLogo->nOsdTitleMaxLen;			//���һ�еĳ���....
	pOsdLogo->nOsdTitleFlag[0] = 0;					//��0��Ԫ��ֱ�����Ϊ0
	pOsdLogo->nOsdLine = 1;							//��0�е����ݵĿյ�...
	
	printf("pOsdLogo->nOsdTitleMaxLen = %d\n",pOsdLogo->nOsdTitleMaxLen);
	memset(&pOsdLogo->nOsdTitleLen[0], 0, sizeof(unsigned char)*HH_MAX_TITLE_LINE);//ʵ����Ҫ����֧�ֶ���		/*pOsdLogo->nOsdTitleMaxLen ÿһ�е�osd�ĳ���*/

	//һ��һ���ַ��Ľ���.  ԭʼ��OSD��Ҳ����\0��β��...  ����11�оͲ��ù���; ��������
	//���͵��ַ�����������...
	while (pOsdLogo->szOsdTitle[index] != '\0' && pOsdLogo->nOsdLine <= HH_MAX_TITLE_LINE)//HH_MAX_TITLE_LINE = 11
	{
		if (pOsdLogo->szOsdTitle[index] == '\n')		//ԭʼ��OSD�����л��е�....
		{
			pOsdLogo->nOsdTitleLen[pOsdLogo->nOsdLine-1] = index - pOsdLogo->nOsdTitleFlag[pOsdLogo->nOsdLine-1];	//index��ǰ������ȥ֮ǰ�� �õ���ǰ��OSD���ݳ���
				
			pOsdLogo->nOsdLine    += 1;// '/n' �ַ�

			if (pOsdLogo->nOsdLine > HH_MAX_TITLE_LINE)
				break; 

			if (pOsdLogo->nOsdLine <= HH_MAX_TITLE_LINE)
			{
				//nOsdTitleFlag[]���ֻ��������....
				pOsdLogo->nOsdTitleFlag[pOsdLogo->nOsdLine-1] = index + 1;
			}
			if (len > maxLen)
				maxLen = len;
			len = 0;
		} 
		//ASCII����127�϶�������...
		else if (pOsdLogo->szOsdTitle[index] > 0x7F)			//����ռ������8λ
		{
			if (pOsdLogo->szOsdTitle[index+1] > 0x7F)			//������һ��.....
			{
				len   += 1;
				index += 1;
			}
		}
		else
			len += 1;
		
		++index;
	}

	if (pOsdLogo->nOsdLine <= HH_MAX_TITLE_LINE)		//�Ϸ���������
	{
		pOsdLogo->nOsdTitleLen[pOsdLogo->nOsdLine-1] =
			index - pOsdLogo->nOsdTitleFlag[pOsdLogo->nOsdLine-1];
		if (len > maxLen)
			maxLen = len;
	}

	if (pOsdLogo->nOsdLine > HH_MAX_TITLE_LINE)		//��������...���滹�оͲ�Ҫ..��������...
		pOsdLogo->nOsdLine = HH_MAX_TITLE_LINE;

	pOsdLogo->nOsdTitleMaxLen = maxLen;
	if (oldLine == pOsdLogo->nOsdLine && oldMaxLen == pOsdLogo->nOsdTitleMaxLen)
		return 0;
	return 1;
}

//��ȡͼ��Ĵ�С...
//��ȡ����Ŀ��;
//���ֵ����ipc_configȫ�ֱ��������ó�����
//������osd�Ķ���; ��ipc����ȫ��;
//���԰����OSD�����κε���ƵԴ; 
//���������κε���ƵԴ
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
		    *pWidth = ipc_conf.width;		//��ȡ����ipc_conf��Ŀ�͸�; 
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
//����ʾ�й�..
//���ݴ���showΪtrueΪfalse ���رտ���osd����ʾ;
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
	stChnAttr.bShow = show;		//����region���»��ǲ���ʾ; 
    s32Ret = HI_MPI_RGN_SetDisplayAttr(pOsdLogo->regHdl, &stChn, &stChnAttr);
    if(HI_SUCCESS != s32Ret)
    {
        COMM_SYSLOG(LOG_ERR,"%s: HI_MPI_RGN_SetDisplayAttr (%d)) failed with %#x!\n", __FUNCTION__, pOsdLogo->regHdl, s32Ret);
        return HI_FAILURE;
    }
	return HI_SUCCESS;
}
//��������Դͷ�Ĵ�С������ģ�Ĵ�С; 
//����IPC_CONF���õ���ƵԴͷ�Ĵ�С������pOsdLogo�м�¼��OSD����
//��ģ�Ĵ�С������x y;
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
	
	//��ȡ��ǰ��ƵԴͷ�Ŀ��...
	HI_OSD_Get_ImaSize(pOsdLogo->vochn, &vencWith, &vencHeight);
	printf("current vedio width hegiht %d %d\n",vencWith,vencHeight);
	pOsdLogo->nVencFeild = 0;		
	
	//��ƵԴͷ�Ŀ��...
	//��ƵԴͷ�Ŀ��ֻ֧������4��; 
	//729��1080��֧��..
	if(vencWith == 384)			//��ƵԴͷ�Ŀ��...
	{
		pOsdLogo->nOsdFontW = 32;		//��ģ����Ŀ��..
		pOsdLogo->nOsdFontH = 32;
		if(pOsdLogo->regHdl%2)
			pOsdLogo->nFontInt = -16;	//nFontInt�ַ�����
										//����ļ�ȥС��; С�ü�ȥ���
		else
			pOsdLogo->nFontInt = -16;
	}
	else if(vencWith == 1536)
	{
		pOsdLogo->nOsdFontW = 32;		//��ģ����Ŀ��..
		pOsdLogo->nOsdFontH = 32;		//��ģ����Ŀ��..
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
	//pOsdLogo�еĽṹ��;�������������������������Ϣ��¼���ܿ���;
	//osd�Ŀ�ȼ����м�ļ��  nFontInt--���
	//����osd������������һ��;
	pOsdLogo->nWidth = (pOsdLogo->nOsdFontW + pOsdLogo->nFontInt)* pOsdLogo->nOsdTitleMaxLen - pOsdLogo->nFontInt;
	pOsdLogo->nHeight= pOsdLogo->nOsdFontH * pOsdLogo->nOsdLine;
	//Ϊ����������....
	pOsdLogo->nWidth = (pOsdLogo->nWidth+1) & 0xFFE;
	pOsdLogo->nHeight= (pOsdLogo->nHeight+1)& 0xFFE;
	//COMM_SYSLOG(LOG_ERR,"test---------pOsdLogo->nWidth = %d \n",pOsdLogo->nWidth);


	//title???���ݵ��Ų�!!!
	if(vencWith == 1536)
	{
		//�����Ǳ��˵��Ų�; û��Ҫһ�����ձ��˵��Ų���ʽ..
		if(pOsdLogo->regHdl%2)			//������....
		{
			pOsdLogo->osdPos.nOrgX = 0;
			//pOsdLogo->osdPos.nOrgY = 652;
			pOsdLogo->osdPos.nOrgY = 0;

		}else
		{
			//pOsdLogo->osdPos.nOrgX = 1280 - pOsdLogo->nWidth;
			pOsdLogo->osdPos.nOrgX = vencWith - 400 ;		//ż�����ұ�..
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
	//������pOsdLogo��������tmpLogo
	//�ж�osdlogo��������û�������汻�޸ĵ�; 
	//���û���޸�; �ͻ᷵��0;
	if (memcmp(&tmpLogo, pOsdLogo, sizeof(HH_OSD_LOGO)) == 0)
		return 0;

	return 1;
}

int    HI_OSD_Refresh_BitmapSize(HH_OSD_LOGO *pOsdLogo)
{

	return HI_SUCCESS;
}
// ����������osd��bmpͼƬ�ڴ�; ���ú�˼��region api����ʼ��overlay_rgn
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
	//nWidth--������OSD�������һ��
	//nHeight --���е�OSD����Ū�м���; 
	//*2��ԭ��:������ARGB1555,һ������ռ��2���ֽ�; 
	//
	//������������ֽ�Ϊ��λ������osd�����ռ�õ�
	//bmpͼƬ
	//�����е�OSD��Ϣ;���ܼ���'һ�м�����; Ū��һ��BMP
	//ͼƬˢ; ����һ��һ���ֵ�ˢ
	nBmpSize = pOsdLogo->nWidth * pOsdLogo->nHeight * 2;
	if (pOsdLogo->nWidth <= 0 || pOsdLogo->nHeight <= 0 || nBmpSize == 0 || nBmpSize > 1024*1024)
	{
		COMM_SYSLOG(LOG_ERR,"HI_Create_Osd_Reg width %d height %d Err  [pOsdLogo->nWidth = %d]\n", pOsdLogo->nWidth, pOsdLogo->nHeight,pOsdLogo->nWidth);
		return HI_FAILURE;
	}
	//�洢���ɵ�BMPͼƬ; �ȸ�BMP�����ڴ�; 
	pOsdLogo->pBmpBuf = malloc(nBmpSize);
	if (pOsdLogo->pBmpBuf == NULL)
	{
		COMM_SYSLOG(LOG_ERR,"HI_Create_Osd_Bitmap malloc error\n");
		return -1;
	}
	//������0X55--ԭʼ��  û�б�������...; �����ʾ; ˵��û����.
	memset(pOsdLogo->pBmpBuf, 0x55, pOsdLogo->nWidth * pOsdLogo->nHeight * 2);
	memset(&stRgnAttr, 0, sizeof(RGN_ATTR_S));
	//֮��ʼ����background û�г�ʼ��frontbackground
	//stRgnAttr.enType								= OVERLAY_RGN;
	stRgnAttr.enType								= OVERLAYEX_RGN;
	stRgnAttr.unAttr.stOverlayEx.enPixelFmt			= PIXEL_FORMAT_ARGB_1555;
	stRgnAttr.unAttr.stOverlayEx.stSize.u32Width  	= pOsdLogo->nWidth;
	stRgnAttr.unAttr.stOverlayEx.stSize.u32Height 	= pOsdLogo->nHeight;
	//stRgnAttr.unAttr.stOverlay.u32BgColor			= pOsdLogo->osdColor.bgColor;
	stRgnAttr.unAttr.stOverlayEx.u32BgColor			= 64 ;			//˵������ɫ�ǰ�͸����....
	stRgnAttr.unAttr.stOverlayEx.u32CanvasNum 		= 2; 			//hsx������ڴ�����
	printf("pOsdLogo->nWidth = %d ,  pOsdLogo->nWidth = %d\n",pOsdLogo->nWidth,pOsdLogo->nHeight);
	COMM_SYSLOG(LOG_DEBUG, "HI_Create_Osd_Reg handle  = %d (%d, %d) with = %d height = %d (group : %d)(unknow : %d)\n",
		pOsdLogo->regHdl,
		pOsdLogo->osdPos.nOrgX, pOsdLogo->osdPos.nOrgY,
		pOsdLogo->nWidth, pOsdLogo->nHeight,
		pOsdLogo->vochn,
		pOsdLogo->regHdl % HH_MAX_OSD_PER_GROUP);					//pOsdLogo->regHdl % HH_MAX_OSD_PER_GROUP == VOͨ���� + ��������

    //---------->��˼��API����region
	s32Ret = HI_MPI_RGN_Create(pOsdLogo->regHdl, &stRgnAttr);		// ����ط�Ӧ����voͨ����pOsdLogo->regHdl
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
    //---------->>>��������ӵ�ͨ���ϡ�
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
//����posdlogo��¼��osdҪ��ʾ������������bmpͼƬ����; 
//����HI_MPI_RGN_SetBitMap ��BMP��ʾ��OSD; ��ʾOSD�е�һ��֡��....

//���bmp������rgb�� û�б����ڱ���...ֱ�ӱ��������ڴ�....
int HI_OSD_Create_Bitmap(HH_OSD_LOGO *pOsdLogo)
{
	int             line    		 =  0 ;
	char            szTitle[128]= {0};
	int             begPos     	 =  0 ;
	HI_U32          s32Ret    =  0 ;
	
	BITMAP_S	stBitmap;					//��������bmpͼƬ������������Ϣ�ṹ��

	if (pOsdLogo == NULL || pOsdLogo->pBmpBuf == NULL || pOsdLogo->regHdl  == -1)
	{
		COMM_SYSLOG(LOG_ERR,"HI_OSD_Create_Bitmap Param Err\n");
		return HI_FAILURE;
	}

	memset(pOsdLogo->pBmpBuf, 0x55, pOsdLogo->nWidth * pOsdLogo->nHeight * 2);
	
	for(line = 0; line < pOsdLogo->nOsdLine; line++)
	{
		begPos = pOsdLogo->nOsdTitleFlag[line];		//��һ�п�ʼλ�õ�����
		memset(szTitle, 0, sizeof(szTitle));			//������һ���ֲ��ı���; 
		//��������; ���￪ʼ�Ŷ���
		if (pOsdLogo->nOsdTitleLen[line] <= 0 ||
			pOsdLogo->nOsdTitleLen[line] >= sizeof(szTitle))
		{
			continue ;
		}
		strncpy(szTitle, &(pOsdLogo->szOsdTitle[begPos]), pOsdLogo->nOsdTitleLen[line]);
		//����BMPͼƬ....
		//ʵ����û����һ����ʵ��BMP;�������ڴ���...
		//û�ж�/ д/����/ ��ΪBMP���Լ����ɵ�...

		//���ַ���ת������bmp()��һ������; ���濪ʼ���...
		HI_OSD_CreateBMP(pOsdLogo->nVencFeild, pOsdLogo->nOsdFontW,
			pOsdLogo->nWidth, pOsdLogo->nFontInt,
			pOsdLogo->pBmpBuf + line * pOsdLogo->nWidth * pOsdLogo->nOsdFontH*2,
			szTitle,
			((line == pOsdLogo->nSelLine)?pOsdLogo->osdColor.selColor:pOsdLogo->osdColor.fgColor),
			pOsdLogo->osdColor.bgColor,
			pOsdLogo->osdColor.edgeColor);
	}

	//���������ȡ��bmp����Ϣ ����bmp���ݵ����....
	
	//���bmp������rgb�� û�б����ڱ���...ֱ�ӱ��������ڴ�....
	//stBitmap.������¼���洴����bmpͼ�����������; 
	stBitmap.u32Width		= pOsdLogo->nWidth; 		//��HI_OSD_CreateBMP�õ���BMP�����Էŵ� ����
    stBitmap.u32Height		= pOsdLogo->nHeight;	//��HI_OSD_CreateBMP�õ���BMP�����Էŵ� ����
    stBitmap.enPixelFormat	= PIXEL_FORMAT_ARGB_1555;
	stBitmap.pData          = pOsdLogo->pBmpBuf;			//��HI_OSD_CreateBMP�õ���BMP��ʵ�����ݷŵ� ����
 
    //��������ʾ...��������λͼ�������������λͼ��䡣
    s32Ret = HI_MPI_RGN_SetBitMap(pOsdLogo->regHdl, &stBitmap);
    if(s32Ret != HI_SUCCESS)
    {
        COMM_SYSLOG(LOG_ERR,"%s: HI_MPI_RGN_SetBitMap failed with %#x(%d)!\n", __FUNCTION__, s32Ret, pOsdLogo->regHdl);
        //return HI_FAILURE;
    }
	return 0;
}
//����posdlogo����������; ��ˢ����ʾtitle...
int HI_OSD_Refresh_Title(HH_OSD_LOGO *pOsdLogo)
{
	int ret = 0;

	if (pOsdLogo == NULL)
	{
		COMM_SYSLOG(LOG_ERR,"HI_OSD_Refresh_Title Param Err\n");
		return HI_FAILURE;
	}
	//����....���߼���
	if (HI_OSD_Parse_OsdTitle(pOsdLogo) || HI_OSD_Get_BmpSize(pOsdLogo))
	{
		COMM_SYSLOG(LOG_ERR,"ERR_POINT\n");
		if ((ret = HI_Create_Osd_Reg(pOsdLogo)) != 0)
			return ret;
	}
	//��ʾ....
	return HI_OSD_Create_Bitmap(pOsdLogo);
}
//����ʾ.........��Ч��HI_OSD_Create_Bitmap
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
//ˢ��osd��ʾ�����Ͻ������xy'ˢ�º���Ϣû�б�; �����������; 
//���ø�xy; ָ�봫����Ϊ����; ��ʾ������û��;�����������.
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

	/* longjian 2012-3-29 �޸Ĺ� */
	//ˢ��xy
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
//���img�Ĵ�С�Ƿ�ı�
//����ipc_conf�ж�ȡ����Ƶ���Ĵ�С��posdlogo�м�¼�Ĵ�С�ǹ�һ��;
//��һ���ͽ�posdloogo�Ĵ�С����Ϊ�µĴ�С; 
//�Զ�����������������..����Ӧ����....
//����ŭ���ü�����; ����������Ϊ��С������Ƶ�Ĳ�����; 
//�����Ŀ������osd��ʾ�Ĵ�С���������Ĵ�С������ı�; 
//���û�������osd�����д�С����ı仯��; ��Ȼ���������; osd����.....

int HI_OSD_Check_ImaSizeChange(HH_OSD_LOGO *pOsdLogo)
{
	int nWidth = 0;
	int nHeight= 0;

	if (pOsdLogo == NULL)
		return 0;
	//ԭʼͼ��Ĵ�С....
	HI_OSD_Get_ImaSize(pOsdLogo->vochn, &nWidth, &nHeight);
	
	if (pOsdLogo->nImageWidth != nWidth || pOsdLogo->nImageHeight != nHeight)
	{
		pOsdLogo->nImageWidth = nWidth;
		pOsdLogo->nImageHeight= nHeight;
		return 1;
	}
	return 0;
}
//ˢ�±���ʵ����һ��; 
//����û��ʵ�ֲ˵�;
//ԭʼ�������в˵���; �������ȥ����...
//�������������û���κ�����; 
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
//������ڴ�ʲô�Ķ�ȥ��..
int HI_OSD_Destroy(HH_OSD_LOGO *pOsdLogo)
{
	HI_S32 s32Ret = 0;

    COMM_SYSLOG(LOG_DEBUG,"Enten HI_OSD_Destroy pOsdLogo->regHdl[%d].\n",pOsdLogo->regHdl);
	if (pOsdLogo == NULL)
	{
		COMM_SYSLOG(LOG_ERR,"HI_OSD_Destroy Param Err\n");
		return 0;
	}

	pOsdLogo->nStatus = 0xFF;		//׼����������; ����Ϊ0xFF
	if (pOsdLogo->regHdl != -1)
	{
		//rgn�ڲ�ģ������; 
		if ((s32Ret = HI_MPI_RGN_Destroy(pOsdLogo->regHdl)) != HI_SUCCESS)
		{
		    COMM_SYSLOG(LOG_ERR,"HI_OSD_Destroy failed %x.\n",s32Ret);
		//	HH_SYSLOG(HH_LOG_ERR,"HI_MPI_VPP_DestroyRegion Err %x %d\n", s32Ret, pOsdLogo->regHdl);
		}

		//pOsdLogo->regHdl = -1;
	}
	//��ֹҰָ��ķ���; 
	if (pOsdLogo->pBmpBuf != NULL)
	{
		free(pOsdLogo->pBmpBuf);
		pOsdLogo->pBmpBuf = NULL;
	}

	return 0;
}

//����osd�ṹ��/����λͼ/ֱ����ʾ
//���������Ҫ; ֮ǰ�ĺ�����ֱ�ӵĻ��߼�ӵı�����ĵ���; 
int HI_OSD_Build(HH_OSD_LOGO *pOsdLogo)
{
	//printf("**********************************************************************************************\n");
	HI_S32 ret = 0;

    //��osd���ַ������ַ�����,��������Ӣ��,�����ַ��������
	//����OSD title  ����szOsdTitle������һ��һ��
	//�ŵ�nOsdTileLen[]/nOsdTileFlag[]
	HI_OSD_Parse_OsdTitle(pOsdLogo);
	   
	//��ȡbmp�ļ��Ĵ�С....
	//��ȡ���õĴ�С...
	//���ݿ�ߵ�  ����bmp ��ģ�ߴ�;    �ֿ�תbmp
	HI_OSD_Get_BmpSize(pOsdLogo);//��������pOsdLogo->nOsdFontW Ϊ32(ԭ����0)

	//��������
	// ����������osd��bmpͼƬ�ڴ�; ���ú�˼��region api����ʼ��overlay_rgn
	if ((ret = HI_Create_Osd_Reg(pOsdLogo)) != 0)
	{
		COMM_SYSLOG(LOG_ERR,"HI_OSD_Build Creat e Reg Err\n");
		return -1;
	}

	//����bmp  λͼ
	if ((ret = HI_OSD_Create_Bitmap(pOsdLogo)) != 0)
	{
		COMM_SYSLOG(LOG_ERR,"HI_OSD_Build Create Reg Err\n");
		return -1;
	}

	//��ʾOSD  ��ʾλͼ
	if ((ret = HI_OSD_Set_Show(pOsdLogo, pOsdLogo->nStatus)) != 0)
	{
		COMM_SYSLOG(LOG_ERR,"HI_OSD_Build Create Reg Err\n");
		return -1;
	}
	//printf("**********************************************************************************************1\n");

	return 0;
}

/******************************************************************************
* ��������:	HH_OSD_GetCharBit(unsigned char *in, short qu, short wei, short fontSize)
* ��������: ȡ�ú��ֵĵ���
* ����˵��:���ֿ��ѯ��ú��ӵ�����; 
short qu-------����
short wei------λ��
short fontSize--�����С
�ô����quhewei����Ѷ��ī��; ���鵽�����������in�ַ����� 
//������֪��; ��������д�˺���; ���������Զ��Ĳ�...quwei
*******************************************************************************/
int HI_OSD_GetCharBit(unsigned char *in, short qu, short wei, short fontSize)
{
	int pos,k;
	unsigned char *pData;
	unsigned char *pReal_HZ = g_real_HZ16p;   //�����С
	
	//COMM_SYSLOG(LOG_INFO,"HH_OSD_GetCharBit qu = %d wei = %d  fontSize= %d \n", qu, wei,fontSize);

	if (fontSize <= 8)
		pReal_HZ = g_real_HZ8p;		//�����С
	else if (fontSize <= 12 || fontSize == 24)
		pReal_HZ = g_real_HZ12p;		//�����С

	if (qu > 94 || wei > 94 || qu < 1 || wei < 1)
	{
		COMM_SYSLOG(LOG_ERR,"HH_OSD_GetCharBit qu = %d wei = %d\n", qu, wei);
		return 1;
	}

	pos = (qu - 1) * 94 + wei - 1;			//��ѯ������±�; ��0��ʼ

	if (pos > REAL_HZ16_COUNT)
	{
		COMM_SYSLOG(LOG_ERR,"HH_OSD_GetCharBit pos = %d\n", pos);
		return 2;
	}
	if (fontSize <= 8)
	{
 		pData = pReal_HZ + pos * 8 ;//(pos << 3);		//*32
		memcpy(in, pData, 8);						//����8
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
* ��������:	HH_OSD_DrawBitmapBit(hzPos, bitmap, fColor, bColor, bmpBuffer, fontSize)
* ��������: �����ֵĵ���ת��Ϊbmpд�뻺����
* ����˵��:HI_U8* bitmapҲ�п���������
///HI_U8* bitmap�����
//imagetolcd   ��λͼת������ĥ;

//�������෴��; ��ȡ������ģת��һ��ͼƬ;
*******************************************************************************/
int HI_OSD_DrawBitmapBit(int x_pos, HI_U8* bitmap, HI_U16 fColor,
		HI_U16 bColor, HI_U16 edgeColor, HI_U8* bmpBuffer, HI_U16 bmpWidth,
		HI_U16 fontSize)

{
	int i,  j;
	int w,  h;
	int startTop = 0;
	unsigned char  *pBuf = bmpBuffer;				//ֻ�봩����ͬ��ָ��ͬ����
	unsigned short *pData = NULL;
	unsigned char   index;
	int charwidth = 2;								//argb1555������
	int lineWidth = bmpWidth << 1;
	int hzSize    = 16;
	int bZoom     = 0;
	startTop = 0;

	if (fontSize <= 8)			//���ӵ�СС..
	{
		hzSize = 8;
		charwidth = 1;		//ֻ�������˺���size
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
//��ĥҪ���ҵ�
			index = (*(bitmap + startTop + (i>>3))) & (0x80>>(i&0x07)) ? 1: 0;		
			//index ��0 ��1
			//printf("\r\n === %d %d ===\r\n", h,w);
//bmpͼƬ�ŵ�λ��Ҫ�ҵ�
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
* ��������:	HH_OSD_DrawBitmapBit(hzPos, bitmap, fColor, bColor, bmpBuffer, fontSize)
* ��������: �����ֵĵ���ת��Ϊbmpд�뻺����
* ����˵��: д������ͬ����(D1 ����ʱʹ��)
//32���ĸ���..
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
//��������������ָ�������Ǹ����ص����к��������ڵ����ص�λ��
			index = (*(bitmap + startTop + (i>>3))) & (0x80>>(i&0x07)) ? 1: 0;
			pData = (unsigned short *)((unsigned char *)pBuf + h + w);
			if(index)
			{
				if (bZoom)
				{									//ǰ���м������...
					*pData       = fColor;				//д�ֵĲ������ֵ���ɫ
					//���ֵĵ�����һ�����ص�; ��ô������ϸ....
					//pData /pData + 1�������������; ���忴�ʼ��ж�ͼ
					*(pData + 1) = fColor; //edgeColor				//��Ե����������; 
					*(pData + 2) = edgeColor;
					*(pData + 3) = edgeColor;
					//ͼƬһ�ж�Ӧ���ֽڵĿ��; 
					*(pData + bmpWidth)     = fColor;		//ע�⣺�����(unsigned short *)
					*(pData + bmpWidth + 1) = fColor;
					*(pData + bmpWidth + 2) = edgeColor;
					*(pData + bmpWidth + 3) = edgeColor;
					//���
					*(pData + 2*bmpWidth)     = fColor;		//ע�⣺�����(unsigned short *)
					*(pData + 2*bmpWidth + 1) = fColor;
					*(pData + 2*bmpWidth + 2) = edgeColor;
					*(pData + 2*bmpWidth + 3) = edgeColor;
					*(pData + 3*bmpWidth)     = fColor;		//ע�⣺�����(unsigned short *)
					*(pData + 3*bmpWidth + 1) = fColor;
					*(pData + 3*bmpWidth + 2) = edgeColor;
					*(pData + 3*bmpWidth + 3) = edgeColor;
				}
				else
				{
					*pData       = fColor;
					*(pData + 1) = edgeColor;
					*(pData + bmpWidth)     = fColor;		//ע�⣺�����(unsigned short *)
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
* ��������:	HH_OSD_CreateBMP()
* ��������: ���ַ�������bmp()
* ����˵��:һ���Դ��ݽ���һ���ַ��������������Ĳ���ÿһ���ַ�
//���ÿһ���ַ�; ��ÿһ���ַ�����BMP;��bmpƴ�ӳ�һ���ܵ�BMP
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
				//����qu, wei, nFontSize�õ� bitmap
				HI_OSD_GetCharBit(bitmap, qu, wei, nFontSize);
						
				if( bTwoFeild )			//��2��
					HI_OSD_DrawBitmapBit32(hzPos, bitmap, fColor, bColor,
								edgeColor, bmpBuffer, bmpWidth, nFontSize);
				else						//��1��
					HI_OSD_DrawBitmapBit(hzPos, bitmap, fColor, bColor,
								edgeColor, bmpBuffer, bmpWidth, nFontSize);
				i++;
				hzPos += (realSizeX + space);
			}
			else if (*(pstr+i) != '\0')	//������λ��....�Ƚ���Ƨ...�������漰�Ͳ���Ҫ; 
			{						//��ϱʼ��е�ͼ..
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
			    //λ��
			    //�ô����quhewei����Ѷ��ī��; ���鵽�����������in�ַ����� 
				HI_OSD_GetCharBit(bitmap, qu, wei, nFontSize);
				if( bTwoFeild )
					//�����ֵĵ���ת��Ϊbmpд�뻺��
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


