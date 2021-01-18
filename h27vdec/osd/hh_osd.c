/**************************************************************************
 * 	FileName:		osd.c
 *	Description:	OSD Process(��Ļ����)
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



HH_VIDEO_RESOLUTION ipc_conf;		 //RESOLUTION--�ֱ���??
VIDEO_OSD_STATUS *pvideo_osd_status; //osd״̬��Ϣ; ��������OSD�Ŀ���


int UART_TEMP_FLAG;

//osd��logo
HH_OSD_LOGO		*pGOsdLog[HH_MAX_OSD_NUM];	//={0}//����֧�ֶ���·OSD  ���ǵ���Ŀ����ֻ��2·....    HH_MAX_OSD_NUM=48


//���ַ��������ĺ�Ӣ�ĵ�; ģʽ��Ӣ��; 
#if 0
static char *S_WEEK[7]   = {"������", "����һ", "���ڶ�", "������",
							"������", "������", "������"};
#else
static char *S_WEEK[7]   = {"SUN", "MON", "TUE", "WED",
							"THU", "FRI", "SAT"};
#endif
pthread_mutex_t gOsdMutex   = PTHREAD_MUTEX_INITIALIZER;
unsigned int           nOsdRebuild = 0;

//��ȡ��logo��handle
//�㷨��������кܶ��osd; �Ǹ�osd��Ӧ��hanle�Ƕ���.......
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
	//�㷨��������кܶ��osd; �Ǹ�osd��Ӧ��hanle�Ƕ���; ����osd����chnȥ
	//�����handle; �����ط���������㷨�ó�handle�����ظ�; 
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
	//�㷨��������кܶ��osd; �Ǹ�osd��Ӧ��hanle�Ƕ���; ����osd����chnȥ
	//�����handle; �����ط���������㷨�ó�handle�����ظ�; 
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
//��������Ǵ������ܾ����; ���ܴ��벻ȫ...
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

/*���û��OSD����ԭ��.....
nCh			---ͨ����Ŀ
nEnc		--VOͨ����
osdType		---OSD����ĸ��� ������2������(�����ʱ��  ��������̬�����½Ǻ����½�)  2��osd����  0 1
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
	if(nEnc == 0)							//����venc����ͨ��0��ʱ��ͱ�������
	{
		switch(osdType)
		{
			case 0:			//TIME�����������; 
				pOsdOrg->nOrgX = 0	;
				pOsdOrg->nOrgY = 0 ;
				break;

			case 9:			//���ϽǱ��������
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

	}else if(nEnc == 1)						//����venc����ͨ��1��ʱ��ͱ�������
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

//��ȡ��ɫ
int HH_OSD_GetColor(int nCh, HI_OSD_COLOR *pOsdColor)
{
	if (nCh < 0 || pOsdColor == NULL)
	{
		COMM_SYSLOG(LOG_ERR,"HH_OSD_GetColor Param Err\n");
		return -1;
	}

	pOsdColor->bgAlpha = 0;									//������Ϊ͸��
	pOsdColor->fgAlpha = 0x7F;              				//ǰ����͸��
	//��ɫ
	if (pvideo_osd_status->nOsdColor == 1)
	{
		pOsdColor->fgColor  = HH_OSD_COLOR_BLACK;
		pOsdColor->edgeColor= HH_OSD_COLOR_WHITE;
		pOsdColor->selColor = HH_OSD_COLOR_WHITE;
	}
	//��ɫ
	else if (pvideo_osd_status->nOsdColor == 2)
	{
		pOsdColor->fgColor  = HH_OSD_COLOR_YELLOW;			//�����ǻ�
		pOsdColor->edgeColor= HH_OSD_COLOR_BLACK;			//�����Ǻ�
		pOsdColor->selColor = HH_OSD_COLOR_WHITE;			//ѡ���ǰ�
	}
	//��ɫ
	else if (pvideo_osd_status->nOsdColor == 3)
	{
		pOsdColor->fgColor  = HH_OSD_COLOR_RED;
		pOsdColor->edgeColor= HH_OSD_COLOR_BLACK;
		pOsdColor->selColor = HH_OSD_COLOR_WHITE;
	}
	//��ɫ
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
	pOsdColor->bgColor = pOsdColor->fgColor; //������Ϊ͸�� ��ɫ���д
	return 0;
}
//��ȡ�˵�����ɫ..
int HH_OSD_GetMenuColor(int nCh, HI_OSD_COLOR *pOsdColor)
{
	if (nCh != 0 || pOsdColor == NULL)
	{
		COMM_SYSLOG(LOG_ERR,"HH_OSD_GetMenuColor Param Err\n");
		return -1;
	}

	pOsdColor->bgAlpha = 0x7F;					//������Ϊ͸��
	pOsdColor->fgAlpha = 0x7F;					//ǰ����͸��
	
	pOsdColor->fgColor  = HH_OSD_COLOR_WHITE;
	pOsdColor->edgeColor= HH_OSD_COLOR_BLACK;
	pOsdColor->selColor = HH_OSD_COLOR_YELLOW;

	//pOsdColor->bgColor = pOsdColor->fgColor; 		//������Ϊ͸�� ��ɫ���д
	pOsdColor->bgColor = HH_OSD_COLOR_BLACK;
	return 0;
}

//��ȡʱ����ʾ������..
int HH_OSD_GetTimeTitle(int nCh, int bMin, char *pTitle)
{
	//׼����ʼϵͳ����ʱ�亯���ṹ��
	time_t timep;
	struct tm *pLocalTime;
	//����1970-1-1, 00:00:00��������������?
	time(&timep);
	//��ʱ����ֵ�任�ɱ���ʱ�䣬���ǵ�����ʱ��������ʱ��־;
	pLocalTime = localtime(&timep);					//���׵�ʱ��

////////////////////////////////////////////////////////////////////////////////////////////
	
	//׷ԶԴ��--->>>tstDateOSD.
	if(pvideo_osd_status->stDateOSD.Show ||		//�Լ�֮ǰ���õ����ڿ���
	   pvideo_osd_status->stTimeOSD.Show ||		//�Լ�֮ǰ���õ�ʱ�俪��
	   pvideo_osd_status->stWeekOSD.Show )		//�Լ�֮ǰ���õ����ڿ���
	{
		if (pvideo_osd_status->stDateOSD.Show )	//�����Ǳ���Ҫ��ʾ��...
		{
		      //nOsdPrefer--����ͬ���ҵ�year-month-day ��ʾ��ʽ..
			if (pvideo_osd_status->nOsdPrefer == 0){    //1900 
			//д�� pTitle
                		sprintf(pTitle,"%04d-%02d-%02d",1900+pLocalTime->tm_year,1+pLocalTime->tm_mon, pLocalTime->tm_mday) ;
			}else if (pvideo_osd_status->nOsdPrefer == 1){		//��ͬ�Ĺ���/������ ������
				sprintf(pTitle,"%02d-%02d-%04d",1+pLocalTime->tm_mon,pLocalTime->tm_mday, 1900+pLocalTime->tm_year) ;
			}else{
				sprintf(pTitle,"%02d-%02d-%04d",pLocalTime->tm_mday,1+pLocalTime->tm_mon, 1900+pLocalTime->tm_year) ;
			}
		}

		if(pvideo_osd_status->stTimeOSD.Show )		//��????
		{
			sprintf(pTitle,"%s %02d:%02d:%02d %s",pTitle, pLocalTime->tm_hour, pLocalTime->tm_min, pLocalTime->tm_sec, S_WEEK[pLocalTime->tm_wday % 7] ) ;
		}
		//week�ǲ�������; ��ʾ����û��week��
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
	       //��ȡʱ����ʾ������..
		HH_OSD_GetTimeTitle(nCh, bMin, pTitle);// time
	}
	else if (nOsdType == HH_OSD_TYPE_TITLE)
	{
		//��ȡ������ʾ������..  ֮ǰ�Լ����õ�ZIOTLAB
		HH_OSD_GetChNameTitle(nCh, bMin, pTitle);//�ַ�
	}

	return 0;
}
//����ַ����
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
//showˢ��
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
	//----------��ȡhandle
	if ((pOsdLogo = HH_OSD_GetLogoHandle(nCh, nMinEnc, osdType)) == NULL)
	{
		COMM_SYSLOG(LOG_ERR,"HH_Check_Show_Refresh Get Handle Err\n");
		return -2;
	}
	
	showsStatus = HH_OSD_GetShow(nCh, nMinEnc, osdType);
	if (showsStatus != pOsdLogo->nStatus)
	{
		//���ݴ���showΪtrue����fllse���������߹ر�osd��ʾ; 
		HI_OSD_Set_Show(pOsdLogo, showsStatus);
		pOsdLogo->nStatus = showsStatus; //2010-07-12 �޸Ĳ������ص�����
	}
	return 0;
}
 
//osdType = HH_OSD_TYPE_TIME   ʱ��ˢ��....
//osdType = HH_OSD_TYPE_TITLE  ����ˢ��....
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
      //����ȡҪ������vergion��handle....
      //�㷨��������кܶ��osd; �Ǹ�osd��Ӧ��hanle�Ƕ���
	if ((pOsdLogo = HH_OSD_GetLogoHandle(nCh, nMinEnc, osdType)) == NULL)
	{
		//COMM_SYSLOG(HH_LOG_ERR,"HH_OSD_ChName_Refresh Get Handle Err nCh=%d,osdType=%d\n", nCh, osdType);
		return -2;
	}

	memset(szTitle, 0, sizeof(szTitle));

	//��ȡҪ��ʾ��ʱ��ʱ�����ݻ��߱�������
	//osdType = HH_OSD_TYPE_TIME   ʱ��ˢ��....
	//osdType = HH_OSD_TYPE_TITLE  ����ˢ��....
	HH_OSD_GetTitle(nCh, nMinEnc, osdType, szTitle);
	//�Ա�������ʾ�ĺ͸ղ���ʾ���Ƿ�һ��..

	if (strncmp(szTitle, pOsdLogo->szOsdTitle, HH_MAX_TITLE_NUM) != 0 ||
		//ipconfig����Ū�Ĵ�С��posdlogo�����¼�Ĵ�С�ǲ���һ����
		HI_OSD_Check_ImaSizeChange(pOsdLogo))
	{
		memcpy(pOsdLogo->szOsdTitle, szTitle, HH_MAX_TITLE_NUM - 1);
		nNeedReCreate |= HI_OSD_Parse_OsdTitle(pOsdLogo);
		nNeedReCreate |= HI_OSD_Get_BmpSize(pOsdLogo);//�Ͻڿε��ù�..

		if (nNeedReCreate)		//ͼƬ�����˸���..
		{
			HI_OSD_Destroy(pOsdLogo);
			//COMM_SYSLOG(LOG_ERR,"111111111111111111111111111111111111111111111111\n");
			if ((ret = HI_Create_Osd_Reg(pOsdLogo)) != 0)
			{
				COMM_SYSLOG(LOG_ERR,"HH_ChName_Refresh Create Reg Err %x\n", ret);
				return ret;
			}
			//ˢ��osd��ʾ�����Ͻ������xyˢ�º���Ϣû�б�; �����������; 
			HI_OSD_Refresh_Org(pOsdLogo);
			//HI_OSD_Refresh_Color(pOsdLogo);
		}
		// 1--����posdlogo�е�osd������,���ڴ�������bmpͼƬ����(����������BMPͼƬ); 
		// 2--���ڴ������ɵ�bmpλͼ�������ڲ�������ʾ
		if ((ret = HI_OSD_Create_Bitmap(pOsdLogo)) != 0)
		{
			COMM_SYSLOG(LOG_ERR,"HH_ChName_Refresh Create Bitmap Err %x\n", ret);
			return ret;
		}
		//�����Ŀ�ʼ��ʾ��.....
		HH_OSD_Show_Refresh(nCh, nMinEnc, osdType);
	}


	return 0;
}
//������ʾ�������Ͻ�
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
	      //����ͨ��HH_OSD_TYPE_TITLE--��ʾʱ��-------������title
	      //����ͨ��HH_OSD_TYPE_TIME---��ʾ����
		HH_OSD_ChName_Refresh(nCh, nEncType, HH_OSD_TYPE_TITLE);
	}
	return 0;
}
//������ʾ����
int HH_OSD_AllName_Refresh()
{
	int nCh      = 0;

	for (nCh = 0; nCh < VIDEO_NUM; ++nCh)
	{
		//������ʾ����
		HH_OSD_ChnAllName_Refresh(nCh);
		//COMM_SYSLOG(LOG_ERR,"111111\n");
	}
	return 0;
}
//��ʾ����ʱ��...
int HH_OSD_ChnTime_Refresh(int nCh)
{
	int nEncType = 0;

	for (nEncType = 0; nEncType < VIDEO_NUM; ++nEncType)
	{
		//HH_OSD_TYPE_TIME   ʱ��ˢ��....------������time
		//HH_OSD_TYPE_TITLE  ����ˢ��....
		HH_OSD_ChName_Refresh(nCh, nEncType, HH_OSD_TYPE_TIME);
	}
	return 0;
}

//������ʾʱ��
int HH_OSD_AllTime_Refresh()
{
	int nCh      = 0;

	for (nCh = 0; nCh < VIDEO_NUM; ++nCh)
	{
		//������ʾʱ��
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
		//��ʾʱ��...
		HH_OSD_AllTime_Refresh();			//���Ͻ�....
		
		//��ʾ����...
		HH_OSD_AllName_Refresh();			//���Ͻ�
	}
	return 0;
}


/**************************************************************
* ��������	 HH_OSD_SetOsdPosDefault
* ��������:   ����OSDĬ�����꣬��OSD�ص�ʱ����
* �������:
* �������:
* ����ֵ:
* ����:	 by yj 2012-09-17
**************************************************************/
//osd״̬��Ϣ�����������;
int HH_OSD_SetOsdPosDefault(int chn,int mode,int bigshow)//chn��cmd��Ӧ
{
#if 1
	int nCh      = 0;
	pvideo_osd_status->stDateOSD.X		= 0;    
	pvideo_osd_status->stDateOSD.Y		= 32;
	pvideo_osd_status->stDateOSD.Show   = 1;	 		// 0:����������,   1:��������
	pvideo_osd_status->stTitleOSD.X		= 0;
	pvideo_osd_status->stTitleOSD.Y		= 32;
	pvideo_osd_status->stTitleOSD.Show  = 1 ;	 		// 0:�����ӱ���,   1:���ӱ���
	pvideo_osd_status->nOsdColor        = 5;			//osd ������ɫ
	if(bigshow == 1)
		pvideo_osd_status->nOsdColor        = 3;			//osd ������ɫ
	
	pvideo_osd_status->stTimeOSD.Show   = 1;	 		//0:����������,   1:��������
	
	pvideo_osd_status->stWeekOSD.Show   = 1 ;	 		// 0:����������,   1:��������
	pvideo_osd_status->nOsdPrefer       = 0 ;		 	//OSDʱ����ʾ 0 Ϊ year-month-day
										 			    //            1 Ϊ   month-day-year
										 			    //            1 Ϊ   month-day-year
	pvideo_osd_status->stBitrateOSD.Show = 0;   		// 0:����������,   1:��������
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
			sprintf(pvideo_osd_status->Title,"%s", "HDMI");	//ZIOTLAB����ҪҪ��ʾ������
		}
		else if(chn == 0)
		{
			sprintf(pvideo_osd_status->Title,"%s", "ZHC-1-1");	//ZIOTLAB����ҪҪ��ʾ������
		}
		else if(chn == 1)
		{
			sprintf(pvideo_osd_status->Title,"%s", "ZHC-1-2");	//ZIOTLAB����ҪҪ��ʾ������
		}
		else if(chn == 2)
		{
			sprintf(pvideo_osd_status->Title,"%s", "ZHC-2-1");	//ZIOTLAB����ҪҪ��ʾ������
		}
		else if(chn == 3)
		{
			sprintf(pvideo_osd_status->Title,"%s", "ZHC-2-2");	//ZIOTLAB����ҪҪ��ʾ������
		}
		else if(chn == 4)
		{
			sprintf(pvideo_osd_status->Title,"%s", "ZC-1-1");	//ZIOTLAB����ҪҪ��ʾ������
		}
		else if(chn == 5)
		{
			sprintf(pvideo_osd_status->Title,"%s", "ZC-1-2");	//ZIOTLAB����ҪҪ��ʾ������
		}
		else if(chn == 6)
		{
			sprintf(pvideo_osd_status->Title,"%s", "ZC-2-1");	//ZIOTLAB����ҪҪ��ʾ������
		}
		else if(chn == 7)
		{
			sprintf(pvideo_osd_status->Title,"%s", "ZC-2-2");	//ZIOTLAB����ҪҪ��ʾ������
		}
		else if(chn == 8)
		{
			sprintf(pvideo_osd_status->Title,"%s", "CAMER");	//ZIOTLAB����ҪҪ��ʾ������
		}
		else
		{
			printf("title error\n");
		}
	}
	if(mode == 9)
	{
		ipc_conf.width = 384;								//�ֱ���
		ipc_conf.height = 216;
	}
	else if(mode == 4)
	{
		ipc_conf.width = 960;								//�ֱ���
		ipc_conf.height = 540;		
	}
	else if(mode == 10)
	{
		ipc_conf.width = 1536;								//�ֱ���
		ipc_conf.height = 864;		
	}
	else if(mode == 1)
	{
		ipc_conf.width = 1920;								//�ֱ���
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
	int 		 nCh      = 0;			//ͨ��0
	int 		 nEncType = 0;
	int 		 osdType  = 0;
	int 		 index    = 0;
	HI_OSD_ORG	 osdOrg;
	HI_OSD_COLOR osdColor;
	char         szTitle[HH_MAX_TITLE_NUM] = {0};
	COMM_SYSLOG(LOG_DEBUG,"HH_OSD_Init Start... %d\n",VIDEO_NUM);
	//VIDEO_NUM---venc��chn��Ŀǰֻ��1·��Ƶ����ֱ��д1. ������chanel 0			//��Ƶ����
	nEncType = chn;
	osdType = 0;
	//HH_MAX_OSD_PER_GROUP---OSD �������,������2 ������

	memset(&osdOrg,   0, sizeof(HI_OSD_ORG));
	memset(&osdColor, 0, sizeof(HI_OSD_COLOR));
	memset(szTitle,   0, sizeof(szTitle));
	
	/*
	osd״̬��ʾ����Ϣ(�������� �ߴ�,������ɫ��ʱ���ʽ)�������������:
		����OSD �����Ͻ���ʾ�ı�������"ZIOTLAB"  "ht706" 
		����OSD �п�߳ߴ���ɫ
	ʵ����Ŀ����������OSD�Ŀ��صĽṹ����	pvideo_osd_status = (VIDEO_OSD_STATUS *)malloc(sizeof(VIDEO_OSD_STATUS));
	*/
	pvideo_osd_status = (VIDEO_OSD_STATUS *)malloc(sizeof(VIDEO_OSD_STATUS));
	if(ischn9 == 10)
	{
		osdType = 10;
		HH_OSD_SetOsdPosDefault(cmd,10,bigshow);  //10����1536
	}
	else if(ischn9 == 9) //9���������С��Ļ
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
	//�������ԭ��osdOrg--Ӧ���ǻ�ò�ʹ֮OSD��<���ϽǱ���>��<���Ͻ�ʱ��>������ԭ��..
	//nEncType-����ͨ��������;osdType-OSD_REGION������
	
	if(bigshow == 1)
	{
		HH_OSD_GetOrg(nCh, 0, osdType, &osdOrg);					//��ȡ��ʾ������
	}
	else
	{
		HH_OSD_GetOrg(nCh, 0, 0, &osdOrg);					//��ȡ��ʾ������
	}
		

	//���ñ���<>ǰ��<>���ص���ɫ,��ȡ��ɫ--osdColor
	//ͨ��HH_OSD_SetOsdPosDefault���õ�ȫ�ֱ���pvideo_osd_status���û�ȡ��ɫ--osdColor---�������ص���ɫ>
	HH_OSD_GetColor(nCh, &osdColor);
	//��ȡ��ʾ��ˮӡ��Ϣ������
	//�������HH_OSD_SetOsdPosDefault()��ȫ�ֱ����е�ˮӡ��Ϣ..
	osdType = 0;
	HH_OSD_GetTitle(nCh, nEncType, osdType, szTitle);		
	//���OSDת���index  ��תΪΪ 0 �� 1��ʵ��ʹ��ֱ�� ���� osdType����
	//�ó�һ��index���,����û��ʲô����, ֻҪʹ��ÿ��nEncType osdType �����õ�index�������ͬ����; 
	index = nEncType;//nEncType * HH_MAX_OSD_PER_GROUP + osdType;				//osdmenu			HH_MAX_OSD_PER_GROUP 2
	//printf("index=========%d",index);
	//printf("szTitle=========%s",szTitle);
	
	//ͨ������Ļ�ȡ����Ϣ:����,��ɫ,ˮӡ��Ϣ,���,��,����osd�Ĵ�����ʵ����
	//pGOsdLog[index]--����������·��osd
	//���HH_OSD_LOGO
	//index������ʶ
	if (NULL == HI_Create_Osd(index,&pGOsdLog[index], (int)nEncType, osdOrg, osdColor, szTitle, HH_OSD_GetFontInt(nCh, nEncType, osdType), HH_OSD_GetShow(nCh, nEncType, osdType)))
		COMM_SYSLOG(LOG_ERR,"HI_Create_Osd %d %d %d failed !\n", nCh,nEncType, osdType);
	
	COMM_SYSLOG(LOG_DEBUG,"HH_OSD_Init End...\n");
	printf("**********************************************************************************************\n");
	return 0;
}


