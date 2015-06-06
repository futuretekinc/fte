#ifndef __FTE_GAI_H__
#define __FTE_GAI_H__

typedef enum
{
    FTE_GAI_VALUE_TYPE_UNKNOWN      = 0,
    FTE_GAI_VALUE_TYPE_0_2_TO_1_518V  = 1,
    FTE_GAI_VALUE_TYPE_0_2_TO_1_10A
}   FTE_GAI_VALUE_TYPE, _PTR_ FTE_GAI_VALUE_TYPE_PTR;

typedef struct
{
    FTE_GAI_VALUE_TYPE  xType;
    struct
    {
        uint_32 ulMin;
        uint_32 ulMax;
    }   xInputRange;
    struct
    {
        FTE_VALUE_TYPE  xType;
        uint_32         ulMin;
        uint_32         ulMax;
    }   xValueRange;
    uint_32     ulCalibration;
}   FTE_GAI_VALUE_DESCRIPT, _PTR_ FTE_GAI_VALUE_DESCRIPT_PTR;


/*****************************************************************************
 * AD7785-based GAI Object Structure Description
 *****************************************************************************/
typedef struct _FTE_GAI_CONFIG_STRUCT
{
    FTE_COMMON_CONFIG   xCommon;
    uint_32             nDevID;
    uint_32             nInterval;
    uint_32             nGain;
    uint_32             ulDivide;
    FTE_GAI_VALUE_TYPE  xValueType;
}   FTE_GAI_CONFIG, _PTR_ FTE_GAI_CONFIG_PTR;

typedef struct  _fte_gai_status_struct
{
    FTE_OBJECT_STATUS           xCommon;
    uint_32                     hRepeatTimer;
    uint_32                     hConvertTimer;
    FTE_AD7785_PTR              pADC;
    FTE_GAI_VALUE_DESCRIPT_PTR  pValueDescript;
}   FTE_GAI_STATUS, _PTR_ FTE_GAI_STATUS_PTR;

typedef struct _FTE_GAI_ACTION_STRUCT
{
    _mqx_uint       (*f_init)(FTE_OBJECT_PTR pSelf);
    _mqx_uint       (*f_run)(FTE_OBJECT_PTR pSelf);
    _mqx_uint       (*f_stop)(FTE_OBJECT_PTR pSelf);
    _mqx_uint       (*f_get)(FTE_OBJECT_PTR pSelf, uint_32_ptr pValue, TIME_STRUCT_PTR pTimeStamp);
    _mqx_uint       (*f_set)(FTE_OBJECT_PTR pSelf, uint_32 nValue);
    uint_32         (*f_print_value)(FTE_OBJECT_PTR pSelf, char_ptr pBuff, uint_32 nBuffLen);
}   FTE_GAI_ACTION, _PTR_ FTE_GAI_ACTION_PTR;

_mqx_uint   FTE_GAI_attach(FTE_OBJECT_PTR pObj);
_mqx_uint   FTE_GAI_detach(FTE_OBJECT_PTR pObj);

#endif
