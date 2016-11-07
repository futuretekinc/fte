#ifndef __FTE_FIS_H__
#define __FTE_FIS_H__

#define FTE_FIS_MODEL_3061  3061

typedef struct  
{
    uint_32 nADC;
    uint_32 nPPM;
}   FTE_FIS_VALUE_PAIR, _PTR_ FTE_FIS_VALUE_PAIR_PTR;

typedef FTE_FIS_VALUE_PAIR const _PTR_ FTE_FIS_VALUE_PAIR_CONST_PTR;

typedef struct _fte_fis_model_info
{
    uint_32     nModel;
    char_ptr    pName;
    uint_32     nCount;
    FTE_FIS_VALUE_PAIR_CONST_PTR  pPairs;
} FTE_FIS_MODEL_INFO, _PTR_ FTE_FIS_MODEL_INFO_PTR;

typedef FTE_FIS_MODEL_INFO const _PTR_ FTE_FIS_MODEL_INFO_CONST_PTR;
/*****************************************************************************
 * FIS Object Structure Description
 *****************************************************************************/
typedef struct FTE_FIS_CONFIG_STRUCT
{
    FTE_COMMON_CONFIG   xCommon;
    uint_32             nModel;
    uint_32             nUCSID;
    uint_32             nREQID;
    uint_32             nInterval;
}   FTE_FIS_CONFIG, _PTR_ FTE_FIS_CONFIG_PTR;

#if sizeof(FTE_OBJECT_CONFIG) < sizeof(FTE_DI_CONFIG)
#error  "FTE_OBJECT_CONFIG size is too small!"
#endif

typedef FTE_FIS_CONFIG const _PTR_ FTE_FIS_CONFIG_CONST_PTR;

typedef struct  FTE_FIS_STATUS_STRUCT
{
    uint_32         xFlags;
    FTE_VALUE_PTR   pValue;
    TIME_STRUCT     xEventTimeStamp;
    uint_32         hRepeatTimer;
    uint_32         hConvertTimer;
    int_32          nValue;
    int_32          nTemperature;
    int_32          nAlarm;
    int_32          nDate;
    FTE_UCS_PTR     pUCS;
    FTE_LWGPIO_PTR  pREQ;
    FTE_FIS_MODEL_INFO_CONST_PTR  pModelInfo;
}   FTE_FIS_STATUS, _PTR_ FTE_FIS_STATUS_PTR;

_mqx_uint   FTE_FIS_attach(FTE_OBJECT_PTR pObj);
_mqx_uint   FTE_FIS_detach(FTE_OBJECT_PTR pObj);

#endif
