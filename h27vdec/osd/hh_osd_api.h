#ifndef _HH_OSD_API_H
#define _HH_OSD_API_H

//海思规定每个编码组只支持4 个osd叠加区域
//如果要增加osd 区域需修改以下两个参数

#include <hi_type.h>
#include <hi_common.h>
#include <mpi_region.h>
#include "hi_comm_pciv.h"
#include <hi_comm_region.h>

#define COMM_SYSLOG(value,fmt, args...) do { printf("[%s, %s, %d] "fmt, __FILE__, __FUNCTION__, __LINE__, ##args); } while(0)
#define VIDEO_NUM	9						//配置视频有几路

#define HH_MAX_OSD_NUM	  48   //(4*3*3)	//做多支持多少路OSD  我们的项目里面只有2路....

#define HH_MAX_OSD_PER_GROUP    1  		//目前 victa 只用2个   每一组最多2个OSD 我们的项目用不到


#define HH_MAX_TITLE_NUM  256				//最多有256个字符.....   最多没有那么长
#define HH_MAX_TITLE_LINE 11				//单行......支持最多11行; 11行加起来做多256个字符....

typedef struct _HI_OSD_ORG
{
	HI_U16      nOrgX;       //osd 区域的左上角X坐标
	HI_U16      nOrgY;       //osd 区域的左上角Y坐标
}HI_OSD_ORG, *PHI_OSD_ORG;

typedef struct _HI_OSD_COLOR
{
	HI_U16      bgColor;     		//osd 背景颜色
	HI_U16      fgColor;     		//osd 前景颜色
	HI_U16		bgAlpha;        //osd 背景透明度
	HI_U16		fgAlpha;	 	 //osd 前景透明度
	HI_U16		edgeColor;	 //osd 前景边缘颜色			文字边沿的颜色; 文字用字库生成不是BMP..一圈多余的地方是什么颜色
	HI_U16      selColor;    		 //osd 菜单选中行颜色
}HI_OSD_COLOR, *PHI_OSD_COLOR;

typedef struct _HH_OSD_LOGO		//显示OSD LOGO
{
	//VENC_GRP     vencGroup ;         //osd 区域叠加到的编码组
	int			 vochn;				   //叠加至VO的通道
	HI_U8        nStatus   ;  		   //显示 0 隐藏---ZHAUNGTAI BIAOZHI ...
	HI_U8        nOsdLine  ;   	       //osd 字幕的行数
	HI_U8        nOsdFontH ;	       //字宽
	HI_U8        nOsdFontW ;	       //字高
	HI_U16		 nWidth    ;           //整个osd 区域的宽度
	HI_U16		 nHeight   ;	       //整个osd 区域的高度
	HI_U16       nImageWidth;
	HI_U16       nImageHeight;
	HI_S16       nFontInt  ;    	       //osd 字符间隔多少个像素位
	HI_U16       nVencFeild;		
	HI_U8        nOsdTitleMaxLen;     //标记最长的一行字符的个?
//下面最大支持11行
	//存的是每行字符串开始索引号....
	HI_U8        nOsdTitleFlag[HH_MAX_TITLE_LINE]; //osd 最大支持8 行 标记每一行osd 第一个字符在osdtitle 中的位置
	//每一行的长度十多号; 内容是字符串,这个记录长度; 上面的记录内容...
	HI_U8        nOsdTitleLen[HH_MAX_TITLE_LINE];  //标记每一行osd 的长度

	//一个字符串数组,记录几行OSD里面的每一行的内容是什么
	//整个的OSD信息是记录到这里的....解析了之后填充内容和长度到nOsdTitleFlag[]/nOsdTitleLen[]
	char         szOsdTitle[HH_MAX_TITLE_NUM];  	 //osd 叠加字幕信息256字节

	HI_OSD_ORG   osdPos    ;         //OSD的起始坐标....
	HI_OSD_COLOR osdColor  ;       //OSD显示的颜色
	HI_U8          *pBmpBuf;		//将来创建的BMP文件; 指向他BMP文件bufer
//#if defined(HI3516_ARCH) || defined(HI3518_ARCH)
	int			  regHdl;		      //osd 区域句柄

	int          nSelLine  ;    //标记当前行为选中行  //用来做菜单; 用不到没有任何的作用的...
}HH_OSD_LOGO, *PHH_OSD_LOGO;

typedef struct _HH_VIDEO_RESOLUTION
{
	int      width;
	int      height;
}HH_VIDEO_RESOLUTION, *PHH_VIDEO_RESOLUTION;

HH_OSD_LOGO *HI_Create_Osd(int index, HH_OSD_LOGO **ppLog, int vencGroup,
						   HI_OSD_ORG	osdOrg, HI_OSD_COLOR osdColor,
 							char *pSzTitle, HI_S16 nFontInt, HI_U8 nShow);

int  HI_OSD_Parse_OsdTitle(HH_OSD_LOGO *pOsdLogo);

int  HI_OSD_Get_BmpSize(HH_OSD_LOGO *pOsdLogo);

int  HI_OSD_Set_Show(HH_OSD_LOGO *pOsdLogo, int show);

int  HI_Create_Osd_Reg(HH_OSD_LOGO *pOsdLogo);

int  HI_OSD_Create_Bitmap(HH_OSD_LOGO *pOsdLogo);

int  HI_OSD_Refresh_Color(HH_OSD_LOGO *pOsdLogo);

int  HI_OSD_Refresh_Title(HH_OSD_LOGO *pOsdLogo);

int  HI_OSD_Refresh_Org(HH_OSD_LOGO *pOsdLogo);
int HI_OSD_Check_ImaSizeChange(HH_OSD_LOGO *pOsdLogo);

int  HI_OSD_Destroy(HH_OSD_LOGO *pOsdLogo);

int  HI_OSD_Build(HH_OSD_LOGO *pOsdLogo);

int  HI_OSD_GetCharBit(unsigned char *in, short qu, short wei, short fontSize);

int  HI_OSD_DrawBitmapBit(int  x_pos, HI_U8 *bitmap, HI_U16 fColor, HI_U16 bColor,
						  HI_U16 edgeColor, HI_U8 *bmpBuffer, HI_U16 bmpWidth,
						  HI_U16 fontSize);

int  HI_OSD_DrawBitmapBit32(int x_pos, HI_U8 *bitmap, HI_U16 fColor, HI_U16 bColor,
						  HI_U16 edgeColor, HI_U8 *bmpBuffer, HI_U16 bmpWidth,
						  HI_U16 fontSize);

void HI_OSD_CreateBMP(int bTwoFeild,  int nSize, int bmpWidth, int space,
					  char *bmpBuffer, char *string, HI_U16 fColor,
					  HI_U16 bColor, HI_U16 edgeColor);

int  HI_OSD_Refresh_SelLine(HH_OSD_LOGO *pOsdLogo, int nLine);

#endif

