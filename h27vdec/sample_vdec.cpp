
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <math.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>


#include "openWrapper.h"

extern "C"{
#include "sample_comm.h"
#include "fifo_buffer.h"
#include "hh_osd.h"
#include "do_switch.h"
}
VO_INTF_SYNC_E g_enIntfSync = VO_OUTPUT_1080P30;
VO_INTF_TYPE_E enVoIntfTypebt1120 = VO_INTF_HDMI;


#define SAMPLE_STREAM_PATH "./source_file"

#ifndef __HuaweiLite__
HI_VOID SAMPLE_VDEC_HandleSig(HI_S32 signo)
{
    if (SIGINT == signo || SIGTSTP == signo || SIGTERM == signo)
    {
        SAMPLE_COMM_SYS_Exit();
        printf("\033[0;31mprogram exit abnormally!\033[0;39m\n");
    }

    exit(0);
}
#endif

HI_VOID SAMPLE_VDEC_Usage(char *sPrgNm)
{
    printf("\n/************************************/\n");
    printf("Usage : %s <index> <IntfSync >\n", sPrgNm);
    printf("index:\n");
    printf("\t0:  VDEC(H265)-VPSS-VO\n");
    printf("\t1:  VDEC(H264)-VPSS-VO\n");
    printf("\t2:  VDEC(JPEG->YUV)-VPSS-VO\n");
    printf("\t3:  VDEC(JPEG->RGB)\n");
    printf("\t4:  VDEC(H264)->VPSS-VO(DHD0-hdmi,DHD1-mipi_tx)\n");

    printf("\nIntfSync :\n");
    printf("\t0: VO HDMI 4K@30fps.\n");
    printf("\t1: VO HDMI 1080P@30fps.\n");
    printf("/************************************/\n\n");
}





