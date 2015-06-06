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
typedef struct _fte_fis_config_struct
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

typedef struct  _fte_fis_status_struct
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

typedef struct _fte_fis_action_struct
{
    _mqx_uint       (*f_init)(FTE_OBJECT_PTR pSelf);
    _mqx_uint       (*f_run)(FTE_OBJECT_PTR pSelf);
    _mqx_uint       (*f_stop)(FTE_OBJECT_PTR pSelf);
    _mqx_uint       (*f_get)(FTE_OBJECT_PTR pSelf, uint_32_ptr pValue, TIME_STRUCT_PTR pTimeStamp);
    _mqx_uint       (*f_set)(FTE_OBJECT_PTR pSelf, uint_32 nValue);
    uint_32         (*f_print_value)(FTE_OBJECT_PTR pSelf, char_ptr pBuff, uint_32 nBuffLen);
}   FTE_FIS_ACTION, _PTR_ FTE_FIS_ACTION_PTR;

_mqx_uint   fte_fis_attach(FTE_OBJECT_PTR pObj);
_mqx_uint   fte_fis_detach(FTE_OBJECT_PTR pObj);

#endif
