#ifndef __SAMPLE_VDEC_H__
#define __SAMPLE_VDEC_H__
#include "sample_comm.h"
int previous_mode_all;
int current_vdeio_display_mode; //==9标识9屏   ==4 标识四屏  ==1 标识大屏
int cmd_response;
int who_in_pos9;
int cmd_confirm;
int witch_mode_4;

HI_S32 SAMPLE_H265_VDEC_VPSS_VO(HI_VOID);
HI_S32 SAMPLE_H264_VDEC_VPSS_VO(HI_VOID);
HI_S32 SAMPLE_JPEG_VDEC_VPSS_VO(HI_VOID);
HI_S32 SAMPLE_JPEG_VDEC_To_RGB(HI_VOID);
HI_S32 SAMPLE_H264_VDEC_VPSS_VO_MIPI_Tx(HI_VOID);

#endif