HI_S32 SAMPLE_VIO_8K30_PARALLEL(VO_INTF_TYPE_E enVoIntfType)
{
    HI_S32                  s32Ret              = HI_SUCCESS;
    VI_DEV                  ViDev0              = 3;
    VI_PIPE                 ViPipe0             = 0;
    VI_CHN                  ViChn               = 0;
    HI_S32                  s32ViCnt            = 1;
    VPSS_GRP                VpssGrp0            = 0;
    VPSS_CHN                VpssChn	            = 9;
    VPSS_GRP_ATTR_S         stVpssGrpAttr       = {0};
    VPSS_CHN_ATTR_S         stVpssChnAttr[VPSS_MAX_PHY_CHN_NUM];
    HI_BOOL                 abChnEnable[VPSS_MAX_PHY_CHN_NUM] = {0};
    VO_DEV                  VoDev               = SAMPLE_VO_DEV_DHD0;
    VO_CHN                  VoChn               = 9;
    VO_INTF_SYNC_E          g_enIntfSync        = VO_OUTPUT_1080P30;					//change
    HI_U32                  g_u32DisBufLen      = 3;
    PIC_SIZE_E              enPicSize           = PIC_1080P;							//change
    WDR_MODE_E              enWDRMode           = WDR_MODE_NONE;
    DYNAMIC_RANGE_E         enDynamicRange      = DYNAMIC_RANGE_SDR8;
    PIXEL_FORMAT_E          enPixFormat         = PIXEL_FORMAT_YVU_SEMIPLANAR_420;
    VIDEO_FORMAT_E          enVideoFormat       = VIDEO_FORMAT_LINEAR;
    COMPRESS_MODE_E         enCompressMode      = COMPRESS_MODE_NONE;
    VI_VPSS_MODE_E          enMastPipeMode      = VI_OFFLINE_VPSS_OFFLINE;
    SIZE_S                  stSize;
    HI_U32                  u32BlkSize;
    VB_CONFIG_S             stVbConf;
    SAMPLE_VI_CONFIG_S      stViConfig;
    SAMPLE_VO_CONFIG_S      stVoConfig;


    /************************************************
    step 1:  Get all sensors information, need one vi
        ,and need one mipi --
    *************************************************/
    SAMPLE_COMM_VI_GetSensorInfo(&stViConfig);
	printf("stViConfig.astViInfo[0].stSnsInfo.enSnsType=%d\n",stViConfig.astViInfo[0].stSnsInfo.enSnsType);
    stViConfig.s32WorkingViNum                           = s32ViCnt;

    stViConfig.as32WorkingViId[0]                        = 0;
    stViConfig.astViInfo[0].stSnsInfo.MipiDev            = SAMPLE_COMM_VI_GetComboDevBySensor(stViConfig.astViInfo[0].stSnsInfo.enSnsType, 0);
    stViConfig.astViInfo[0].stSnsInfo.s32BusId           = 0;

    stViConfig.astViInfo[0].stDevInfo.ViDev              = ViDev0;
    stViConfig.astViInfo[0].stDevInfo.enWDRMode          = enWDRMode;

    stViConfig.astViInfo[0].stPipeInfo.enMastPipeMode    = enMastPipeMode;
    stViConfig.astViInfo[0].stPipeInfo.aPipe[0]          = ViPipe0;
    stViConfig.astViInfo[0].stPipeInfo.aPipe[1]          = -1;
    stViConfig.astViInfo[0].stPipeInfo.aPipe[2]          = -1;
    stViConfig.astViInfo[0].stPipeInfo.aPipe[3]          = -1;

    stViConfig.astViInfo[0].stChnInfo.ViChn              = ViChn;
    stViConfig.astViInfo[0].stChnInfo.enPixFormat        = enPixFormat;
    stViConfig.astViInfo[0].stChnInfo.enDynamicRange     = enDynamicRange;
    stViConfig.astViInfo[0].stChnInfo.enVideoFormat      = enVideoFormat;
    stViConfig.astViInfo[0].stChnInfo.enCompressMode     = enCompressMode;

    /************************************************
    step 2:  Get  input size
    *************************************************/
    s32Ret = SAMPLE_COMM_VI_GetSizeBySensor(stViConfig.astViInfo[0].stSnsInfo.enSnsType, &enPicSize);

    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VI_GetSizeBySensor failed with %d!\n", s32Ret);
        return s32Ret;
    }

    s32Ret = SAMPLE_COMM_SYS_GetPicSize(enPicSize, &stSize);

    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_SYS_GetPicSize failed with %d!\n", s32Ret);
        return s32Ret;
    }

    /************************************************
    step3:  Init SYS and common VB
    *************************************************/
	/*hi_memset(&stVbConf, sizeof(VB_CONFIG_S), 0, sizeof(VB_CONFIG_S));
    stVbConf.u32MaxPoolCnt              = 2;

    u32BlkSize = COMMON_GetPicBufferSize(stSize.u32Width, stSize.u32Height, SAMPLE_PIXEL_FORMAT, DATA_BITWIDTH_10, COMPRESS_MODE_SEG, DEFAULT_ALIGN);
    stVbConf.astCommPool[0].u64BlkSize  = u32BlkSize;
    stVbConf.astCommPool[0].u32BlkCnt   = 10;

    u32BlkSize = VI_GetRawBufferSize(stSize.u32Width, stSize.u32Height, PIXEL_FORMAT_RGB_BAYER_16BPP, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
    stVbConf.astCommPool[1].u64BlkSize  = u32BlkSize;
    stVbConf.astCommPool[1].u32BlkCnt   = 4;

    s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);

    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("system init failed with %d!\n", s32Ret);
        goto EXIT;
    }*/

    s32Ret = SAMPLE_COMM_VI_SetParam(&stViConfig);

    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VI_SetParam failed with %d!\n", s32Ret);
        //goto EXIT;
    }


    /************************************************
    step 4: start VI
    *************************************************/
    s32Ret = SAMPLE_COMM_VI_StartVi(&stViConfig);

    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VI_StartVi failed with %d!\n", s32Ret);
        //goto EXIT3;
    }

    /************************************************
    step 5: start VPSS, need one grp
    *************************************************/
    /*stVpssGrpAttr.u32MaxW                        = stSize.u32Width;
    stVpssGrpAttr.u32MaxH                        = stSize.u32Height;
    stVpssGrpAttr.enPixelFormat                  = enPixFormat;
    stVpssGrpAttr.enDynamicRange                 = enDynamicRange;
    stVpssGrpAttr.stFrameRate.s32SrcFrameRate    = -1;
    stVpssGrpAttr.stFrameRate.s32DstFrameRate    = -1;

    abChnEnable[0]                               = HI_TRUE;
    stVpssChnAttr[0].u32Width                    = stSize.u32Width;
    stVpssChnAttr[0].u32Height                   = stSize.u32Height;
    stVpssChnAttr[0].enChnMode                   = VPSS_CHN_MODE_USER;
    stVpssChnAttr[0].enCompressMode              = enCompressMode;
    stVpssChnAttr[0].enDynamicRange              = enDynamicRange;
    stVpssChnAttr[0].enPixelFormat               = enPixFormat;
    stVpssChnAttr[0].enVideoFormat               = enVideoFormat;
    stVpssChnAttr[0].stFrameRate.s32SrcFrameRate = -1;
    stVpssChnAttr[0].stFrameRate.s32DstFrameRate = -1;
    stVpssChnAttr[0].u32Depth                    = 1;
    stVpssChnAttr[0].bMirror                     = HI_FALSE;
    stVpssChnAttr[0].bFlip                       = HI_FALSE;
    stVpssChnAttr[0].stAspectRatio.enMode        = ASPECT_RATIO_NONE;*/

	//add
    /*s32Ret = SAMPLE_COMM_VPSS_Start(VpssGrp0, abChnEnable, &stVpssGrpAttr, stVpssChnAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VPSS_Start Grp0 failed with %d!\n", s32Ret);
        goto EXIT2;
    }*/

    /************************************************
    step 6:  VI bind VPSS, for total parallel, no need bind
    *************************************************/

    /*s32Ret = SAMPLE_COMM_VI_Bind_VPSS(ViPipe0, ViChn, VpssGrp0);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VI_Bind_VPSS failed with %d!\n", s32Ret);
        goto EXIT1;
    }*/


    /************************************************
    step 7:  start V0
    *************************************************/
    /*SAMPLE_COMM_VO_GetDefConfig(&stVoConfig);
    stVoConfig.VoDev                                    = VoDev;
    stVoConfig.enVoIntfType                             = enVoIntfType;
    stVoConfig.enIntfSync                               = g_enIntfSync;
    stVoConfig.enPicSize                                = enPicSize;
    stVoConfig.u32DisBufLen                             = g_u32DisBufLen;
    stVoConfig.enDstDynamicRange                        = enDynamicRange;
    stVoConfig.enVoMode                                 = VO_MODE_1MUX;*/

    //s32Ret = SAMPLE_COMM_VO_StartVO(&stVoConfig);

