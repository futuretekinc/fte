#ifndef __FTE_RTD_H__
#define __FTE_RTD_H__


/*****************************************************************************
 * AD7785-based RTD Object Structure Description
 *****************************************************************************/
typedef struct FTE_RTD_CONFIG_STRUCT
{
    FTE_COMMON_CONFIG   xCommon;
    FTE_UINT32          nDevID;
    FTE_UINT32          nInterval;
}   FTE_RTD_CONFIG, _PTR_ FTE_RTD_CONFIG_PTR;

typedef struct  FTE_RTD_STATUS_STRUCT
{
    FTE_OBJECT_STATUS   xCommon;
    FTE_UINT32          hRepeatTimer;
    FTE_UINT32          hConvertTimer;
    FTE_AD7785_PTR      pADC;
}   FTE_RTD_STATUS, _PTR_ FTE_RTD_STATUS_PTR;

FTE_RET     FTE_RTD_preinit(FTE_UINT32 nMaxObjects);

FTE_RET     FTE_RTD_attach(FTE_OBJECT_PTR pObj, FTE_VOID_PTR pOpts);
FTE_RET     FTE_RTD_detach(FTE_OBJECT_PTR pObj);
FTE_UINT32  FTE_RTD_printValue(FTE_OBJECT_PTR pObj, char_ptr pBuff, FTE_UINT32 nLen);

extern  FTE_RTD_CONFIG FTE_RTD_defaultConfig;
#endif
