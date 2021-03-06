/******************************************************************************

  Copyright (C), 2017-2018, Hisilicon Tech. Co., Ltd.

 ******************************************************************************
  File Name     : mpi_match.h
  Version       : Initial Draft
  Author        : Hisilicon multimedia software group
  Created       : 2017/09/20
  Description   :
  History       :
  1.Date        : 2017/09/20
    Modification: Created file

******************************************************************************/

#ifndef __MPI_MATCH_H__
#define __MPI_MATCH_H__


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */
#include "hi_comm_dpu_match.h"

HI_S32 HI_MPI_DPU_MATCH_GetAssistBufSize(HI_U16 u16DispNum, HI_U32 u32DstHeight, HI_U32 *pu32Size);
HI_S32 HI_MPI_DPU_MATCH_CreateGrp(DPU_MATCH_GRP DpuMatchGrp, const DPU_MATCH_GRP_ATTR_S *pstGrpAttr);
HI_S32 HI_MPI_DPU_MATCH_DestroyGrp(DPU_MATCH_GRP DpuMatchGrp);
HI_S32 HI_MPI_DPU_MATCH_SetGrpAttr(DPU_MATCH_GRP DpuMatchGrp,const DPU_MATCH_GRP_ATTR_S *pstGrpAttr);
HI_S32 HI_MPI_DPU_MATCH_GetGrpAttr(DPU_MATCH_GRP DpuMatchGrp, DPU_MATCH_GRP_ATTR_S *pstGrpAttr);
HI_S32 HI_MPI_DPU_MATCH_StartGrp(DPU_MATCH_GRP DpuMatchGrp);
HI_S32 HI_MPI_DPU_MATCH_StopGrp(DPU_MATCH_GRP DpuMatchGrp);
HI_S32 HI_MPI_DPU_MATCH_SetChnAttr(DPU_MATCH_GRP DpuMatchGrp, DPU_MATCH_CHN DpuMatchChn,const DPU_MATCH_CHN_ATTR_S *pstChnAttr);
HI_S32 HI_MPI_DPU_MATCH_GetChnAttr(DPU_MATCH_GRP DpuMatchGrp, DPU_MATCH_CHN DpuMatchChn, DPU_MATCH_CHN_ATTR_S *pstChnAttr);
HI_S32 HI_MPI_DPU_MATCH_EnableChn(DPU_MATCH_GRP DpuMatchGrp, DPU_MATCH_CHN DpuMatchChn);
HI_S32 HI_MPI_DPU_MATCH_DisableChn(DPU_MATCH_GRP DpuMatchGrp, DPU_MATCH_CHN DpuMatchChn);
HI_S32 HI_MPI_DPU_MATCH_GetFrame(DPU_MATCH_GRP DpuMatchGrp, VIDEO_FRAME_INFO_S *pstSrcLeftFrame,
    VIDEO_FRAME_INFO_S *pstSrcRightFrame, VIDEO_FRAME_INFO_S *pstDstFrame, HI_S32 s32MilliSec);
HI_S32 HI_MPI_DPU_MATCH_ReleaseFrame(DPU_MATCH_GRP DpuMatchGrp,const VIDEO_FRAME_INFO_S *pstSrcLeftFrame,
   const VIDEO_FRAME_INFO_S *pstSrcRightFrame,const VIDEO_FRAME_INFO_S *pstDstFrame);
HI_S32 HI_MPI_DPU_MATCH_SendFrame(DPU_MATCH_GRP DpuMatchGrp,const VIDEO_FRAME_INFO_S *pstLeftFrame,
    const VIDEO_FRAME_INFO_S *pstRightFrame, HI_S32 s32MilliSec);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __MPI_MATCH_H__ */