/*if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VO_StartVO failed with %d!\n", s32Ret);
        //goto EXIT1;
    }*/
    /************************************************
    step 8:  VI bind VPSS bind VO
    *************************************************/

    //PAUSE();
    //SAMPLE_COMM_VPSS_UnBind_VO(VpssGrp0, VpssChn, stVoConfig.VoDev, VoChn);
//EXIT1:
 //   SAMPLE_COMM_VO_StopVO(&stVoConfig);
//EXIT2:
//    SAMPLE_COMM_VPSS_Stop(VpssGrp0, abChnEnable);
//EXIT3:
//    SAMPLE_COMM_VI_StopVi(&stViConfig);
//EXIT:
    //SAMPLE_COMM_SYS_Exit();
	printf("end of SAMPLE_VIO_8K30_PARALLEL");
    return s32Ret;
}





HI_S32 SAMPLE_H264_VDEC_VPSS_VO(MESSAGE *msg)
{
    VB_CONFIG_S stVbConfig;
    HI_S32 i, s32Ret = HI_SUCCESS;
    VDEC_THREAD_PARAM_S stVdecSend[VDEC_MAX_CHN_NUM];
    SIZE_S stDispSize;
    VO_LAYER VoLayer;
    HI_U32 u32VdecChnNum, VpssGrpNum;
    VPSS_GRP VpssGrp;
    pthread_t   VdecThread[2*VDEC_MAX_CHN_NUM];
    PIC_SIZE_E enDispPicSize;
    SAMPLE_VDEC_ATTR astSampleVdec[VDEC_MAX_CHN_NUM];
    VPSS_CHN_ATTR_S astVpssChnAttr[VPSS_MAX_CHN_NUM];
    SAMPLE_VO_CONFIG_S stVoConfig;
    VPSS_GRP_ATTR_S stVpssGrpAttr;
    HI_BOOL abChnEnable[VPSS_MAX_CHN_NUM];
    VO_INTF_SYNC_E enIntfSync;
	HI_U32 u32BlkSize;
    u32VdecChnNum = 10;
    VpssGrpNum    = u32VdecChnNum;




    /************************************************
    step1:  init SYS, init common VB(for VPSS and VO)
    *************************************************/
    if(VO_OUTPUT_3840x2160_30 == g_enIntfSync)
    {
        enDispPicSize = PIC_3840x2160;
        enIntfSync    = VO_OUTPUT_3840x2160_30;
    }
    else
    {
        enDispPicSize = PIC_1080P;
        enIntfSync    = VO_OUTPUT_1080P30;
    }

    s32Ret =  SAMPLE_COMM_SYS_GetPicSize(enDispPicSize, &stDispSize);
    if(s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("sys get pic size fail for %#x!\n", s32Ret);
        goto END1;
    }

    memset(&stVbConfig, 0, sizeof(VB_CONFIG_S));
    stVbConfig.u32MaxPoolCnt             = 2;				//缓存块个数
    stVbConfig.astCommPool[0].u32BlkCnt  = 10*u32VdecChnNum;//每个缓存吃缓存块的个数 取值范围(0,10240]
    stVbConfig.astCommPool[0].u64BlkSize = COMMON_GetPicBufferSize(stDispSize.u32Width, stDispSize.u32Height,
                                                PIXEL_FORMAT_YVU_SEMIPLANAR_420, DATA_BITWIDTH_8, COMPRESS_MODE_SEG, 0);
	printf("stVbConfig.astCommPool[0].u64BlkSize=%d\n",stVbConfig.astCommPool[0].u64BlkSize);				
	
	u32BlkSize = VI_GetRawBufferSize(stDispSize.u32Width, stDispSize.u32Height, PIXEL_FORMAT_RGB_BAYER_16BPP, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
    stVbConfig.astCommPool[1].u64BlkSize  = u32BlkSize;
    stVbConfig.astCommPool[1].u32BlkCnt   = 4;
	printf("stVbConfig.astCommPool[1].u64BlkSize=%d\n",u32BlkSize);

    s32Ret = SAMPLE_COMM_SYS_Init(&stVbConfig);
    if(s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("init sys fail for %#x!\n", s32Ret);
        goto END1;
    }



    /************************************************
    step2:  init module VB or user VB(for VDEC)
    *************************************************/
    for(i=0; i<u32VdecChnNum-1; i++)
    {
        astSampleVdec[i].enType                           = PT_H264;
        astSampleVdec[i].u32Width                         = 1920;
        astSampleVdec[i].u32Height                        = 1080;
        astSampleVdec[i].enMode                           = VIDEO_MODE_FRAME;	//按帧方式发送码流
        astSampleVdec[i].stSapmleVdecVideo.enDecMode      = VIDEO_DEC_MODE_IPB;	//ipb全解吗
        astSampleVdec[i].stSapmleVdecVideo.enBitWidth     = DATA_BITWIDTH_8;	//8bit
        astSampleVdec[i].stSapmleVdecVideo.u32RefFrameNum = 8;					//实际解码帧数应该是
        astSampleVdec[i].u32DisplayFrameNum               = 4;					//解码图像缓存的最小帧数
        astSampleVdec[i].u32FrameBufCnt = astSampleVdec[i].stSapmleVdecVideo.u32RefFrameNum + astSampleVdec[i].u32DisplayFrameNum + 1;//解码图像帧存个数  满足解码要求  h264 参考帧+显示帧+1
    }
	//分配视频缓存空间
    s32Ret = SAMPLE_COMM_VDEC_InitVBPool(u32VdecChnNum-1, &astSampleVdec[0]);
    if(s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("init mod common vb fail for %#x!\n", s32Ret);
        goto END2;
    }




    /************************************************
    step3:  start VDEC
    *************************************************/
    s32Ret = SAMPLE_COMM_VDEC_Start(u32VdecChnNum-1, &astSampleVdec[0]);
    if(s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("start VDEC fail for %#x!\n", s32Ret);
        goto END3;
    }
	
	/*
			special step  enable bt1120 collection 
	*/
	s32Ret = SAMPLE_VIO_8K30_PARALLEL(enVoIntfTypebt1120);
	
	
	
	
    /************************************************
    step4:  start VPSS		GROUP 	无需修改
							CHN		VPSS_CHN_MODE_AUTO->VPSS_CHN_MODE_USER
									COMPRESS_MODE_SEG ->COMPRESS_MODE_NONE
									u32Depth		  -> 0->1								
							经验证无需修改
    *************************************************/
    stVpssGrpAttr.u32MaxW = 1920;
    stVpssGrpAttr.u32MaxH = 1080;
    stVpssGrpAttr.stFrameRate.s32SrcFrameRate = -1;											//输入帧率   1-240
    stVpssGrpAttr.stFrameRate.s32DstFrameRate = -1;											//输出帧率	 -1-srcrate
    stVpssGrpAttr.enDynamicRange = DYNAMIC_RANGE_SDR8;										//输入图像动态范围
    stVpssGrpAttr.enPixelFormat  = PIXEL_FORMAT_YVU_SEMIPLANAR_420;							//输入图像像素格式
    stVpssGrpAttr.bNrEn   = HI_FALSE;														//nr使能开关

    memset(abChnEnable, 0, sizeof(abChnEnable));
    abChnEnable[0] 								  = HI_TRUE;
    astVpssChnAttr[0].u32Width                    = stDispSize.u32Width;					//显示大小
    astVpssChnAttr[0].u32Height                   = stDispSize.u32Height;					//
    astVpssChnAttr[0].enChnMode                   = VPSS_CHN_MODE_AUTO;						//自动模式
    astVpssChnAttr[0].enCompressMode              = COMPRESS_MODE_SEG;						//按照256字节压缩
    astVpssChnAttr[0].enDynamicRange              = DYNAMIC_RANGE_SDR8;						//8bit
    astVpssChnAttr[0].enPixelFormat               = PIXEL_FORMAT_YVU_SEMIPLANAR_420;
    astVpssChnAttr[0].stFrameRate.s32SrcFrameRate = -1;										//输入帧率   1-240									
    astVpssChnAttr[0].stFrameRate.s32DstFrameRate = -1;                                     //输出帧率	 -1-srcrate
    astVpssChnAttr[0].u32Depth                    = 0;										//用户获取通道图像的队列长度
    astVpssChnAttr[0].bMirror                     = HI_FALSE;								//水平镜使能
    astVpssChnAttr[0].bFlip                       = HI_FALSE;								//垂直翻转使能
    astVpssChnAttr[0].stAspectRatio.enMode        = ASPECT_RATIO_NONE;						//无副型比
    astVpssChnAttr[0].enVideoFormat               = VIDEO_FORMAT_LINEAR;					//线性存储视频格式
    for(i=0; i<u32VdecChnNum; i++)
    {
        VpssGrp = i;
        s32Ret = SAMPLE_COMM_VPSS_Start(VpssGrp, &abChnEnable[0], &stVpssGrpAttr, &astVpssChnAttr[0]);
        if(s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("start VPSS fail for %#x!\n", s32Ret);
            goto END4;
        }
    }
	
	
	

    /************************************************
    step5:  start VO		增加bt1120无需修改
    *************************************************/
    stVoConfig.VoDev                 = SAMPLE_VO_DEV_UHD;									// 0
    stVoConfig.enVoIntfType          = VO_INTF_HDMI;										//HDMI输出模式
    stVoConfig.enIntfSync            = enIntfSync;											//VO_OUTPUT_1080P30
    stVoConfig.enPicSize             = enDispPicSize;										//PIC_1080P
    stVoConfig.u32BgColor            = COLOR_RGB_BLUE;										//无输出的vo输出界面 蓝色
    stVoConfig.u32DisBufLen          = 3;													//显示输出buf大小
    stVoConfig.enDstDynamicRange     = DYNAMIC_RANGE_SDR8;									//8bit
    stVoConfig.enVoMode              = VO_MODE_9CUS;										//1						
    stVoConfig.enPixFormat           = PIXEL_FORMAT_YVU_SEMIPLANAR_420;		
    stVoConfig.stDispRect.s32X       = 0;													//配置视频的宽度高度
    stVoConfig.stDispRect.s32Y       = 0;
    stVoConfig.stDispRect.u32Width   = stDispSize.u32Width;									//1920
    stVoConfig.stDispRect.u32Height  = stDispSize.u32Height;								//1080
    stVoConfig.stImageSize.u32Width  = stDispSize.u32Width;									//1920
    stVoConfig.stImageSize.u32Height = stDispSize.u32Height;								//1080
    stVoConfig.enVoPartMode          = VO_PART_MODE_SINGLE;									//视频层按照单区域配置给硬件显示

    s32Ret = SAMPLE_COMM_VO_StartVO(&stVoConfig);
    if(s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("start VO fail for %#x!\n", s32Ret);
        goto END5;
    }
	
	
	
    /************************************************
    step6:  VDEC bind VPSS
	vichn	vipipe	vdec   vpssgrop   	vpsschn  	vo
	  0		  0					0			0		0	
					0 			1			0		1	
					1			2			0		2
					2		   	3			0		3
					3			4			0		4
					4			5			0		5	
					5			6			0		6
					6			7			0		7
					7			8			0		8
					8			9			0		9

    *************************************************/
	printf("smmmmmmmmmmmmmmmmmmmmsmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm\n");
	s32Ret = SAMPLE_COMM_VI_Bind_VPSS(0, 0, 0);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VI_Bind_VPSS failed with %d!\n", s32Ret);
    }
	printf("smmmmmmmmmmmmmmmmmmmmsmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm1\n");

    for(i=1; i<u32VdecChnNum; i++)
    {
        s32Ret = SAMPLE_COMM_VDEC_Bind_VPSS(i-1, i);
        if(s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("vdec bind vpss fail for %#x!\n", s32Ret);
            goto END6;
        }
    }
    /************************************************
    step7:  VPSS bind VO	
    *************************************************/
    VoLayer = stVoConfig.VoDev;
    for(i=0; i<VpssGrpNum; i++)
    {
        s32Ret = SAMPLE_COMM_VPSS_Bind_VO(i, 0, VoLayer,i);			//参数列表   vpss组     通道号     视频层    vo通道号
        if(s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("vpss bind vo fail for %#x!\n", s32Ret);
            goto END7;
        }
    }
	
	/*
	step8: create osd 
	*/
	
	for(i = 0 ; i < 10 ; i++ )
	{
		if(i == 9)
		{
			HH_OSD_Init(i,10,199,0);
			continue;	
		}
		HH_OSD_Init(i,9,190+i,0);
	}
	previous_mode_all = 199;
	current_vdeio_display_mode = 9;
	who_in_pos9 = 9;
	
	
	
	
    /************************************************
    step8:  send stream to VDEC
    *************************************************/
    for(i=0; i<u32VdecChnNum-1; i++)
    {
        snprintf(stVdecSend[i].cFileName, sizeof(stVdecSend[i].cFileName), "3840x2160_8bit.h264");
        snprintf(stVdecSend[i].cFilePath, sizeof(stVdecSend[i].cFilePath), "%s", SAMPLE_STREAM_PATH);
        stVdecSend[i].enType          = astSampleVdec[i].enType;											//h264
        stVdecSend[i].s32StreamMode   = astSampleVdec[i].enMode;											//VIDEO_MODE_FRAME
        stVdecSend[i].s32ChnId        = i;
        stVdecSend[i].s32IntervalTime = 1000;
        stVdecSend[i].u64PtsInit      = 0;
        stVdecSend[i].u64PtsIncrease  = 0;
        stVdecSend[i].eThreadCtrl     = THREAD_CTRL_START;													//线程工作方式
        stVdecSend[i].bCircleSend     = HI_TRUE;
        stVdecSend[i].s32MilliSec     = 0;
        stVdecSend[i].s32MinBufSize   = (astSampleVdec[i].u32Width * astSampleVdec[i].u32Height * 3)>>1; //缓存一帧的大小  YUV420格式视频  Y = W *H , UV=W*H/2  ALL = W*H + W+H/2
		stVdecSend[i].msg = msg;
    }
    SAMPLE_COMM_VDEC_StartSendStream(u32VdecChnNum-1, &stVdecSend[0], &VdecThread[0]);
    SAMPLE_COMM_VDEC_CmdCtrl(u32VdecChnNum-1, &stVdecSend[0], &VdecThread[0]);


    SAMPLE_COMM_VDEC_StopSendStream(u32VdecChnNum-1, &stVdecSend[0], &VdecThread[0]);

END7:
    for(i=0; i<VpssGrpNum; i++)
    {
        s32Ret = SAMPLE_COMM_VPSS_UnBind_VO(i, 0, VoLayer, i);
        if(s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("vpss unbind vo fail for %#x!\n", s32Ret);
        }
    }

END6:
    for(i=0; i<u32VdecChnNum-1; i++)
    {
        s32Ret = SAMPLE_COMM_VDEC_UnBind_VPSS(i, i);
        if(s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("vdec unbind vpss fail for %#x!\n", s32Ret);
        }
    }

END5:
    SAMPLE_COMM_VO_StopVO(&stVoConfig);

END4:
    for(i = VpssGrp; i >= 0; i--)
    {
        VpssGrp = i;
        SAMPLE_COMM_VPSS_Stop(VpssGrp, &abChnEnable[0]);
    }
END3:
    SAMPLE_COMM_VDEC_Stop(u32VdecChnNum-1);

END2:
    SAMPLE_COMM_VDEC_ExitVBPool();

END1:
    SAMPLE_COMM_SYS_Exit();

    return s32Ret;
}





