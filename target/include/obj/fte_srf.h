#ifndef __FTE_SRF_H__
#define __FTE_SRF_H__


/*****************************************************************************
 * SRF Object Structure Description
 *****************************************************************************/
typedef struct FTE_SRF_CONFIG_STRUCT
{
    FTE_COMMON_CONFIG   xCommon;
    FTE_UINT32          nDevID;
    FTE_UINT32          nInterval;
}   FTE_SRF_CONFIG, _PTR_ FTE_SRF_CONFIG_PTR;

typedef FTE_SRF_CONFIG const _PTR_ FTE_SRF_CONFIG_CONST_PTR;

typedef struct  FTE_SRF_STATUS_STRUCT
{
    FTE_OBJECT_STATUS   xCommon;
    FTE_UINT32             hRepeatTimer;
    FTE_UINT32             hConvertTimer;
    MQX_FILE_PTR        pFP;
}   FTE_SRF_STATUS, _PTR_ FTE_SRF_STATUS_PTR;

FTE_RET FTE_SRF_preinit(FTE_UINT32 nMaxObjects);

FTE_RET   FTE_SRF_attach(FTE_OBJECT_PTR pObj, FTE_VOID_PTR pOpts);
FTE_RET   FTE_SRF_detach(FTE_OBJECT_PTR pObj);

#endif
