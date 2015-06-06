#ifndef __FTE_RTD_H__
#define __FTE_RTD_H__


/*****************************************************************************
 * AD7785-based RTD Object Structure Description
 *****************************************************************************/
typedef struct _fte_rtd_config_struct
{
    FTE_COMMON_CONFIG   xCommon;
    uint_32             nDevID;
    uint_32             nInterval;
}   FTE_RTD_CONFIG, _PTR_ FTE_RTD_CONFIG_PTR;

typedef struct  _fte_rtd_status_struct
{
    FTE_OBJECT_STATUS   xCommon;
    uint_32             hRepeatTimer;
    uint_32             hConvertTimer;
    FTE_AD7785_PTR      pADC;
}   FTE_RTD_STATUS, _PTR_ FTE_RTD_STATUS_PTR;

typedef struct _fte_rtd_action_struct
{
    _mqx_uint       (*f_init)(FTE_OBJECT_PTR pSelf);
    _mqx_uint       (*f_run)(FTE_OBJECT_PTR pSelf);
    _mqx_uint       (*f_stop)(FTE_OBJECT_PTR pSelf);
    _mqx_uint       (*f_get)(FTE_OBJECT_PTR pSelf, uint_32_ptr pValue, TIME_STRUCT_PTR pTimeStamp);
    _mqx_uint       (*f_set)(FTE_OBJECT_PTR pSelf, uint_32 nValue);
    uint_32         (*f_print_value)(FTE_OBJECT_PTR pSelf, char_ptr pBuff, uint_32 nBuffLen);
}   FTE_RTD_ACTION, _PTR_ FTE_RTD_ACTION_PTR;

_mqx_uint fte_rtd_preinit(uint_32 nMaxObjects);

_mqx_uint   fte_rtd_attach(FTE_OBJECT_PTR pObj);
_mqx_uint   fte_rtd_detach(FTE_OBJECT_PTR pObj);
uint_32     FTE_RTD_printValue(FTE_OBJECT_PTR pObj, char_ptr pBuff, uint_32 nLen);

#endif