void *Media_Message_Receive_thread(void *arg)
{
	MESSAGE *msg =(MESSAGE *)arg;
	openall(msg);
	return ;
}





/******************************************************************************
* function    : main()
* Description : video vdec sample
******************************************************************************/
#ifdef __HuaweiLite__
    int app_main(int argc, char *argv[])
#else
    int main(int argc, char *argv[])
#endif
{ 	
	int ret = 0;
	int i;	
	MESSAGE *msg = (MESSAGE *)malloc(sizeof(MESSAGE));
	for(i = 0; i < 9; i++)
	{
		msg->rindex[i] = 0;
		msg->windex[i] = 0;
		msg->windexEndPos[i] = 24883200;
	}
	//初始化管道
	create_fifo("./fifo");
	ret = open_fifo(msg->fd);
	//int类型数组bck_not_show -1通道正常显示   1 通道不显示
	memset(msg->bck_not_show,-1,sizeof(int)*BCK_NOTSHOW_CHN);
	msg->pos[0] = 0;
	msg->pos[1] = 0;
	if(ret < 0)
	{
		printf("open fifo Failed!!\n");
	}
	
	//初始化线程锁
	ret = pthread_cond_init(&cond,NULL);
	ret = pthread_mutex_init(&mutex,NULL);
	if(ret != 0)
	{
		return;
	}
	HI_S32 s32Ret = HI_SUCCESS;
	pthread_t tid[2];
	g_enIntfSync = VO_OUTPUT_1080P30;

	s32Ret = pthread_create(&tid[0], NULL, Media_Message_Receive_thread,msg);
	s32Ret = pthread_create(&tid[1], NULL, (void *)Media_Common_Receive_thread,msg);
	
	s32Ret = SAMPLE_H264_VDEC_VPSS_VO(msg);
#ifndef __HuaweiLite__
    signal(SIGINT, SAMPLE_VDEC_HandleSig);
    signal(SIGTERM, SAMPLE_VDEC_HandleSig);
#endif
    if (HI_SUCCESS == s32Ret)
    {
        SAMPLE_PRT("program exit normally!\n"); 
    }
    else
    {
        SAMPLE_PRT("program exit abnormally!\n");
    }
	
	int media_Recv_Thread_Exit_Flag = pthread_join(&tid[0],NULL);
	media_Recv_Thread_Exit_Flag = pthread_join(&tid[1],NULL);
	if(media_Recv_Thread_Exit_Flag == 0)
	{
		printf("media recv thread exit!!\n");
	}
    return s32Ret;
}


