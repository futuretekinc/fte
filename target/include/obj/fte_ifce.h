#ifndef __FTE_IFCE_H__
#define __FTE_IFCE_H__


/*****************************************************************************
 * AD7785-based IFCE Object Structure Description
 *****************************************************************************/
typedef struct FTE_IFCE_CONFIG_STRUCT
{
    FTE_COMMON_CONFIG   xCommon;
    FTE_UINT32             nDevID;
    FTE_UINT32             nRegID;
    FTE_UINT32             nInterval;
}   FTE_IFCE_CONFIG, _PTR_ FTE_IFCE_CONFIG_PTR;

typedef struct  FTE_IFCE_STATUS_STRUCT
{
    FTE_OBJECT_STATUS   xCommon;
    FTE_UINT32          hRepeatTimer;
    FTE_UINT32          hConvertTimer;
    FTE_OBJECT_PTR      pParent;
}   FTE_IFCE_STATUS, _PTR_ FTE_IFCE_STATUS_PTR;

FTE_RET FTE_IFCE_preinit(FTE_UINT32 nMaxObjects);

FTE_RET FTE_IFCE_attach(FTE_OBJECT_PTR pObj, FTE_VOID_PTR pOpts);
FTE_RET FTE_IFCE_detach(FTE_OBJECT_PTR pObj);

#endif
